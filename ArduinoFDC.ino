// -----------------------------------------------------------------------------
// Double Density (DD, 720k) 3.5" disk controller for Arduino
// Copyright (C) 2020 David Hansel
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

static byte data[515];


static const uint8_t PROGMEM bootsector[512] = {
  0xEB, 0x3E, 0x90, 0x2F, 0x29, 0x61, 0x6A, 0x6C, 0x49, 0x48, 0x43, 0x00, 0x02, 0x02, 0x01, 0x00, 
  0x02, 0x70, 0x00, 0xA0, 0x05, 0xF9, 0x03, 0x00, 0x09, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29, 0x56, 0x0C, 0xE6, 0x14, 0x4E, 0x4F, 0x20, 0x4E, 0x41, 
  0x4D, 0x45, 0x20, 0x20, 0x20, 0x20, 0x46, 0x41, 0x54, 0x31, 0x32, 0x20, 0x20, 0x20, 0xF1, 0x7D, 
  0xFA, 0x33, 0xC9, 0x8E, 0xD1, 0xBC, 0xFC, 0x7B, 0x16, 0x07, 0xBD, 0x78, 0x00, 0xC5, 0x76, 0x00, 
  0x1E, 0x56, 0x16, 0x55, 0xBF, 0x22, 0x05, 0x89, 0x7E, 0x00, 0x89, 0x4E, 0x02, 0xB1, 0x0B, 0xFC, 
  0xF3, 0xA4, 0x06, 0x1F, 0xBD, 0x00, 0x7C, 0xC6, 0x45, 0xFE, 0x0F, 0x8B, 0x46, 0x18, 0x88, 0x45, 
  0xF9, 0xFB, 0x38, 0x66, 0x24, 0x7C, 0x04, 0xCD, 0x13, 0x72, 0x3C, 0x8A, 0x46, 0x10, 0x98, 0xF7, 
  0x66, 0x16, 0x03, 0x46, 0x1C, 0x13, 0x56, 0x1E, 0x03, 0x46, 0x0E, 0x13, 0xD1, 0x50, 0x52, 0x89, 
  0x46, 0xFC, 0x89, 0x56, 0xFE, 0xB8, 0x20, 0x00, 0x8B, 0x76, 0x11, 0xF7, 0xE6, 0x8B, 0x5E, 0x0B, 
  0x03, 0xC3, 0x48, 0xF7, 0xF3, 0x01, 0x46, 0xFC, 0x11, 0x4E, 0xFE, 0x5A, 0x58, 0xBB, 0x00, 0x07, 
  0x8B, 0xFB, 0xB1, 0x01, 0xE8, 0x94, 0x00, 0x72, 0x47, 0x38, 0x2D, 0x74, 0x19, 0xB1, 0x0B, 0x56, 
  0x8B, 0x76, 0x3E, 0xF3, 0xA6, 0x5E, 0x74, 0x4A, 0x4E, 0x74, 0x0B, 0x03, 0xF9, 0x83, 0xC7, 0x15, 
  0x3B, 0xFB, 0x72, 0xE5, 0xEB, 0xD7, 0x2B, 0xC9, 0xB8, 0xD8, 0x7D, 0x87, 0x46, 0x3E, 0x3C, 0xD8, 
  0x75, 0x99, 0xBE, 0x80, 0x7D, 0xAC, 0x98, 0x03, 0xF0, 0xAC, 0x84, 0xC0, 0x74, 0x17, 0x3C, 0xFF, 
  0x74, 0x09, 0xB4, 0x0E, 0xBB, 0x07, 0x00, 0xCD, 0x10, 0xEB, 0xEE, 0xBE, 0x83, 0x7D, 0xEB, 0xE5, 
  0xBE, 0x81, 0x7D, 0xEB, 0xE0, 0x33, 0xC0, 0xCD, 0x16, 0x5E, 0x1F, 0x8F, 0x04, 0x8F, 0x44, 0x02, 
  0xCD, 0x19, 0xBE, 0x82, 0x7D, 0x8B, 0x7D, 0x0F, 0x83, 0xFF, 0x02, 0x72, 0xC8, 0x8B, 0xC7, 0x48, 
  0x48, 0x8A, 0x4E, 0x0D, 0xF7, 0xE1, 0x03, 0x46, 0xFC, 0x13, 0x56, 0xFE, 0xBB, 0x00, 0x07, 0x53, 
  0xB1, 0x04, 0xE8, 0x16, 0x00, 0x5B, 0x72, 0xC8, 0x81, 0x3F, 0x4D, 0x5A, 0x75, 0xA7, 0x81, 0xBF, 
  0x00, 0x02, 0x42, 0x4A, 0x75, 0x9F, 0xEA, 0x00, 0x02, 0x70, 0x00, 0x50, 0x52, 0x51, 0x91, 0x92, 
  0x33, 0xD2, 0xF7, 0x76, 0x18, 0x91, 0xF7, 0x76, 0x18, 0x42, 0x87, 0xCA, 0xF7, 0x76, 0x1A, 0x8A, 
  0xF2, 0x8A, 0x56, 0x24, 0x8A, 0xE8, 0xD0, 0xCC, 0xD0, 0xCC, 0x0A, 0xCC, 0xB8, 0x01, 0x02, 0xCD, 
  0x13, 0x59, 0x5A, 0x58, 0x72, 0x09, 0x40, 0x75, 0x01, 0x42, 0x03, 0x5E, 0x0B, 0xE2, 0xCC, 0xC3, 
  0x03, 0x18, 0x01, 0x27, 0x0D, 0x0A, 0x49, 0x6E, 0x76, 0x61, 0x6C, 0x69, 0x64, 0x20, 0x73, 0x79, 
  0x73, 0x74, 0x65, 0x6D, 0x20, 0x64, 0x69, 0x73, 0x6B, 0xFF, 0x0D, 0x0A, 0x44, 0x69, 0x73, 0x6B, 
  0x20, 0x49, 0x2F, 0x4F, 0x20, 0x65, 0x72, 0x72, 0x6F, 0x72, 0xFF, 0x0D, 0x0A, 0x52, 0x65, 0x70, 
  0x6C, 0x61, 0x63, 0x65, 0x20, 0x74, 0x68, 0x65, 0x20, 0x64, 0x69, 0x73, 0x6B, 0x2C, 0x20, 0x61, 
  0x6E, 0x64, 0x20, 0x74, 0x68, 0x65, 0x6E, 0x20, 0x70, 0x72, 0x65, 0x73, 0x73, 0x20, 0x61, 0x6E, 
  0x79, 0x20, 0x6B, 0x65, 0x79, 0x0D, 0x0A, 0x00, 0x49, 0x4F, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
  0x53, 0x59, 0x53, 0x4D, 0x53, 0x44, 0x4F, 0x53, 0x20, 0x20, 0x20, 0x53, 0x59, 0x53, 0x80, 0x01, 
  0x00, 0x57, 0x49, 0x4E, 0x42, 0x4F, 0x4F, 0x54, 0x20, 0x53, 0x59, 0x53, 0x00, 0x00, 0x55, 0xAA
};

  

inline void print_hex(byte b)
{
  if( b<16 ) Serial.write('0');
  Serial.print(b, HEX);
}


static void dump_buffer(byte *buf, int n)
{
  int offset = 0;
  while( offset<n )
    {
      print_hex(offset/256); 
      print_hex(offset&255); 
      Serial.write(':');

      for(int i=0; i<16; i++)
        {
          if( (i&7)==0  ) Serial.write(' ');
          print_hex(buf[offset+i]);
          Serial.write(' ');
        }

      Serial.write(' ');
      for(int i=0; i<16; i++)
        {
          if( (i&7)==0  ) Serial.write(' ');
          Serial.write(isprint(buf[offset+i]) ? buf[offset+i] : '.');
        }

      Serial.println();
      offset += 16;
    }
}


void print_error(byte n)
{
  switch( n )
    {
    case S_OK        : Serial.print(F("No error")); break;
    case S_NOTINIT   : Serial.print(F("ArduinoFDC.begin() was not called")); break;
    case S_NOTREADY  : Serial.print(F("Drive not ready")); break;
    case S_NOSYNC    : Serial.print(F("No sync marks found")); break;
    case S_NOHEADER  : Serial.print(F("Sector header not found")); break;
    case S_INVALIDID : Serial.print(F("Data record has unexpected id")); break;
    case S_CRC       : Serial.print(F("Data checksum error")); break;
    case S_NOINDEX   : Serial.print(F("No index hole detected")); break;
    case S_NOTRACK0  : Serial.print(F("No track 0 signal detected")); break;
    case S_VERIFY    : Serial.print(F("Verify after write failed")); break;
    default          : Serial.print(F("Unknonwn error")); break;
    }
}


void setup() 
{
  Serial.begin(115200);
  ArduinoFDC.begin();
  ArduinoFDC.motorOn();
}



void loop() 
{
  char cmd;
  int a1, a2, a3, head, track, sector, n;

  Serial.setTimeout(1000000);
  Serial.print(F("\r\n\r\nCommand: "));
  String s = Serial.readStringUntil('\n');
  Serial.println(s);
  n = sscanf(s.c_str(), "%c%i,%i,%i", &cmd, &a1, &a2, &a3);
  if( n<=0 || isspace(cmd) ) return;

  if( cmd=='r' && n>=3 )
    {
      track=a1; sector=a2; head= (n==3) ? 0 : a3;
      if( head>=0 && head<2 && track>=0 && track<80 && sector>=1 && sector<10 )
        {
          Serial.print(F("Reading track ")); Serial.print(track); 
          Serial.print(F(" sector ")); Serial.print(sector);
          Serial.print(F(" side ")); Serial.println(head);
          Serial.flush();

          byte status = ArduinoFDC.readSector(track, head, sector, data);
          if( status==S_OK )
            {
              dump_buffer(data+1, 512);
              Serial.println();
            }
          else
            { Serial.print(F("Error: ")); print_error(status); Serial.println('!'); }
        }
      else
        Serial.println(F("Invalid sector specification"));
    }
  else if( cmd=='r' && n==1 )
    {
      byte sectors[9] = {1,3,5,7,9,2,4,6,8};
      ArduinoFDC.motorOn();
      for(track=0; track<80; track++)
        for(head=0; head<2; head++)
          for(byte i=0; i<9; i++)
            {
              sector = sectors[i];
              byte attempts = 0;
              while( true )
                {
                  sector = sectors[i];
                  Serial.print(F("Reading track ")); Serial.print(track); 
                  Serial.print(F(" sector ")); Serial.print(sector);
                  Serial.print(F(" side ")); Serial.print(head);
                  Serial.flush();
                  byte status = ArduinoFDC.readSector(track, head, sector, data);
                  if( status==S_OK )
                    {
                      Serial.println(F(" => ok"));
                      break;
                    }
                  else if( (status==S_INVALIDID || status==S_CRC) && (attempts++ < 10) )
                    Serial.println(F(" => CRC error, trying again"));
                  else
                    {
                      Serial.print(F("=> Error: ")); print_error(status); Serial.println('!');
                      break;
                    }
                }
            }
    }
  else if( cmd=='w' && n>=3 )
    {
      track=a1; sector=a2; head= (n==3) ? 0 : a3;
      if( head>=0 && head<2 && track>=0 && track<80 && sector>=1 && sector<10 )
        {
          Serial.print(F("Writing and verifying track ")); Serial.print(track); 
          Serial.print(F(" sector ")); Serial.print(sector);
          Serial.print(F(" side ")); Serial.println(head);
          Serial.flush();
          
          byte status = ArduinoFDC.writeSector(track, head, sector, data, true);
          if( status==S_OK )
            Serial.println(F("Ok."));
          else
            { Serial.print(F("Error: ")); print_error(status); Serial.println('!'); }
        }
      else
        Serial.println(F("Invalid sector specification"));
    }
  else if( cmd=='w' && n>=1 )
    {
      bool verify = n>1 && a2>0;
      char c;
      Serial.print(F("This will write the current buffer content to all sectors on the disk in drive "));
      Serial.write('A' + ArduinoFDC.selectedDrive());
      Serial.println(F(". Continue (y/n)?"));
      while( (c=Serial.read())<0 );
      if( c=='y' )
        {
          byte sectors[9] = {1,3,5,7,9,2,4,6,8};
          ArduinoFDC.motorOn();
          for(track=0; track<80; track++)
            for(head=0; head<2; head++)
              for(byte i=0; i<9; i++)
                {
                  sector = sectors[i];
                  Serial.print(verify ? F("Writing and verifying track ") : F("Writing track ")); Serial.print(track);
                  Serial.print(F(" sector ")); Serial.print(sector);
                  Serial.print(F(" side ")); Serial.print(head);
                  Serial.flush();
                  if( ArduinoFDC.writeSector(track, head, sector, data, verify)==S_OK )
                    Serial.println(F(" => ok"));
                  else
                    Serial.println(F(" => ERROR writing sector"));
                }
        }
    }
  else if( cmd=='f' && n>=1 )
    {
      char c;
      Serial.print(F("Formatting will erase all data on the disk in drive "));
      Serial.write('A' + ArduinoFDC.selectedDrive());
      Serial.println(F(". Continue (y/n)?"));
      while( (c=Serial.read())<0 );
      if( c=='y' )
        {
          Serial.println(F("Formatting disk..."));
          byte status = ArduinoFDC.formatDisk();
          if( status==S_OK )
            {
              if( n>1 && a1)
                {
                  Serial.println(F("Initializing DOS file system..."));
                  
                  for(int i=0; i<512; i++) data[i+1] = pgm_read_byte_near(bootsector+i);
                  ArduinoFDC.writeSector(0, 0, 1, data, false);
                  
                  memset(data+1, 0x00, 512);
                  ArduinoFDC.writeSector(0, 0, 3, data, false);
                  ArduinoFDC.writeSector(0, 0, 6, data, false);
                  ArduinoFDC.writeSector(0, 0, 8, data, false);
                  ArduinoFDC.writeSector(0, 0, 4, data, false);
                  ArduinoFDC.writeSector(0, 0, 7, data, false);
                  ArduinoFDC.writeSector(0, 0, 9, data, false);
                  
                  data[1]=0xF9; data[2]=0xFF; data[3]=0xFF;
                  ArduinoFDC.writeSector(0, 0, 2, data, false);
                  ArduinoFDC.writeSector(0, 0, 5, data, false);
                }
            }
          else
            { Serial.print(F("Error: ")); print_error(status); Serial.println('!'); }
        }
    }
  else if( cmd=='b' )
    {
      Serial.println(F("Buffer contents:"));
      dump_buffer(data+1, 512);
    }
  else if( cmd=='B' )
    {
      Serial.print(F("Filling buffer"));
      if( n==1 )
        {
          for(int i=0; i<512; i++) data[i+1] = i;
        }
      else
        {
          Serial.print(F(" with 0x"));
          Serial.print(a1, HEX);
          for(int i=0; i<512; i++) data[i+1] = a1;
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
          Serial.print(F("Drive "));
          Serial.write('A' + ArduinoFDC.selectedDrive());
          Serial.println(F(" is currently selected."));
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
  else if( cmd=='c' )
    {
      char c;
      Serial.println(F("Copying drive A to B will overwrite all sectors on drive B. Continue (y/n)?"));
      while( (c=Serial.read())<0 );
      if( c=='y' )
        {
          byte sectors[9] = {1,6,2,7,3,8,4,9,5};
          ArduinoFDC.selectDrive(1);
          ArduinoFDC.motorOn();
          ArduinoFDC.selectDrive(0);
          ArduinoFDC.motorOn();
          for(track=0; track<80; track++)
            for(head=0; head<2; head++)
              for(byte i=0; i<9; i++)
                {
                  ArduinoFDC.selectDrive(0);
                  byte status, attempts = 0;
                  while( true )
                    {
                      sector = sectors[i];
                      Serial.print(F("Reading drive A track ")); Serial.print(track); 
                      Serial.print(F(" sector ")); Serial.print(sector);
                      Serial.print(F(" side ")); Serial.print(head);
                      Serial.flush();
                      status = ArduinoFDC.readSector(track, head, sector, data);
                      if( status==S_OK )
                        {
                          Serial.println(F(" => ok"));
                          break;
                        }
                      else if( (status==S_INVALIDID || status==S_CRC) && (attempts++ < 5) )
                        Serial.println(F(" => CRC error, trying again"));
                      else
                        {
                          Serial.print(F("Error: ")); print_error(status); Serial.println('!');
                          break;
                        }
                    }
                  
                  if( status==S_OK )
                    {
                      attempts = 0;
                      while( true )
                        {
                          Serial.print(F("Writing drive B track ")); Serial.print(track); 
                          Serial.print(F(" sector ")); Serial.print(sector);
                          Serial.print(F(" side ")); Serial.print(head);
                          Serial.flush();
                          
                          ArduinoFDC.selectDrive(1);
                          status = ArduinoFDC.writeSector(track, head, sector, data, n>1 && a1>0);
                          if( status==S_OK )
                            { 
                              Serial.println(F(" => ok")); 
                              break; 
                            }
                          else if( (status==S_VERIFY) && (attempts++ < 5) )
                            Serial.println(F(" => verify error, trying again"));
                          else
                            { 
                              Serial.print(F("Error: ")); print_error(status); Serial.println('!'); 
                              break;
                            }
                        }
                    }
                }
        }
    }
  else if( cmd=='h' )
    {
      Serial.println(F("Supported commands:"));
      Serial.println(F("r track, sector, side     Read sector to buffer and print buffer"));
      Serial.println(F("r                         Read ALL sectors and print pass/fail"));
      Serial.println(F("w track, sector, side     Write buffer to sector"));
      Serial.println(F("w [0/1]                   Write buffer to ALL sectors without/with verify"));
      Serial.println(F("b                         Print buffer"));
      Serial.println(F("B [n]                     Fill buffer with 'n' or 00..FF if n not given"));
      Serial.println(F("m 0/1                     Turn drive motor off/on"));
      Serial.println(F("s 0/1                     Select drive A/B"));
      Serial.println(F("c [0/1]                   Copy contents of disk in drive A to disk in drive B (verify off/on)"));
    }
  else
    Serial.println(F("Invalid command"));
}
