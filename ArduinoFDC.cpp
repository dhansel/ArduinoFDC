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

// instruct the compiler to optimize this code for performance, not size
#pragma GCC optimize ("-O2")

// input/output pin definitions
#define PIN_STEP       2
#define PIN_STEPDIR    3
#define PIN_MOTORA     4
#define PIN_SELECTA    5
#define PIN_SIDE       6
#define PIN_INDEX      7  // hardwired to pin 7 (PD7) in function format_track()
#define PIN_READDATA   8  // must be pin 8 (ICP1 for timer1)
#define PIN_WRITEDATA  9  // must be pin 9 (OCP1 for timer1)
#define PIN_WRITEGATE 10  // hardwired to pin 10 (PB2) in functions writedata() and format_track()
#define PIN_TRACK0    11
#define PIN_MOTORB    12  // only used if SINGLEDRIVE is NOT defined below
#define PIN_SELECTB   13  // only used if SINGLEDRIVE is NOT defined below


// convert microseconds to timer ticks (prescaler 1 => timer is running at CPU clock speed)
#define US2TICKS(n) ((n)*(F_CPU/1000000))

// un-commenting this will avoid using Arduino pins 12+13 but only support one drive
//#define SINGLEDRIVE

// un-commenting this will write more detailed error information to Serial
//#define DEBUG

ArduinoFDCClass ArduinoFDC;

static byte prevbit;
static byte header[7];


static const uint16_t PROGMEM crc16_table[256] =
{
 0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
 0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, 0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
 0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485, 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
 0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4, 0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
 0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823, 0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
 0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
 0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
 0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70, 0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
 0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F, 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
 0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
 0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, 0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
 0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C, 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
 0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB, 0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
 0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A, 0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
 0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
 0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};


static uint16_t calc_crc(byte *buf, int n)
{
  // already includes sync marks (0xA1, 0xA1, 0xA1)
  uint16_t crc = 0xCDB4;

  // compute CRC of remaining data
  while( n-- > 0 )
    crc = pgm_read_word_near(crc16_table + (((crc >> 8) ^ *buf++) & 0xff)) ^ (crc << 8);

  return crc;
}


static bool check_pulse()
{
  // reset timer and capture/overrun flags
  TCNT1 = 0;
  TIFR1 = bit(ICF1) | bit(TOV1);

  // wait for either input capture or timer overrun
  while( !(TIFR1 & (bit(ICF1) | bit(TOV1))) );

  // if there was an input capture then we are ok
  bool res = (TIFR1 & bit(ICF1))!=0;

  // reset input capture and timer overun flags
  TIFR1 = bit(ICF1) | bit(TOV1);

  return res;
}


static bool wait_index_hole()
{
  // reset timer and overrun flags
  TCNT1 = 0;
  TIFR1 = bit(TOV1);
  byte ctr = 0;

  // wait for END of index hole (in case we're on the hole right now)
  while( !(PIND & 0x80) )
    {
      if( TIFR1 & bit(TOV1) )
        {
          // timer overflow happens every 4.096ms (65536 cycles at 16MHz)
          // meaning we haven't found a sync in that amount of time
          if( ++ctr == 0 )
            {
              // we have tried for 256 * 4.096ms = 1.048 seconds to find a index hole
              // one rotation is 200ms so we have tried for 5 full rotations => give up
#ifdef DEBUG
              Serial.println(F("No index hole signal!")); Serial.flush();
#endif
              return false;
            }
          
          // clear overflow flag
          TIFR1 = bit(TOV1);
        }
    }

  // wait for START of index hole (same as above)
  ctr = 0;
  while( (PIND & 0x80) )
    {
      if( TIFR1 & bit(TOV1) )
        {
          if( ++ctr == 0 ) 
            {
#ifdef DEBUG
              Serial.println(F("No index hole signal!")); Serial.flush();
#endif
              return false;
            }

          TIFR1 = bit(TOV1);
        }
    }

  return true;
}


inline byte read_pulse()
{
  static uint16_t prevtime = 0;
  uint16_t t, ticks;
  
  // wait for input capture
  while( !(TIFR1 & bit(ICF1)) );
  TIFR1 |= bit(ICF1);

  // get number of clock cycles since previous capture
  t = ICR1;
  ticks = t - prevtime;
  prevtime = t;

  // convert to 1/2/3 pulse (short/medium/long)
  byte n = 1;
  if( ticks>US2TICKS(7) )
    n = 3;
  else if( ticks>US2TICKS(5) )
    n = 2;
  
  return n;
}


inline bool read_syncmark()
{
  if( read_pulse() != 3 ) return false;
  if( read_pulse() != 2 ) return false;
  if( read_pulse() != 3 ) return false;
  if( read_pulse() != 2 ) return false;
  return true;
}


inline bool wait_sync()
{
  byte n, p, ctr;

  // expect at least 10 bytes of 0x00 followed by three sync marks (0xA1 with one missing clock bit)
  // Data bits :     0 0 ...0  1 0 1 0 0*0 0 1  1 0 1 0 0*0 0 1  1 0 1 0 0*0 0 1
  // In MFM    : (0)1010...10 0100010010001001 0100010010001001 0100010010001001

  // reset timer
  TCNT1 = 0;
  TIFR1 = bit(TOV1);
  ctr = 0;

start:
  if( TIFR1 & bit(TOV1) )
    {
      // timer overflow happens every 4.096ms (65536 cycles at 16MHz)
      // meaning we haven't found a sync in that amount of time
      if( ++ctr == 0 )
        {
#ifdef DEBUG
          Serial.println(F("No Sync!")); Serial.flush();
#endif
          // we have tried for 256 * 4.096ms = 1.048 seconds to find a sync
          // one rotation is 200ms so we have tried for 5 full rotations => give up
          return false;
        }

      // clear overflow flag
      TIFR1 = bit(TOV1);
    }

  n = 0;
  do { p = read_pulse(); n++; } while( p==1 ); // 0101...01
  if( p != 2 || n<80    ) goto start;          // 001
  if( !read_syncmark()  ) goto start;          // 00010010001001
  if( read_pulse() != 1 ) goto start;          // 01
  if( !read_syncmark()  ) goto start;          // 00010010001001
  if( read_pulse() != 1 ) goto start;          // 01
  if( !read_syncmark()  ) goto start;          // 00010010001001

  return true;
}



static bool read_data(byte *ptr, int n, bool verify)
{
  // we only ever enter this after a sync (for which the last data bit is "1")
  byte even = 1;
  int8_t bitctr = 8;
  uint16_t d = 0;

  prevbit = 0;
  while( n>0 )
    {
      byte p = read_pulse();
      if( even )
        {
          // no previous un-processed bits
          if( p==1 ) // 01
            {
              // read "1", still even
              d = (d*2) | 1;
              bitctr--;
            }
          else if( p==2 ) // 001
            {
              // read "0", now odd
              d = (d*2) | 0;
              bitctr--;
              even = false;
            }
          else if( p==3 ) // 0001
            {
              // read "0" followed by "1", still even
              d = (d*4) | 01;
              bitctr-=2;
            }
        }
      else
        {
          // the previous pulse ended with a "1" that was not yet processed
          if( p==1 ) // 101
            {
              // read a "0", still odd
              d = (d*2) | 0;
              bitctr--;
            }
          else if( p==2 ) // 1001
            {
              // read "0", followed by "1", now even
              d = (d*4) | 01;
              bitctr-=2;
              even = true;
            }
          else if( p==3 ) // 10001
            {
              // read "0" followed by "1", still odd
              d = (d*4) | 01;
              bitctr-=2;
            }
        }

      if( bitctr<=0 )
        {
          if( bitctr<0 )
            {
              // already read one data bit ahead
              if( !verify )
                *ptr++ = d / 2;
              else if( *ptr++ != d/2 )
                return false;

              d &= 1;
              bitctr = 7;
            }
          else
            {
              if( !verify )
                *ptr++ = d;
              else if( *ptr++ != d )
                return false;

              d = 0;
              bitctr = 8;
            }

          n--;
        }
    }

  return true;
}


inline void writepulse(byte len)
{
  // OCR1A=0 means output compare match after one clock cycle so if we want
  // to match after "len" clock cycles then we need to set OCR1A to len-1
  OCR1AL = len - 1;

  // wait until output compare flag is set, OCP1 pin will go LOW at that point
  while( !(TIFR1 & bit(OCF1A)) );

  // set OCP1 pin back HIGH and reset output compare flag
  TCCR1C = bit(FOC1A);
  TIFR1  = bit(OCF1A);
}


inline void writesyncmark()
{
  // sync mark is 0xA1 with one missing clock bit: 
  // bits:    1 0 1 0 0 0 0 1
  // in MFM: 0100010010001001
  //                   ^
  // this bit should be 1 for proper MFM encoding but is 0 here

  // this function does NOT write the first bit (01). It gets written
  // before writesyncmark() is called.
  writepulse(US2TICKS(8)); // 0001
  writepulse(US2TICKS(6)); // 001
  writepulse(US2TICKS(8)); // 0001
  writepulse(US2TICKS(6)); // 001
}


void writebyte(byte d)
{
  byte bitctr = 8;
  while( bitctr > 0 )
    {
      if( d & 0x80 )
        {
          // write "1" => 01
          // after writing the pulse we set the timer for 4 microseconds
          // instead of 2, effectively writing "010" instead of "01".
          // This is necessary since a 2 microsecond wait is not enough time for 
          // us to loop around and process the next bit. Since there is never
          // a "11" sequence it is fine to always wrie a 0 after 1, we
          // just have to compensate for it when writing the next bit.

          if( !prevbit ) OCR1AL += US2TICKS(2);
          while( !(TIFR1 & bit(OCF1A)) );
          TCCR1C = bit(FOC1A);
          TIFR1  = bit(OCF1A);
          OCR1AL = US2TICKS(4) - 1;
          prevbit = 1;
        }
      else if( prevbit )
        {
          // write "0" (following a "1") => 00
          // after writing the "1" we already added a 0, so only add one more 0.
          OCR1AL += US2TICKS(2);
          prevbit = 0;
        }
      else
        {
          // write "0" (following a "0") => 10
          while( !(TIFR1 & bit(OCF1A)) );
          TCCR1C = bit(FOC1A);
          TIFR1  = bit(OCF1A);
          OCR1AL = US2TICKS(4) - 1; 
        }

      d = d * 2;
      bitctr--;
    }
}


inline void writeindexgap(byte i)
{
  // pre-index gap (same as data gap but with 0xC2 sync mark)
  // - "i" bytes of 0x4E
  // - 12 bytes of 0x00
  // - three index sync marks (0xC2 with one missing clock bit)
  // => 95 bytes
  //                    4E               00               C2               C2               C2
  // Data bits :  0 1 0 0 1 1 1 0  0 0 0 0 0 0 0 0  1 1 0 0*0 0 1 0  1 1 0 0*0 0 1 0  1 1 0 0*0 0 1 0 
  // In MFM    : 1001001001010100 1010101010101010 0101001000100100 0101001000100100 0101001000100100

  while( i-- > 0 )    writebyte(0x4E);
  for(i=0; i<12; i++) writebyte(0x00);
  writepulse(US2TICKS(6)); // 0 01 (start of first 0xC2)
  writepulse(US2TICKS(4)); // 01
  writepulse(US2TICKS(6)); // 001
  writepulse(US2TICKS(8)); // 0001
  writepulse(US2TICKS(6)); // 001
  writepulse(US2TICKS(8)); // 00 01 (start of second 0xC2)
  writepulse(US2TICKS(4)); // 01
  writepulse(US2TICKS(6)); // 001
  writepulse(US2TICKS(8)); // 0001
  writepulse(US2TICKS(6)); // 001
  writepulse(US2TICKS(8)); // 00 01 (start of third 0xC2)
  writepulse(US2TICKS(4)); // 01
  writepulse(US2TICKS(6)); // 001
  writepulse(US2TICKS(8)); // 0001
  writepulse(US2TICKS(6)); // 001
  OCR1AL  = US2TICKS(6)-1; // 00
  prevbit = 0;      // last bit of index sync mark (0xC2) is "0"
}


inline void writegap(byte i)
{
  // post-index, post-ID and post-data gap:
  // - "i" bytes of 0x4E
  // - 12 bytes of 0x00 
  // - three sync marks (0xA1 with one missing clock bit)
  // => 69 bytes
  //                    4E               00               A1               A1               A1
  // Data bits :  0 1 0 0 1 1 1 0  0 0 0 0 0 0 0 0  1 0 1 0 0*0 0 1  1 0 1 0 0*0 0 1  1 0 1 0 0*0 0 1
  // In MFM    : 1001001001010100 1010101010101010 0100010010001001 0100010010001001 0100010010001001
  
  while( i-- > 0 )    writebyte(0x4E);
  for(i=0; i<12; i++) writebyte(0x00);
  writepulse(US2TICKS(6)); // 001
  writesyncmark();  // 00010010001001
  writepulse(US2TICKS(4)); // 01
  writesyncmark();  // 00010010001001
  writepulse(US2TICKS(4)); // 01
  writesyncmark();  // 00010010001001
  OCR1AL  = US2TICKS(4)-1; // adding one "0"
  prevbit = 1;      // last bit of sync mark (0xA1) is "1"
}


static void write_data(byte *ptr, int n)
{
  // make sure OC1A is high before we enable WRITE_GATE
  DDRB   &= ~0x02;                     // disable OC1A pin
  TCCR1A  = bit(COM1A1) | bit(COM1A0); // set OC1A on compare match
  TCCR1C |= bit(FOC1A);                // force compare match
  TCCR1A  = 0;                         // disable OC1A control by timer
  DDRB   |= 0x02;                      // enable OC1A pin

  // wait through beginning of header gap (22 bytes of 0x4F)
  TCCR1B |= bit(WGM12);  // WGM12:10 = 010 => clear-timer-on-compare (CTC) mode 
  TCNT1 = 0;             // reset timer
  OCR1A = US2TICKS(702);        // 702us (22 bytes * 8 bits/byte * 4us/bit)
  TIFR1 = bit(OCF1A);    // clear OCF1A
  while( !(TIFR1 & bit(OCF1A)) ); // wait for OCF1A
  TIFR1 = bit(OCF1A);    // clear OCF1A
  OCR1A = US2TICKS(4);          // clear OCR1H byte (we only modify OCR1L below)
  
  // enable WRITE_GATE
  PORTB &= ~0x04;

  // enable OC1A output pin control by timer (WRITE_DATA), initially high
  TCCR1A  = bit(COM1A0); // COM1A1:0 =  01 => toggle OC1A on compare match

  // write post-ID gap except 0x4E sequence (skipped above)
  writegap(0);
  
  // write data bytes
  while( n-->0 ) writebyte(*ptr++);

  // re-write first byte of data gap
  writebyte(0x4E);

  // wait for final pulse
  while( !(TIFR1 & bit(OCF1A)) );
  TCCR1C = bit(FOC1A);

  // disable WRITE_GATE
  PORTB |= 0x04;

  // COM1A1:0 = 00 => disconnect OC1A (will go high)
  TCCR1A = 0;

  // WGM12:10 = 000 => Normal timer mode
  TCCR1B &= ~bit(WGM12);
}


static byte format_track(byte track, byte side)
{
  // format track:
  // writing 95 + 1 + 65 + (7 + 37 + 515 + 69) * 8 + (7 + 37 + 515) bytes
  // => 5744 bytes per track = 45952 bits
  // data rate 250 kbit/second, rotation rate 300 RPM (0.2s per rotation)
  // => 50000 bits unformatted capacity per track
  byte i, buffer[7*9];

  // pre-compute ID records
  byte *ptr = buffer;
  for(i=0; i<9; i++)
    {
      *ptr++ = 0xFE;      // ID mark
      *ptr++ = track;     // cylinder number
      *ptr++ = side;      // side number
      *ptr++ = i+1;       // sector number
      *ptr++ = 2;         // sector length
      uint16_t crc = calc_crc(ptr-5, 5);
      *ptr++ = crc / 256; // CRC
      *ptr++ = crc & 255; // CRC
    }
  ptr = buffer;

  noInterrupts();

  // make sure OC1A is high before we enable WRITE_GATE
  DDRB   &= ~0x02;                     // disable OC1A pin
  TCCR1A  = bit(COM1A1) | bit(COM1A0); // set OC1A on compare match
  TCCR1C |= bit(FOC1A);                // force compare match
  TCCR1A  = 0;                         // disable OC1A control by timer
  DDRB   |= 0x02;                      // enable OC1A pin

  // reset timer and overrun flags
  TCNT1 = 0;
  TIFR1 = bit(TOV1);

  // wait for start of index hole
  if( !wait_index_hole() ) { interrupts(); return S_NOINDEX; }

  TCCR1B |= bit(WGM12);  // WGM12:10 = 010 => clear-timer-on-compare (CTC) mode 
  TCNT1 = 0;             // reset timer
  OCR1A = US2TICKS(4);   // clear OCR1H byte (we only modify OCR1L below)
  TIFR1 = bit(OCF1A);    // clear OCF1A

  // enable WRITE_GATE
  PORTB &= ~0x04;

  // enable OC1A output pin control by timer (WRITE_DATA), initially high
  TCCR1A  = bit(COM1A0); // COM1A1:0 =  01 => toggle OC1A on compare match

  // pre-index gap (80+12+3 = 95 bytes)
  prevbit = 0;
  writeindexgap(80);

  // index record (one byte of 0xFC)
  writebyte(0xFC);

  // post-index gap (50+12+3 = 65 bytes)
  writegap(50);

  for(byte sector=0; sector<9; sector++)
    {
      // (pre-calculated) ID record (7 bytes)
      for(i=0; i<7; i++) writebyte(*ptr++);

      // post-ID gap (22+12+3 = 37 bytes)
      writegap(22);

      // Data record:
      // - Data mark: 0xFB
      // - 512 bytes of user data (0xF6)
      // - 2 bytes CRC (0x2B, 0xF6)
      // => 515 bytes
      writebyte(0xFB);
      i=0; do { writebyte(0xF6); } while( (++i) != 0 );
      i=0; do { writebyte(0xF6); } while( (++i) != 0 );
      writebyte(0x2B);
      writebyte(0xF6);

      // post-data gap (54+12+3 = 69 bytes)
      if( sector<8 ) writegap(54);
    }

  // end of track (fill with 0x4E until index mark)
  while( PIND & 0x80 ) writebyte(0x4E);
  
  // disable WRITE_GATE
  PORTB |= 0x04;

  // COM1A1:0 = 00 => disconnect OC1A (will go high)
  TCCR1A = 0;

  // WGM12:10 = 000 => Normal timer mode
  TCCR1B &= ~bit(WGM12);

  interrupts();

  return S_OK;
}


static byte wait_header(byte track, byte side, byte sector)
{
  byte attempts = 50;

  // check whether we get any data pulses from the drive at all
  if( !check_pulse() )
    {
#ifdef DEBUG
      Serial.println(F("Drive not ready!")); Serial.flush();
#endif
      return S_NOTREADY;
    }
  
  do
    {
      // wait for sync sequence
      if( wait_sync() )
        {
          // read 7 bytes of data
          read_data(header, 7, false);
          //if( header[0]==0xFE ) Serial.write('0'+header[3]);
          
          // make sure this is an ID record and check whether it contains the
          // expected track/side/sector information and the CRC is ok
          if( header[0]==0xFE && (track==0xFF || track==header[1]) && side==header[2] && sector==header[3] )
            {
              if( calc_crc(header, 5) == 256*header[5]+header[6] )
                return S_OK;
#ifdef DEBUG
              else
                { Serial.println(F("Header CRC error!")); Serial.flush(); }
#endif
            }
        }
      else
        return  false;
    }
  while( --attempts>0 );
  
#ifdef DEBUG
  if( attempts==0 )
    { Serial.println(F("Unable to find header!")); Serial.flush(); }
#endif

  return S_NOHEADER;
}


static void step_track()
{
  // produce LOW->HIGH pulse on STEP pin
  delay(10);
  digitalWrite(PIN_STEP, LOW);
  delay(1);
  digitalWrite(PIN_STEP, HIGH);
}


static bool step_to_track0()
{
  byte n = 82;

  // step outward until TRACK0 line goes low
  digitalWrite(PIN_STEPDIR, HIGH);
  while( --n > 0 && digitalRead(PIN_TRACK0) ) step_track();

  if( n==0 )
    {
      // we have stpped for more than 80 tracks and are still not 
      // seeing the TRACK0 signal
#ifdef DEBUG
      Serial.println(F("No Track0 signal!")); Serial.flush();
#endif
      return false;
    }

  return true;
}


static void step_tracks(int tracks)
{
  // if tracks<0 then step outward (outward towards track 0) otherwise step inward
  digitalWrite(PIN_STEPDIR, tracks<0 ? HIGH : LOW);
  tracks = abs(tracks);
  while( tracks-->0 ) step_track();
  delay(100); 
}


// --------------------------------------------------------------------------------------------------


ArduinoFDCClass::ArduinoFDCClass() 
{ 
  initialized=false;
  driveA=true;
  motorStateA=false; 
  motorStateB=false; 
}


void ArduinoFDCClass::begin() 
{
  // make sure all outputs pins are HIGH when we switch them to output mode
  digitalWrite(PIN_STEP,      HIGH);
  digitalWrite(PIN_STEPDIR,   HIGH);
  digitalWrite(PIN_MOTORA,    HIGH);
  digitalWrite(PIN_SELECTA,   HIGH);
#ifndef SINGLEDRIVE
  digitalWrite(PIN_MOTORB,    HIGH);
  digitalWrite(PIN_SELECTB,   HIGH);
#endif
  digitalWrite(PIN_SIDE,      HIGH);
  digitalWrite(PIN_WRITEDATA, HIGH);
  digitalWrite(PIN_WRITEGATE, HIGH);

  // set pins to input/output mode
  pinMode(PIN_STEP,      OUTPUT);
  pinMode(PIN_STEPDIR,   OUTPUT);
  pinMode(PIN_MOTORA,    OUTPUT);
  pinMode(PIN_MOTORB,    OUTPUT);
  pinMode(PIN_SELECTA,   OUTPUT);
  pinMode(PIN_SELECTB,   OUTPUT);
  pinMode(PIN_SIDE,      OUTPUT);
  pinMode(PIN_WRITEDATA, OUTPUT);
  pinMode(PIN_WRITEGATE, OUTPUT);
  pinMode(PIN_READDATA,  INPUT_PULLUP);
  pinMode(PIN_INDEX,     INPUT_PULLUP);
  pinMode(PIN_TRACK0,    INPUT_PULLUP);

  initialized = true;
  driveA      = true;
  motorStateA = false;
  motorStateB = false;
}


void ArduinoFDCClass::end()
{
  // release all output pins
  pinMode(PIN_STEP,      INPUT);
  pinMode(PIN_STEPDIR,   INPUT);
  pinMode(PIN_MOTORA,    INPUT);
  pinMode(PIN_SELECTA,   INPUT);
#ifndef SINGLEDRIVE
  pinMode(PIN_MOTORB,    INPUT);
  pinMode(PIN_SELECTB,   INPUT);
#endif
  pinMode(PIN_SIDE,      INPUT);
  pinMode(PIN_WRITEDATA, INPUT);
  pinMode(PIN_WRITEGATE, INPUT);

  initialized = false;
}


byte ArduinoFDCClass::readSector(byte track, byte side, byte sector, byte *buffer)
{
  byte res = S_OK;

  // check whether begin() has been called
  if( !initialized )
    {
#ifdef DEBUG
      Serial.println(F("Not initialized!")); Serial.flush();
#endif
      return S_NOTINIT;
    }

  // set up timer1
  TCCR1A = 0;  
  TCCR1B = bit(CS10); // falling edge input capture, prescaler 1, no output compare
  TCCR1C = 0;  

  // if motor is not running then turn it on now
  bool turnMotorOff = false;
  if( !motorRunning() )
    {
      turnMotorOff = true;
      motorOn();
    }

  // select disk drive
  digitalWrite(driveA ? PIN_SELECTA : PIN_SELECTB, LOW);

  // select side
  digitalWrite(PIN_SIDE, side>0 ? LOW : HIGH);

  // reading data is very time sensitive so we can't have interrupts
  noInterrupts();

  // wait for sector header
  res = wait_header(-1, side, sector);

  // found the sector header but it's not on the correct track => go to correct track and check again
  if( res==S_OK && header[1]!=track ) { interrupts(); step_tracks(track-header[1]); noInterrupts(); res = wait_header(track, side, sector); }

  // couldn't find a header => step to correct track by going to track 0 and then stepping out and check again
  if( res!=S_OK ) { interrupts(); step_to_track0(); step_tracks(track); noInterrupts(); res = wait_header(track, side, sector); }

  if( res==S_OK )
    {
      // wait for data sync mark
      if( wait_sync() )
        {
          // read data
          read_data(buffer, 515, false);
          if( buffer[0]!=0xFB )
            { 
#ifdef DEBUG
              Serial.println(F("Unexpected record identifier")); 
#endif
              res = S_INVALIDID;
            }
          else if( calc_crc(buffer, 513) != 256*buffer[513]+buffer[514] )
            { 
#ifdef DEBUG
              Serial.println(F("Data CRC error!")); 
#endif
              res = S_CRC; 
            }
        }
    }

  // interrupts are ok again
  interrupts();

  // de-select disk drive
  digitalWrite(driveA ? PIN_SELECTA : PIN_SELECTB, HIGH);

  // if we turned the motor on then turn it off again
  if( turnMotorOff ) motorOff();

  // stop timer
  TCCR1B = 0;
      
  return res;
}


byte ArduinoFDCClass::writeSector(byte track, byte side, byte sector, byte *buffer, bool verify)
{
  byte res = S_OK;

  // check whether begin() has been called
  if( !initialized )
    {
#ifdef DEBUG
      Serial.println(F("Not initialized!")); Serial.flush();
#endif
      return S_NOTINIT;
    }

  // set up timer1
  TCCR1A = 0;  
  TCCR1B = bit(CS10); // select falling edge input capture, prescaler 1, no output compare
  TCCR1C = 0;  

  // if motor is not running then turn it on now
  bool turnMotorOff = false;
  if( !motorRunning() )
    {
      turnMotorOff = true;
      motorOn();
    }

  // select disk drive
  digitalWrite(driveA ? PIN_SELECTA : PIN_SELECTB, LOW);

  // calculate CRC for the sector data
  buffer[0]   = 0xFB;
  uint16_t crc = calc_crc(buffer, 513);
  buffer[513] = crc/256;
  buffer[514] = crc&255;
  
  // select side
  digitalWrite(PIN_SIDE, side>0 ? LOW : HIGH);

  // reading/writing data is very time sensitive so we can't have interrupts
  noInterrupts();

  // wait for sector header
  res = wait_header(-1, side, sector);

  // found the sector header but it's not on the correct track => go to correct track and check again
  if( res==S_OK && header[1]!=track ) { interrupts(); step_tracks(track-header[1]); noInterrupts(); res = wait_header(track, side, sector); }

  // couldn't find a header => step to correct track by going to track 0 and then stepping out and check again
  if( res!=S_OK ) { interrupts(); step_to_track0(); step_tracks(track); noInterrupts(); res = wait_header(track, side, sector); }
  
  // if we found the header then write the data
  if( res==S_OK ) 
    {
      write_data(buffer, 515);

      // if we are supposed to verify the write then do so now
      if( verify )
        {
          // wait for sector header
          res = wait_header(track, side, sector);

          // wait for data sync mark
          if( res==S_OK && !wait_sync() ) res = S_NOSYNC;

          // compare the data
          if( res==S_OK && !read_data(buffer, 515, true) )
            {
#ifdef DEBUG
              Serial.println("Verify after write failed!"); Serial.flush();
#endif
              res = S_VERIFY;
            }
        }
    }

  // interrupts are ok again
  interrupts();

  // de-select disk drive
  digitalWrite(driveA ? PIN_SELECTA : PIN_SELECTB, HIGH);

  // if we turned the motor on then turn it off again
  if( turnMotorOff ) motorOff();

  // stop timer
  TCCR1B = 0;
      
  return res;
}


byte ArduinoFDCClass::formatDisk()
{
  byte res = S_OK;
  
  // check whether begin() has been called
  if( !initialized )
    {
#ifdef DEBUG
      Serial.println(F("Not initialized!")); Serial.flush();
#endif
      return S_NOTINIT;
    }

  // set up timer1
  TCCR1A = 0;  
  TCCR1B = bit(CS10); // select falling edge input capture, prescaler 1, no output compare
  TCCR1C = 0;  

  // if motor is not running then turn it on now
  bool turnMotorOff = false;
  if( !motorRunning() )
    {
      turnMotorOff = true;
      motorOn();
    }

  // select disk drive
  digitalWrite(driveA ? PIN_SELECTA : PIN_SELECTB, LOW);

  // go to track 0
  if( !step_to_track0() )
    return S_NOTRACK0;

  delay(100);
  for(byte track=0; track<80; track++)
    {
      digitalWrite(PIN_SIDE, HIGH);
      res = format_track(track, 0); if( res!=S_OK ) break;
      digitalWrite(PIN_SIDE, LOW);
      res = format_track(track, 1); if( res!=S_OK ) break;
      if( track<79 ) step_tracks(1);
    }

  // de-select disk drive
  digitalWrite(driveA ? PIN_SELECTA : PIN_SELECTB, HIGH);

  // if we turned the motor on then turn it off again
  if( turnMotorOff ) motorOff();

  // stop timer
  TCCR1B = 0;

  return res;
}


void ArduinoFDCClass::motorOn()
{
  if( !motorRunning() )
    {
      if( driveA )
        {
          digitalWrite(PIN_MOTORA, LOW);
          motorStateA = true;
        }
      else
        {
          digitalWrite(PIN_MOTORB, LOW);
          motorStateB = true;
        }
      
      // allow some time for the motor to spin up
      delay(1000);
    }
}


void ArduinoFDCClass::motorOff()
{
  if( driveA )
    {
      digitalWrite(PIN_MOTORA, HIGH);
      motorStateA = false;
    }
  else
    {
      digitalWrite(PIN_MOTORB, HIGH);
      motorStateB = false;
    }
}


bool ArduinoFDCClass::motorRunning() 
{ 
  return driveA ? motorStateA : motorStateB;
}



void ArduinoFDCClass::selectDrive(byte drive)
{
#ifdef SINGLEDRIVE
  if( drive!=0 ) Serial.println(F("Code compiled with SINGLEDRIVE defined - Can only control one drive"));
#else
  driveA = (drive==0);
#endif
}


byte ArduinoFDCClass::selectedDrive()
{
  return driveA ? 0 : 1;
}
