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

static byte data[516];


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


static String read_user_cmd()
{
  String s;
  do
    {
      int i = Serial.read();

      if( (i==13 || i==10) && s!="" )
        { Serial.println(); break; }
      else if( i==27 )
        { s = ""; Serial.println(); break; }
      else if( i==8 )
        { 
          if( s.length()>0 )
            { Serial.write(8); Serial.write(' '); Serial.write(8); s = s.substring(0, s.length()-1); }
        }
      else if( isprint(i) )
        { s = s + String((char )i); Serial.write(i); }
    }
  while(true);

  return s;
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
    case S_READONLY  : Serial.print(F("Disk is write protected")); break;
    default          : Serial.print(F("Unknonwn error")); break;
    }
}


void print_drive_type(byte n)
{
  switch( n )
    {
    case ArduinoFDCClass::DT_5_DD: Serial.print(F("5.25\" double density")); break;
    case ArduinoFDCClass::DT_5_DDonHD: Serial.print(F("5.25\" double density disk in high density drive")); break;
    case ArduinoFDCClass::DT_5_HD: Serial.print(F("5.25\" high density")); break;
    case ArduinoFDCClass::DT_3_DD: Serial.print(F("3.5\" double density")); break;
    case ArduinoFDCClass::DT_3_HD: Serial.print(F("3.5\" high density")); break;
    default: Serial.print(F("Unknown"));
    }
}


void setup() 
{
  Serial.begin(115200);
  ArduinoFDC.begin();

  Serial.print("Drive A is: "); print_drive_type(ArduinoFDC.getDriveType()); Serial.println();
  if( ArduinoFDC.selectDrive(1) )
    {
      Serial.print("Drive B is: "); print_drive_type(ArduinoFDC.getDriveType()); Serial.println();
      ArduinoFDC.selectDrive(0);
    }

  ArduinoFDC.motorOn();
}



void loop() 
{
  char cmd;
  int a1, a2, a3, head, track, sector, n;

  Serial.setTimeout(1000000);
  Serial.print(F("\r\n\r\nCommand: "));
  String s = read_user_cmd();
  n = sscanf(s.c_str(), "%c%i,%i,%i", &cmd, &a1, &a2, &a3);
  if( n<=0 || isspace(cmd) ) return;

  if( cmd=='r' && n>=3 )
    {
      track=a1; sector=a2; head= (n==3) ? 0 : a3;
      if( head>=0 && head<2 && track>=0 && track<ArduinoFDC.numTracks() && sector>=1 && sector<=ArduinoFDC.numSectors() )
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
      ArduinoFDC.motorOn();
      for(track=0; track<ArduinoFDC.numTracks(); track++)
        for(head=0; head<2; head++)
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

                sector+=2;
                if( sector>ArduinoFDC.numSectors() ) sector = 2;
              }
          }
    }
  else if( cmd=='w' && n>=3 )
    {
      track=a1; sector=a2; head= (n==3) ? 0 : a3;
      if( head>=0 && head<2 && track>=0 && track<ArduinoFDC.numTracks() && sector>=1 && sector<=ArduinoFDC.numSectors() )
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
          ArduinoFDC.motorOn();
          for(track=0; track<ArduinoFDC.numTracks(); track++)
            for(head=0; head<2; head++)
              {
                sector = 1;
                for(byte i=0; i<ArduinoFDC.numSectors(); i++)
                  {
                    Serial.print(verify ? F("Writing and verifying track ") : F("Writing track ")); Serial.print(track);
                    Serial.print(F(" sector ")); Serial.print(sector);
                    Serial.print(F(" side ")); Serial.print(head);
                    Serial.flush();
                    byte status = ArduinoFDC.writeSector(track, head, sector, data, verify);
                    if( status==S_OK )
                      Serial.println(F(" => ok"));
                    else
                      {
                        Serial.print(F(" => ERROR: "));
                        print_error(status);
                        Serial.println();
                      }

                    sector+=2;
                    if( sector>ArduinoFDC.numSectors() ) sector = 2;
                  }
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
                  Serial.flush();

                  byte secPerCluster, secPerFat, maxDirEntries, mediaDesc, numReserved;
                  byte secPerTrack =  ArduinoFDC.numSectors();
                  uint16_t totalSec = secPerTrack * ArduinoFDC.numTracks() * 2;
                  switch( ArduinoFDC.getDriveType() )
                    {
                    case ArduinoFDCClass::DT_5_DD:
                    case ArduinoFDCClass::DT_5_DDonHD:
                      secPerCluster = 2;
                      secPerFat     = 2;
                      maxDirEntries = 112;
                      mediaDesc     = 0xFD;
                      numReserved   = 1 + 2*2 + 7;
                      break;

                    case ArduinoFDCClass::DT_5_HD:
                      secPerCluster = 1;
                      secPerFat     = 7;
                      maxDirEntries = 224;
                      mediaDesc     = 0xF9;
                      numReserved   = 1 + 2*7 + 14;
                      break;

                    case ArduinoFDCClass::DT_3_DD:
                      secPerCluster = 2;
                      secPerFat     = 3;
                      maxDirEntries = 112;
                      mediaDesc     = 0xF9;
                      numReserved   = 1 + 2*3 + 7;
                      break;

                    case ArduinoFDCClass::DT_3_HD:
                      secPerCluster = 1;
                      secPerFat     = 9;
                      maxDirEntries = 224;
                      mediaDesc     = 0xF0;
                      numReserved   = 1 + 2*9 + 14;
                      break;
                    }
                  
                  for(int i=0; i<512; i++) data[i+1] = pgm_read_byte_near(bootsector+i);
                  data[13+1] = secPerCluster;
                  data[17+1] = maxDirEntries;
                  data[19+1] = totalSec & 255;
                  data[20+1] = totalSec / 256;
                  data[21+1] = mediaDesc;
                  data[22+1] = secPerFat;
                  data[24+1] = secPerTrack;
                  ArduinoFDC.writeSector(0, 0, 1, data, false);
                  
                  memset(data+1, 0x00, 512);
                  for(int i=1; i<numReserved; i++)
                    {
                      bool isFatStart = (i==1) || (i==1+secPerFat);
                      data[1] = isFatStart ? mediaDesc : 0x00;
                      data[2] = isFatStart ? 0xFF : 0x00;
                      data[3] = isFatStart ? 0xFF : 0x00;
                      ArduinoFDC.writeSector(0, i/secPerTrack, (i%secPerTrack)+1, data, false);
                    }
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
  else if( cmd=='t' && n>0 )
    {
      ArduinoFDC.setDriveType(a1);
      Serial.print(F("Setting type of drive ")); Serial.print(ArduinoFDC.selectedDrive()); 
      Serial.print(F(" to: ")); print_drive_type(ArduinoFDC.getDriveType()); Serial.println();
    }
  else if( cmd=='c' )
    {
      char c;
      Serial.println(F("Copying drive A to B will overwrite all sectors on drive B. Continue (y/n)?"));
      while( (c=Serial.read())<0 );
      if( c=='y' )
        {
          if( !ArduinoFDC.selectDrive(1) )
            {
              Serial.println(F("PIN_MOTORB and/or PIN_SELECTB not set in ArduinoFDC.cpp"));
              return;
            }

          ArduinoFDC.motorOn();
          byte t = ArduinoFDC.getDriveType();
          ArduinoFDC.selectDrive(0);
          ArduinoFDC.motorOn();
          
          if( ArduinoFDC.getDriveType() != t )
            {
              Serial.println(F("Can only copy between drives of the same type"));
              return;
            }

          for(track=0; track<ArduinoFDC.numTracks(); track++)
            for(head=0; head<2; head++)
              {
                sector = 1;
                for(byte i=0; i<ArduinoFDC.numSectors(); i++)
                  {
                    ArduinoFDC.selectDrive(0);
                    byte status, attempts = 0;
                    while( true )
                      {
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

                sector+=2;
                if( sector>ArduinoFDC.numSectors() ) sector = 2;
              }
        }
    }
  else if( cmd=='h' || cmd=='?' )
    {
      Serial.println(F("Supported commands:"));
      Serial.println(F("r track, sector, side     Read sector to buffer and print buffer"));
      Serial.println(F("r                         Read ALL sectors and print pass/fail"));
      Serial.println(F("w track, sector, side     Write buffer to sector"));
      Serial.println(F("w [0/1]                   Write buffer to ALL sectors (without/with verify)"));
      Serial.println(F("b                         Print buffer"));
      Serial.println(F("B [n]                     Fill buffer with 'n' or 00..FF if n not given"));
      Serial.println(F("m 0/1                     Turn drive motor off/on"));
      Serial.println(F("s 0/1                     Select drive A/B"));
      Serial.println(F("t 0/1/2/3/4               Set type of current drive (3.5DD/3.5DDinHD/5.25DD/5.25HD)"));
      Serial.println(F("f [0/1]                   Format disk (without/with initializing DOS file system)"));
      Serial.println(F("c [0/1]                   Copy contents of disk in drive A to disk in drive B (verify off/on)"));
      
    }
  else
    Serial.println(F("Invalid command"));
}
