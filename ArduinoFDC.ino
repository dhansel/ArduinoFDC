// -----------------------------------------------------------------------------
// 3.5"/5.25" DD/HD Disk controller for Arduino
// Copyright (C) 2021 David Hansel
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
// -----------------------------------------------------------------------------

#include "ArduinoFDC.h"
#include "ff.h"


// comment this out to remove high-level ArduDOS functions
//#define USE_ARDUDOS

// commenting this out will remove the low-level disk monitor
#define USE_MONITOR

// comenting this out will remove support for XModem data transfers
#define USE_XMODEM


#if defined(__AVR_ATmega32U4__) && defined(USE_ARDUDOS) && (defined(USE_MONITOR) || defined(USE_XMODEM))
#warning "Arduino Leonardo/Micro program memory is too small to support ArduDOS together with other components!"
#undef USE_MONITOR
#undef USE_XMODEM
#endif

// -------------------------------------------------------------------------------------------------
// Basic helper functions
// -------------------------------------------------------------------------------------------------

#define TEMPBUFFER_SIZE 80
byte tempbuffer[TEMPBUFFER_SIZE];

unsigned long motor_timeout = 0;


void print_hex(byte b)
{
  if( b<16 ) Serial.write('0');
  Serial.print(b, HEX);
}


void dump_buffer(int offset, byte *buf, int n)
{
  int i = 0;
  while( i<n )
    {
      print_hex((offset+i)/256); 
      print_hex((offset+i)&255); 
      Serial.write(':');

      for(int j=0; j<16; j++)
        {
          if( (j&7)==0  ) Serial.write(' ');
          if( i+j<n ) print_hex(buf[i+j]); else Serial.print(F("  "));
          Serial.write(' ');
        }

      Serial.write(' ');
      for(int j=0; j<16; j++)
        {
          if( (j&7)==0  ) Serial.write(' ');
          if( i+j<n ) Serial.write(isprint(buf[i+j]) ? buf[i+j] : '.'); else Serial.write(' ');
        }

      Serial.println();
      i += 16;
    }
}


char *read_user_cmd(void *buffer, int buflen)
{
  char *buf = (char *) buffer;
  byte l = 0;
  do
    {
      int i = Serial.read();

      if( (i==13 || i==10) )
        { Serial.println(); break; }
      else if( i==27 )
        { l=0; Serial.println(); break; }
      else if( i==8 )
        { 
          if( l>0 )
            { Serial.write(8); Serial.write(' '); Serial.write(8); l--; }
        }
      else if( isprint(i) && l<buflen-1 )
        { buf[l++] = i; Serial.write(i); }
      
      if( motor_timeout>0 && millis() > motor_timeout ) { ArduinoFDC.motorOff(); motor_timeout = 0; }
    }
  while(true);

  while( l>0 && isspace(buf[l-1]) ) l--;
  buf[l] = 0;
  return buf;
}


bool confirm_formatting()
{
  int c;
  Serial.print(F("Formatting will erase all data on the disk in drive "));
  Serial.write('A' + ArduinoFDC.selectedDrive());
  Serial.print(F(". Continue (y/n)?"));
  while( (c=Serial.read())<0 );
  do { delay(1); } while( Serial.read()>=0 );
  Serial.println();
  return c=='y';
}


void print_drive_type(byte n)
{
  switch( n )
    {
    case ArduinoFDCClass::DT_5_DD: Serial.print(F("5.25\" DD")); break;
    case ArduinoFDCClass::DT_5_DDonHD: Serial.print(F("5.25\" DD disk in HD drive")); break;
    case ArduinoFDCClass::DT_5_HD: Serial.print(F("5.25\" HD")); break;
    case ArduinoFDCClass::DT_3_DD: Serial.print(F("3.5\" DD")); break;
    case ArduinoFDCClass::DT_3_HD: Serial.print(F("3.5\" HD")); break;
    case ArduinoFDCClass::DT_5_RX50: Serial.print(F("5.25\" RX50 DD")); break;
#ifdef WOZ
    case ArduinoFDCClass::DT_5_WOZ: Serial.print(F("5.25\" Apple][")); break;
#endif
    default: Serial.print(F("Unknown"));
    }
}


void print_error(byte n)
{
  Serial.print(F("Error: "));
  switch( n )
    {
    case S_OK        : Serial.print(F("No error")); break;
    case S_NOTINIT   : Serial.print(F("ArduinoFDC.begin() was not called")); break;
    case S_NOTREADY  : Serial.print(F("Drive not ready")); break;
    case S_NOSYNC    : Serial.print(F("No sync marks found")); break;
    case S_NOHEADER  : Serial.print(F("Sector header not found")); break;
    case S_INVALIDID : Serial.print(F("Data record has unexpected id")); break;
    case S_CRC       : Serial.print(F("Data checksum error")); break;
    case S_NOTRACK0  : Serial.print(F("No track 0 signal detected")); break;
    case S_VERIFY    : Serial.print(F("Verify after write failed")); break;
    case S_READONLY  : Serial.print(F("Disk is write protected")); break;
    default          : Serial.print(F("Unknonwn error")); break;
    }
  Serial.println('!');
}


void set_drive_type(int n)
{
  ArduinoFDC.setDriveType((ArduinoFDCClass::DriveType) n);
  Serial.print(F("Setting disk type for drive ")); Serial.write('A'+ArduinoFDC.selectedDrive());
  Serial.print(F(" to ")); print_drive_type(ArduinoFDC.getDriveType());
  Serial.println();
}


// -------------------------------------------------------------------------------------------------
// XModem data transfer functions
// -------------------------------------------------------------------------------------------------


int recvChar(int msDelay) 
{ 
  unsigned long start = millis();
  while( (int) (millis()-start) < msDelay) 
    { 
      if( Serial.available() ) return (uint8_t) Serial.read(); 
    }

  return -1; 
}


void sendData(const char *data, int size)
{
  Serial.write((const uint8_t *) data, size);
}


// -------------------------------------------------------------------------------------------------
// High-level ArduDOS 
// -------------------------------------------------------------------------------------------------


#ifdef USE_ARDUDOS

static FATFS FatFs;
static FIL   FatFsFile;

#ifdef USE_XMODEM

#include "XModem.h"
FRESULT xmodem_status = FR_OK;

bool xmodemHandlerSend(unsigned long no, char* data, int size)
{
  UINT br;
  xmodem_status = f_read(&FatFsFile, data, size, &br);

  // if there is an error or there is nothing more to read then return
  if( xmodem_status != FR_OK || br==0 ) return false;
  
  // XMODEM sends blocks of 128 bytes so if we have less than that
  // fill the rest of the buffer with EOF (ASCII 26) characters
  while( (int) br<size ) data[br++]=26;

  return true;
}


bool xmodemHandlerReceive(unsigned long no, char* data, int size)
{
  UINT bw;
  xmodem_status = f_write(&FatFsFile, data, size, &bw);

  // if there is an error or then return
  if( xmodem_status != FR_OK ) return false;
  
  return true;
}

#endif

// Some versions of Arduino appear to have problems with the FRESULT type in the print_ff_error
// function - which reportedly can be fixed by putting a forward declaration here (thanks to rtrimbler!).
// I am not able to reproduce the error but adding a forward declaration can't hurt.
void print_ff_error(FRESULT fr);

void print_ff_error(FRESULT fr)
{
  Serial.print(F("Error #")); 
  Serial.print(fr);
  Serial.print(F(": "));
  switch( fr )
    {
    case FR_DISK_ERR: Serial.print(F("Low-level disk error")); break;
    case FR_INT_ERR: Serial.print(F("Internal error")); break;
    case FR_NOT_READY: Serial.print(F("Drive not ready")); break;
    case FR_NO_FILE: Serial.print(F("File not found")); break;
    case FR_NO_PATH: Serial.print(F("Path not found")); break;
    case FR_INVALID_NAME: Serial.print(F("Invalid path format")); break;
    case FR_DENIED: Serial.print(F("Directory full")); break;
    case FR_EXIST: Serial.print(F("File exists")); break;
    case FR_INVALID_OBJECT: Serial.print(F("Invalid object")); break;
    case FR_WRITE_PROTECTED: Serial.print(F("Disk is write protected")); break;
    case FR_INVALID_DRIVE: Serial.print(F("Invalid drive")); break;
    case FR_NOT_ENABLED: Serial.print(F("The volume has no work area")); break;
    case FR_NO_FILESYSTEM: Serial.print(F("Not a FAT file system")); break;
    case FR_MKFS_ABORTED: Serial.print(F("Format aborted due to error")); break;
    case FR_NOT_ENOUGH_CORE: Serial.print(F("Out of memory")); break;
    case FR_INVALID_PARAMETER: Serial.print(F("Invalid parameter")); break;
    default: Serial.print(F("Unknown")); break;
    }
  Serial.println();
}


void arduDOS()
{
  UINT count;
  FRESULT fr;

  f_mount(&FatFs, "0:", 0);
  while( 1 )
    {
      Serial.println();
      Serial.write('A'+ArduinoFDC.selectedDrive());
      Serial.print(F(":>"));
      char *cmd = read_user_cmd(tempbuffer, TEMPBUFFER_SIZE);

      if( strcmp_PF(cmd, PSTR("a:"))==0 || strcmp_PF(cmd, PSTR("b:"))==0 )
        {
          byte drive = cmd[0]-'a';
          if( drive != ArduinoFDC.selectedDrive() )
            {
              ArduinoFDC.motorOff();
              motor_timeout = 0;
              ArduinoFDC.selectDrive(drive);
            }

          f_mount(&FatFs, "0:", 0);
        }
      else if( strncmp_P(cmd, PSTR("dir"), 3)==0 )
        {
          DIR dir;
          FILINFO finfo;

          ArduinoFDC.motorOn();
          fr = f_opendir(&dir, strlen(cmd)<5 ? "0:\\" : cmd+4);
          if (fr == FR_OK) 
            {
              count = 0;
              while(1)
                {
                  fr = f_readdir(&dir, &finfo);
                  if( fr!=FR_OK || finfo.fname[0]==0 )
                    break;
                  
                  char *c = finfo.fname;
                  byte col = 0;
                  while( *c!=0 && *c!='.' ) { Serial.write(toupper(*c)); col++; c++; }
                  while( col<9 ) { Serial.write(' '); col++; }
                  if( *c=='.' )
                    {
                      c++;
                      while( *c!=0 ) { Serial.write(toupper(*c)); col++; c++; }
                    }
                  while( col<14 ) { Serial.write(' '); col++; }
                  if( finfo.fattrib & AM_DIR )
                    Serial.println(F("<DIR>"));
                  else
                    Serial.println(finfo.fsize);
                  count++;
                }

              f_closedir(&dir);

              if( fr==FR_OK )
                {
                  if( count==0 ) Serial.println(F("No files."));
                  
                  FATFS *fs;
                  DWORD fre_clust;
                  fr = f_getfree("0:", &fre_clust, &fs);

                  if( fr==FR_OK )
                    { Serial.print(fre_clust * fs->csize * 512); Serial.println(F(" bytes free.")); }
                }

              if( fr!=FR_OK )
                print_ff_error(fr);
            }
          else
            print_ff_error(fr);
        }
      else if( strncmp_P(cmd, PSTR("type "), 5)==0 )
        {
          ArduinoFDC.motorOn();
          fr = f_open(&FatFsFile, cmd+5, FA_READ);
          if( fr == FR_OK )
            {
              count = 1;
              while( count>0 )
                {
                  fr = f_read(&FatFsFile, tempbuffer, TEMPBUFFER_SIZE, &count);
                  if( fr == FR_OK )
                    Serial.write(tempbuffer, count);
                  else
                    print_ff_error(fr);
                }
              f_close(&FatFsFile);
            }
          else
            print_ff_error(fr);
        }
      else if( strncmp_P(cmd, PSTR("dump "), 5)==0 )
        {
          ArduinoFDC.motorOn();
          fr = f_open(&FatFsFile, cmd+5, FA_READ);
          if( fr == FR_OK )
            {
              count = 1;
              int offset = 0;
              while( count>0 )
                {
                  fr = f_read(&FatFsFile, tempbuffer, (TEMPBUFFER_SIZE/16)*16, &count);
                  if( fr == FR_OK )
                    { dump_buffer(offset, tempbuffer, count); offset += count; }
                  else
                    print_ff_error(fr);
                }
              f_close(&FatFsFile);
            }
          else
            print_ff_error(fr);
        }
      else if( strncmp_P(cmd, PSTR("write "), 6)==0 )
        {
          ArduinoFDC.motorOn();
          fr = f_open(&FatFsFile, cmd+6, FA_WRITE | FA_CREATE_NEW);
          if( fr == FR_OK )
            {
              motor_timeout = 0;
              while( true )
                {
                  char *s = read_user_cmd(tempbuffer, TEMPBUFFER_SIZE);
                  if( s[0] )
                    {
                      fr = f_write(&FatFsFile, s, strlen(cmd), &count);
                      if( fr==FR_OK ) fr = f_write(&FatFsFile, "\r\n", 2, &count);
                      if( fr!=FR_OK ) print_ff_error(fr);
                    }
                  else
                    break;
                }

              f_close(&FatFsFile);
            }
          else
            print_ff_error(fr);
        }
      else if( strncmp_P(cmd, PSTR("del "), 4)==0 )
        {
          ArduinoFDC.motorOn();
          fr = f_unlink(cmd+4);
          if( fr != FR_OK )
            print_ff_error(fr);
        }
      else if( strncmp_P(cmd, PSTR("mkdir "), 6)==0 )
        {
          ArduinoFDC.motorOn();
          fr = f_mkdir(cmd+6);
          if( fr != FR_OK )
            print_ff_error(fr);
        }
      else if( strncmp_P(cmd, PSTR("rmdir "), 6)==0 )
        {
          ArduinoFDC.motorOn();
          fr = f_rmdir(cmd+6);
          if( fr != FR_OK )
            print_ff_error(fr);
        }
      else if( strncmp_P(cmd, PSTR("disktype "), 9)==0 )
        {
          set_drive_type(atoi(cmd+9));
          f_mount(&FatFs, "0:", 0);
        }
      else if( strncmp_P(cmd, PSTR("format"), 6)==0 )
        {
          MKFS_PARM param;
          param.fmt = FM_FAT | FM_SFD; // FAT12 type, no disk partitioning
          param.n_fat = 2;             // number of FATs
          param.n_heads = 2;           // number of heads  n_heads=2 OK for MSDOS format
          param.n_sec_track = ArduinoFDC.numSectors(); 
          param.align = 1;             // block alignment (not used for FAT12)
          
          switch( ArduinoFDC.getDriveType() )
            {
            case ArduinoFDCClass::DT_5_DD:
            case ArduinoFDCClass::DT_5_DDonHD:
              param.au_size = 1024; // bytes/cluster
              param.n_root  = 112;  // number of root directory entries
              param.media   = 0xFD; // media descriptor
              break;
              
            case ArduinoFDCClass::DT_5_HD:
              param.au_size = 512;  // bytes/cluster
              param.n_root  = 224;  // number of root directory entries
              param.media   = 0xF9; // media descriptor
              break;

            case ArduinoFDCClass::DT_3_DD:
              param.au_size = 1024; // bytes/cluster
              param.n_root  = 112;  // number of root directory entries
              param.media   = 0xF9; // media descriptor
              break;

            case ArduinoFDCClass::DT_3_HD:
              param.au_size = 512;  // bytes/cluster
              param.n_root  = 224;  // number of root directory entries
              param.media   = 0xF0; // media descriptor
              break;
            }
          
          if( confirm_formatting() )
            {
              byte st;
              ArduinoFDC.motorOn();
              f_unmount("0:");
              if( strstr(cmd, "/q") || (st=ArduinoFDC.formatDisk(FatFs.win))==S_OK )
                {
                  Serial.println(F("Initializing file system...\n"));
                  FRESULT fr = f_mkfs ("0:", &param, FatFs.win, 512);
                  if( fr != FR_OK ) print_ff_error(fr);
                }
              else
                print_error(st);
              
              f_mount(&FatFs, "0:", 0);
            }
        }
#ifdef USE_MONITOR
      else if( strncmp_P(cmd, PSTR("monitor"), max(3, strlen(cmd)))==0 )
        {
          motor_timeout = 0;
          monitor();
          f_mount(&FatFs, "0:", 0);
        }
#endif
#ifdef USE_XMODEM
      else if( strncmp_P(cmd, PSTR("receive "), 8)==0 )
        {
          ArduinoFDC.motorOn();
          fr = f_open(&FatFsFile, cmd+8, FA_WRITE | FA_CREATE_NEW);
          if( fr == FR_OK )
            {
              Serial.println(F("Send file via XModem now..."));
              
              XModem modem(recvChar, sendData, xmodemHandlerReceive);
              xmodem_status = FR_OK;
              modem.receive();

              if( xmodem_status == FR_OK )
                Serial.println(F("\r\nSuccess!"));
              else
                {
                  unsigned long t = millis() + 500;
                  while( millis() < t ) { if( Serial.read()>=0 ) t = millis()+500; }
                  while( Serial.read()<0 );
                  
                  Serial.println('\r');
                  if( xmodem_status!=S_OK ) print_ff_error(xmodem_status);
                }
              
              f_close(&FatFsFile);
            }
          else
            print_ff_error(fr);
        }
      else if( strncmp_P(cmd, PSTR("send "), 5)==0 )
        {
          ArduinoFDC.motorOn();
          fr = f_open(&FatFsFile, cmd+5, FA_READ);
          if( fr == FR_OK )
            {
              Serial.println(F("Receive file via XModem now..."));
              
              XModem modem(recvChar, sendData, xmodemHandlerSend);
              xmodem_status = FR_OK;
              modem.transmit();
              
              if( xmodem_status == FR_OK )
                Serial.println(F("\r\nSuccess!"));
              else
                {
                  unsigned long t = millis() + 500;
                  while( millis() < t ) { if( Serial.read()>=0 ) t = millis()+500; }
                  while( Serial.read()<0 );
                  
                  Serial.println('\r');
                  if( xmodem_status!=S_OK ) print_ff_error(xmodem_status);
                }
              
              f_close(&FatFsFile);
            }
          else
            print_ff_error(fr);
        }
#endif
#if !defined(USE_ARDUDOS) || !defined(USE_MONITOR) || !defined(USE_XMODEM) || defined(__AVR_ATmega2560__)
      // must save flash space if all three of ARDUDOS/MONITR/XMODEM are enabled on UNO
      else if( strcmp_P(cmd, PSTR("help"))==0 || strcmp_P(cmd, PSTR("h"))==0 || strcmp_P(cmd, PSTR("?"))==0 )
        {
          Serial.print(F("Valid commands: dir, type, dump, write, del, mkdir, rmdir, disktype, format"));
#ifdef USE_MONITOR
          Serial.print(F(", monitor"));
#endif
#ifdef USE_XMODEM
          Serial.print(F(", send, receive"));
#endif
          Serial.println();
        }
#endif
      else if( cmd[0]!=0 )
        {
          Serial.print(F("Unknown command: ")); 
          Serial.print(cmd);
        }

      motor_timeout = millis() + 5000;
    }
}

#endif


// -------------------------------------------------------------------------------------------------
// Low-level disk monitor
// -------------------------------------------------------------------------------------------------


#ifdef USE_MONITOR

// re-use the FatFs data buffer if ARDUDOS is enabled (to save RAM)
//#ifdef USE_ARDUDOS
//#define databuffer FatFs.win
//#else
//static byte databuffer[516];  // for mega
//#endif

static byte databuffer[BUFF_SIZE+384];  // for mega


#ifdef USE_XMODEM

#include "XModem.h"

byte xmodem_status_mon = S_OK;
bool xmodem_verify = false;
word xmodem_sector = 0, xmodem_data_ptr = 0xFFFF;


bool xmodemHandlerSendMon(unsigned long no, char* data, int size)
{
      byte numsec = ArduinoFDC.numSectors();
      byte numheads = ArduinoFDC.numHeads();
      byte buffer_pages;
      unsigned short offset; // header bytes to take out every 515 bytes
      unsigned short page_size = 512; // make this static
      byte page_shift = 9; // 2 ^9 = 512
      byte p_offset = 1; 
      byte l_offset = 3; // 515 = 512 + 3
      if ( ArduinoFDC.moduLation() == MFM ) {
      buffer_pages = numsec > 10 ? 1 : numsec;  // better 9 if numsec=18 // BUFSIZE verwenden !!
      } 
#ifdef WOZ
      else {   // GCR
      buffer_pages = numsec > 16 ? 1 : numsec;  // apple sectors  fit in 5.6 k
        page_size = 256;
        page_shift = 8; // 2 ^8 = 256
        p_offset = 86; 
        l_offset = 90; // 346 = 256 + 90
      }
#endif
//xmodem_data_prt is type word (16bit)
      //  2**9 = 512
     if( xmodem_data_ptr>=buffer_pages<<page_shift )    //  xmodem requires more data
    {
      //byte numheads = ArduinoFDC.numHeads();
      //byte numsec = ArduinoFDC.numSectors();

      if( xmodem_sector >= numheads*numsec*ArduinoFDC.numTracks() )  // 2 = number of hreads ?
        { xmodem_status_mon = S_OK; return false; }
     short int i,j,lbn; 
// loop over multiple sectors
     //for ( i=0 ; i < buffer_pages ; i++ ) {  // fill databuffer with buffer_pages sectors
     // with interleaving
      for ( i=0 ; i < buffer_pages ; i++ ) {  // fill databuffer with buffer_pages sectors
         j = i << 1; // skew factor 2
         j= (j>=numsec) ? ((j-numsec) | 0x01)  : j ;  //  interleaving, unevens after overflow
         lbn=xmodem_sector+j; // don't touch xmodem_sector
//#define mydebug
#ifdef mydebug
      Serial.print("j,lbn:");
      Serial.print(j);
      Serial.print(" ");
      Serial.println(lbn);
#endif
      
         byte head   = 0; byte track  = lbn / (numsec*numheads);   // heads
         byte sector = lbn % (numsec*numheads);   // heads
         if( sector >= numsec ) { head = 1; sector -= numsec; } // should not happen * numhead=1
      
         byte r = S_NOHEADER, retry = 5;
      //while( retry>0 && r!=S_OK ) { r = ArduinoFDC.readSector(track, head, sector+1, databuffer); retry--; }
#ifdef mydebug
      Serial.print('T');
      Serial.print(track);
      Serial.print('S');
      Serial.print(sector);
      Serial.print('H');
      Serial.println(head);
#endif
         //while( retry>0 && r!=S_OK ) { r = ArduinoFDC.readSector(track, head, sector+1, &databuffer[j*(page_size+p_offset)]) ; retry--; }
         while( retry>0 && r!=S_OK ) { r = ArduinoFDC.readSector(track, head, sector+1, &databuffer[j*(page_size+l_offset)]) ; retry--; }
#ifdef mydebug
       //short page_size = 512; // make this static
       //byte page_shift = 9;
       //byte p_offset = 1; 
       //byte l_offset = 3; // 515 = 512 + 3
      Serial.print("ret:");
      Serial.print(j);
      Serial.print(" ");
      Serial.println(r,HEX);
#endif
        // 513 bytes, record identifyer
         if( r!=S_OK ) { xmodem_status_mon = r; return false; } // better: continue ?
      
    }
// 
      xmodem_sector+= buffer_pages;
      xmodem_data_ptr = 0;
    }
      
  // "size" is always 128 and sector length is 512, i.e. a multiple of "size"
  // readSector returns data in databuffer[1..512]
  //memcpy(data, databuffer+1+xmodem_data_ptr, size);
     offset=(xmodem_data_ptr>>page_shift)*l_offset+p_offset;  // devide by 512, 3 bytes offset (515)

#ifdef mydebug
      Serial.print("O");
      Serial.print(offset);
      Serial.print(" ");
      Serial.println(xmodem_data_ptr,HEX);
#endif
  //  ???? xmodem_data_ptr now ptr ??
  memcpy(data, &databuffer[xmodem_data_ptr+offset], size);  // byte is unsigned
  xmodem_data_ptr += size;
  return true;
}


bool xmodemHandlerReceiveMon(unsigned long no, char* data, int size)
{
  // "size" is always 128 and sector length is 512, i.e. a multiple of "size"
  // writeSector expects data in databuffer[1..512]
  memcpy(databuffer+1+xmodem_data_ptr, data, size);
  xmodem_data_ptr += size;
   //xmodem_data_ptr is tyoe word (16 bit)
  if( xmodem_data_ptr>=512 )
    {
      byte numsec = ArduinoFDC.numSectors();
      byte numheads = ArduinoFDC.numHeads();
      if( xmodem_sector >= numheads*numsec*ArduinoFDC.numTracks() )
        { xmodem_status_mon = S_OK; return false; }
      
      byte head   = 0;
      byte track  = xmodem_sector / (numsec*numheads);
      byte sector = xmodem_sector % (numsec*numheads);
      if( sector >= numsec ) { head = 1; sector -= numsec; }  // not possible for numherads=1
      
      byte r = S_NOHEADER, retry = 5;
      while( retry>0 && r!=S_OK ) { r = ArduinoFDC.writeSector(track, head, sector+1, databuffer, xmodem_verify); retry--; }
      if( r!=S_OK ) { xmodem_status_mon = r; return false; }
      
      xmodem_data_ptr = 0;
      xmodem_sector++;
    }

  return true;
}

#endif


void monitor() 
{
  char cmd;
  int a1, a2, a3, head, track, sector, n;
  
  ArduinoFDC.motorOn();
  while( true )
    {
      Serial.print(F("\r\n\r\nCommand: "));
      n = sscanf(read_user_cmd(tempbuffer, 512), "%c%i,%i,%i", &cmd, &a1, &a2, &a3);
      if( n<=0 || isspace(cmd) ) continue;

      if( cmd=='r' && n>=3 )
        {
          track=a1; sector=a2; head= (n==3) ? a3 : 0 ;
          if( head>=0 && head<2 && track>=0 && track<ArduinoFDC.numTracks() && sector>=1 && sector<=ArduinoFDC.numSectors() )
            {
              Serial.print(F("Reading track ")); Serial.print(track); 
              Serial.print(F(" sector ")); Serial.print(sector);
              Serial.print(F(" side ")); Serial.println(head);
              Serial.flush();

              byte status = ArduinoFDC.readSector(track, head, sector, databuffer);
              if( status==S_OK )
                {
                  if (ArduinoFDC.moduLation() == MFM ) {
                     dump_buffer(0, databuffer+1,  512 ); 
                     }
#ifdef WOZ
                  else {    dump_buffer(0, &databuffer[86], 256  ); 
                       }
#endif
                  Serial.println();
                }
              else
                print_error(status);
            }
          else
            Serial.println(F("Invalid sector specification"));
        }
      else if( cmd=='r' && n==1 )
        {
          ArduinoFDC.motorOn();
          for(track=0; track<ArduinoFDC.numTracks(); track++)
            //for(head=0; head<2; head++)
            for(head=0; head<ArduinoFDC.numHeads(); head++)
              {
                sector = 1;
                for(byte i=0; i<ArduinoFDC.numSectors(); i++)
                  {
                    byte attempts = 0;
                    while( true )
                      {
                        Serial.print(F("Reading track ")); Serial.print(track); 
                        Serial.print(F(" sector ")); Serial.print(sector);
                        Serial.print(F(" side ")); Serial.print(head);
                        Serial.flush();
                        byte status = ArduinoFDC.readSector(track, head, sector, databuffer);
                        if( status==S_OK )
                          {
                            Serial.println(F(" => ok"));
                            break;
                          }
                        else if( (status==S_INVALIDID || status==S_CRC) && (attempts++ < 10) )
                          Serial.println(F(" => CRC error, trying again"));
                        else
                          {
                            Serial.print(F(" => "));
                            print_error(status);
                            break;
                          }
                      }

                    sector+=2;
                    if( sector>ArduinoFDC.numSectors() ) sector = 2;
                  }
              }
        }
#ifdef NIBBLES
      else if( cmd=='n' && n>=2 ) // read nibble data
        {
          track=a1;  
          int read_delay = 0 ;
          head= (n>1) ? a2 : 0 ; 
          if ( head > 5 ) { head = 0; read_delay = a2 ; } ;
          byte mode= (n>2) ? a3 : 1;
          byte maxtrack=ArduinoFDC.numTracks() ;
              Serial.print(F("Reading nibble track ")); Serial.print(track); 
              Serial.print(F(" side ")); Serial.println(head);
              Serial.print(F(" mode ")); Serial.println(mode);
              Serial.print(F(" maxtrack ")); Serial.println(maxtrack);
              Serial.print(F(" delay ")); Serial.println(read_delay);
              Serial.flush();
             
      // a3 = mode 1: nibble, 2:flux 
          if( head>=0 && head<2 && track>=0 && track<ArduinoFDC.numTracks()  )
            {

              byte status = ArduinoFDC.readNibbles(track, head, mode, read_delay, databuffer);
              if( status==S_OK )
                {
                  dump_buffer(0, databuffer, BUFF_SIZE); // all buffer
                  Serial.println();
                }
              else
                print_error(status);
            }
          else
            Serial.println(F("Invalid specification"));
          }
#endif
      else if( cmd=='w' && n>=3 )
        {
          track=a1; sector=a2; head= (n==3) ? a3 : 0 ; // was 0 : a3  
          if( head>=0 && head<ArduinoFDC.numHeads() && track>=0 && track<ArduinoFDC.numTracks() && sector>=1 && sector<=ArduinoFDC.numSectors() )
            {
              Serial.print(F("Writing and verifying track ")); Serial.print(track); 
              Serial.print(F(" sector ")); Serial.print(sector);
              Serial.print(F(" side ")); Serial.println(head);
              Serial.flush();
          
              byte status = ArduinoFDC.writeSector(track, head, sector, databuffer, true);
              if( status==S_OK )
                Serial.println(F("Ok."));
              else
                print_error(status);
            }
          else
            Serial.println(F("Invalid sector specification"));
        }
      else if( cmd=='w' && n>=1 )
        {
          bool verify = n>1 && a2>0;
          char c;
          Serial.print(F("Write current buffer to all sectors in drive "));
          Serial.write('A' + ArduinoFDC.selectedDrive());
          Serial.println(F(". Continue (y/n)?"));
          while( (c=Serial.read())<0 );
          if( c=='y' )
            {
              ArduinoFDC.motorOn();
              for(track=0; track<ArduinoFDC.numTracks(); track++)
                //for(head=0; head<2; head++)
                for(head=0; head<ArduinoFDC.numHeads(); head++)
                  {
                    sector = 1;
                    for(byte i=0; i<ArduinoFDC.numSectors(); i++)
                      {
                        Serial.print(verify ? F("Writing and verifying track ") : F("Writing track ")); Serial.print(track);
                        Serial.print(F(" sector ")); Serial.print(sector);
                        Serial.print(F(" side ")); Serial.print(head);
                        Serial.flush();
                        byte status = ArduinoFDC.writeSector(track, head, sector, databuffer, verify);
                        if( status==S_OK )
                          Serial.println(F(" => ok"));
                        else
                          {
                            Serial.print(F(" => "));
                            print_error(status);
                          }

                        sector+=2;
                        if( sector>ArduinoFDC.numSectors() ) sector = 2;
                      }
                  }
            }
        }
      else if( cmd=='b' )
        {
          Serial.println(F("Buffer contents:"));
          dump_buffer(0, databuffer+1, 512);
        }
      else if( cmd=='B' )
        {
          Serial.print(F("Filling buffer"));
          if( n==1 )
            {
              for(int i=0; i<512; i++) databuffer[i+1] = i;
            }
          else
            {
              Serial.print(F(" with 0x"));
              Serial.print(a1, HEX);
              for(int i=0; i<512; i++) databuffer[i+1] = a1;
            }
          Serial.println();
        }
      else if( cmd=='m' )
        {
          if( n==1 )
            {
              Serial.print(F("Drive "));
              Serial.write('A' + ArduinoFDC.selectedDrive());
              Serial.print(F(" motor is "));
              Serial.println(ArduinoFDC.motorRunning() ? F("on") : F("off"));
            }
          else
            {
              Serial.print(F("Turning drive "));
              Serial.write('A' + ArduinoFDC.selectedDrive());
              Serial.print(F(" motor "));
              if( n==1 || a1==0 )
                { 
                  Serial.println(F("off")); 
                  ArduinoFDC.motorOff();
                }
              else
                { 
                  Serial.println(F("on")); 
                  ArduinoFDC.motorOn();
                }
            }
        }
      else if( cmd=='s' )
        {
          if( n==1 )
            {
              Serial.print(F("Current drive is "));
              Serial.write('A' + ArduinoFDC.selectedDrive());
            }
          else
            {
              Serial.print(F("Selecting drive "));
              Serial.write(a1>0 ? 'B' : 'A');
              Serial.println();
              ArduinoFDC.selectDrive(n>1 && a1>0);
              ArduinoFDC.motorOn();
            }
        }
      else if( cmd=='t' && n>1 )
        {
          set_drive_type(a1);
        }
      else if( cmd=='f' )
        {
          if( confirm_formatting() )
            {
              Serial.println(F("Formatting disk..."));
              byte status = ArduinoFDC.formatDisk(databuffer, n>1 ? a1 : 0, n>2 ? a2 : 255);
              if( status!=S_OK ) print_error(status);
              memset(databuffer, 0, 513);
            }
        }
#ifdef USE_XMODEM
      else if( cmd=='R' )
        {
          Serial.println(F("Send image via XModem now..."));
          xmodem_status_mon = S_OK;
          xmodem_sector = 0;
          xmodem_data_ptr = 0;
          xmodem_verify = n>1 && (a1!=0);
          
          XModem modem(recvChar, sendData, xmodemHandlerReceiveMon);
          if( modem.receive() && xmodem_status_mon==S_OK )
            Serial.println(F("\r\nSuccess!"));
          else
            {
              unsigned long t = millis() + 500;
              while( millis() < t ) { if( Serial.read()>=0 ) t = millis()+500; }
              while( Serial.read()<0 );
              
              Serial.println('\r');
              if( xmodem_status_mon!=S_OK ) print_error(xmodem_status_mon);
            }
        }
      else if( cmd=='S' )
        {
          Serial.println(F("Receive image via XModem now..."));
          xmodem_status_mon = S_OK;
          xmodem_sector = 0;
          xmodem_data_ptr = 0xFFFF;

          ArduinoFDC.motorOn();  // avoid cyclinh motor in readSector

          XModem modem(recvChar, sendData, xmodemHandlerSendMon);
          if( modem.transmit() && xmodem_status_mon==S_OK )
            Serial.println(F("\r\nSuccess!"));
          else
            {
              unsigned long t = millis() + 500;
              while( millis() < t ) { if( Serial.read()>=0 ) t = millis()+500; }
              while( Serial.read()<0 );
              
              Serial.println('\r');
              if( xmodem_status_mon!=S_OK ) print_error(xmodem_status_mon);
            }
          ArduinoFDC.motorOff();  // avoid cyclinh motor in readSector
        }
      else if( cmd=='g' && n>=3 )  // change geomtry
        {
          if  (n!=3)  a3 = 0; // 0: don't change
            Serial.println(" new drive geometry for drive  ");
          ArduinoFDC.patchGeometry(a1,a2,a3);

        }

#endif
#ifdef USE_ARDUDOS
      else if (cmd=='x' )
        return;
#endif
#if !defined(USE_ARDUDOS) || !defined(USE_MONITOR) || !defined(USE_XMODEM) || defined(__AVR_ATmega2560__)
      // must save flash space if all three of ARDUDOS/MONITR/XMODEM are enabled on UNO
      else if( cmd=='h' || cmd=='?' )
        {
          Serial.println(F("Commands (t=track (0-based), s=sector (1-based), h=head (0/1)):"));
          Serial.println(F("r t,s,h  Read sector to buffer and print buffer"));
          Serial.println(F("r        Read ALL sectors and print pass/fail"));
          Serial.println(F("w t,s,h  Write buffer to sector"));
          Serial.println(F("w [0/1]  Write buffer to ALL sectors (without/with verify)"));
          Serial.println(F("b        Print buffer"));
          Serial.println(F("B [n]    Fill buffer with 'n' or 00..FF if n not given"));
          Serial.println(F("m 0/1    Turn drive motor off/on"));
          Serial.println(F("s 0/1    Select drive A/B"));
          Serial.println(F("t 0-6    Set type of current drive (5.25DD/5.25DDinHD/5.25HD/3.5DD/3.5HD/RX50/Apple][)"));
          Serial.println(F("f        Low-level format disk (tf)"));
          Serial.println(F("g t,s[,ts]  Modify drive geometry (Maxtracks, Sectors, Track-Spacing"));
#ifdef USE_XMODEM
          Serial.println(F("S        Read disk image and send via XModem"));
          Serial.println(F("R [0/1]  Receive disk image via XModem and write to disk (without/with verify)"));
#endif
#ifdef NIBBLES
          Serial.println(F("n t,d,m  dump nibbles, d: ms delay, m: mode (2: flux) \n"));
#endif
#ifdef USE_ARDUDOS
          Serial.println(F("x        Exit monitor\n"));
#endif
        }
#endif
      else
        Serial.println(F("Invalid command"));
    }
}

#endif

// -------------------------------------------------------------------------------------------------
// Main functions
// -------------------------------------------------------------------------------------------------


void setup() 
{
  //Serial.begin(115200);
  Serial.begin(230400); // max supported by mincom on OSX
#ifdef WOZ
  ArduinoFDC.begin(ArduinoFDCClass::DT_5_WOZ, ArduinoFDCClass::DT_3_HD);
#else
  ArduinoFDC.begin(ArduinoFDCClass::DT_3_HD, ArduinoFDCClass::DT_3_HD);
#endif

  // must save flash space if all three of ARDUDOS/MONITOR/XMODEM are enabled on UNO
#if !defined(USE_ARDUDOS) || !defined(USE_MONITOR) || !defined(USE_XMODEM) || defined(__AVR_ATmega2560__)
  Serial.print(F("Drive A: ")); print_drive_type(ArduinoFDC.getDriveType()); Serial.println();
  if( ArduinoFDC.selectDrive(1) )
    {
      Serial.print(F("Drive B: ")); print_drive_type(ArduinoFDC.getDriveType()); Serial.println();
      ArduinoFDC.selectDrive(0);
    }
#endif
}


void loop() 
{
#if defined(USE_ARDUDOS)
  arduDOS();
#elif defined(USE_MONITOR)
  monitor();
#else
#error "Need at least one of USE_ARDUDOS and USE_MONITOR"
#endif
}
