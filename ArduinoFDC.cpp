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

#if defined(__AVR_ATmega328P__)

// -------------------------------  Pin assignments for Arduino UNO/Nano/Pro Mini (Atmega328p)  ------------------------------

#define PIN_STEP       2  // can be changed to different pin
#define PIN_STEPDIR    3  // can be changed to different pin
#define PIN_MOTORA     4  // can be changed to different pin
#define PIN_SELECTA    5  // can be changed to different pin
#define PIN_SIDE       6  // can be changed to different pin
#define PIN_INDEX      7  // accesses via IDXPORT/IDXBIT #defines below
#define PIN_READDATA   8  // must be pin 8 (ICP for timer1)
#define PIN_WRITEDATA  9  // must be pin 9 (OCP for timer1)
#define PIN_WRITEGATE 10  // accessed via WGPORT/WGBIT #defines below
#define PIN_TRACK0    11  // can be changed to different pin
#define PIN_WRITEPROT 12  // can be changed to different pin or commented out
#define PIN_DENSITY   13  // can be changed to different pin or commented out
#define PIN_MOTORB    A0  // can be changed to different pin or commented out (together with PIN_SELECTB)
#define PIN_SELECTB   A1  // can be changed to different pin or commented out (together with PIN_MOTORB)


asm ("   .equ TIFR,    0x16\n"  // timer 1 flag register
     "   .equ TOV,     0\n"     // overflow flag
     "   .equ OCF,     1\n"     // output compare flag
     "   .equ ICF,     5\n"     // input capture flag
     "   .equ TCCRC,   0x82\n"  // timer 1 control register C
     "   .equ FOC,     0x80\n"  // force output compare flag
     "   .equ TCNTL,   0x84\n"  // timer 1 counter (low byte)
     "   .equ ICRL,    0x86\n"  // timer 1 input capture register (low byte)
     "   .equ OCRL,    0x88\n"  // timer 1 output compare register (low byte)
     "   .equ IDXPORT, 0x29\n"  // INDEX pin register (digital pin 7, register PD7, accessed via LDS instruction)
     "   .equ IDXBIT,  7\n"     // INDEX pin bit (digital pin 7, register PD7)
     );

#define TIFR    TIFR1   // timer 1 flag register
#define TOV     TOV1    // overflow flag
#define OCF     OCF1A   // output compare flag
#define ICF     ICF1    // input capture flag
#define TCCRA   TCCR1A  // timer 1 control register A
#define COMA1   COM1A1  // timer 1 output compare mode bit 1
#define COMA0   COM1A0  // timer 1 output compare mode bit 0
#define TCCRB   TCCR1B  // timer 1 control register B
#define CS1     CS11    // timer 1 clock select bit 1
#define CS0     CS10    // timer 1 clock select bit 0
#define WGM2    WGM12   // timer 1 waveform mode bit 2
#define TCCRC   TCCR1C  // timer 1 control register C
#define FOC     FOC1A   // force output compare flag
#define OCR     OCR1A   // timer 1 output compare register
#define TCNT    TCNT1   // timer 1 counter
#define IDXPORT PIND    // INDEX pin port     (digital pin  7, register PD7)
#define IDXBIT  7       // INDEX pin bit      (digital pin  7, register PD7)
#define WGPORT  DDRB    // WRITEGATE pin port (digital pin 10, register PB2)
#define WGBIT   2       // WRITEGATE pin bit  (digital pin 10, register PB2)
#define OCDDR   DDRB    // DDR controlling WRITEDATA pin
#define OCBIT   1       // bit for WRITEDATA pin


#elif defined(__AVR_ATmega32U4__)

// -----------------------  Pin assignments for Arduino Leonardo/Micro (Atmega32U4)  --------------------------

#define PIN_STEP       2  // can be changed to different pin
#define PIN_STEPDIR    3  // can be changed to different pin
#define PIN_READDATA   4  // must be pin 4 (ICP for timer1)
#define PIN_MOTORA     5  // can be changed to different pin
#define PIN_SELECTA    6  // can be changed to different pin
#define PIN_SIDE       7  // can be changed to different pin
#define PIN_INDEX      8  // accesses via IDXPORT/IDXBIT #defines below
#define PIN_WRITEDATA  9  // must be pin 9 (OCP for timer1)
#define PIN_WRITEGATE 10  // accessed via WGPORT/WGBIT #defines below
#if defined(ARDUINO_AVR_LEONARDO)
#define PIN_TRACK0    11  // can be changed to different pin
#define PIN_WRITEPROT 12  // can be changed to different pin or commented out
#define PIN_DENSITY   13  // can be changed to different pin or commented out
#else
#define PIN_TRACK0    14  // can be changed to different pin
#define PIN_WRITEPROT 15  // can be changed to different pin or commented out
#define PIN_DENSITY   16  // can be changed to different pin or commented out
#endif
#define PIN_MOTORB    A0  // can be changed to different pin or commented out (together with PIN_SELECTB)
#define PIN_SELECTB   A1  // can be changed to different pin or commented out (together with PIN_MOTORB)


asm ("   .equ TIFR,    0x16\n"  // timer 1 flag register
     "   .equ TOV,     0\n"     // overflow flag
     "   .equ OCF,     1\n"     // output compare flag
     "   .equ ICF,     5\n"     // input capture flag
     "   .equ TCCRC,   0x82\n"  // timer 1 control register C
     "   .equ FOC,     0x80\n"  // force output compare flag
     "   .equ TCNTL,   0x84\n"  // timer 1 counter (low byte)
     "   .equ ICRL,    0x86\n"  // timer 1 input capture register (low byte)
     "   .equ OCRL,    0x88\n"  // timer 1 output compare register (low byte)
     "   .equ IDXPORT, 0x23\n"  // INDEX pin register (digital pin 8, register PB4, accessed via LDS instruction)
     "   .equ IDXBIT,  4\n"     // INDEX pin bit (digital pin 8, register PB4)
     );

#define TIFR    TIFR1   // timer 1 flag register
#define TOV     TOV1    // overflow flag
#define OCF     OCF1A   // output compare flag
#define ICF     ICF1    // input capture flag
#define TCCRA   TCCR1A  // timer 1 control register A
#define COMA1   COM1A1  // timer 1 output compare mode bit 1
#define COMA0   COM1A0  // timer 1 output compare mode bit 0
#define TCCRB   TCCR1B  // timer 1 control register B
#define CS1     CS11    // timer 1 clock select bit 1
#define CS0     CS10    // timer 1 clock select bit 0
#define WGM2    WGM12   // timer 1 waveform mode bit 2
#define TCCRC   TCCR1C  // timer 1 control register C
#define FOC     FOC1A   // force output compare flag
#define OCR     OCR1A   // timer 1 output compare register
#define TCNT    TCNT1   // timer 1 counter
#define IDXPORT PINB    // INDEX pin port     (digital pin  8, register PB4)
#define IDXBIT  4       // INDEX pin bit      (digital pin  8, register PB4)
#define WGPORT  DDRB    // WRITEGATE pin port (digital pin 10, register PB6)
#define WGBIT   6       // WRITEGATE pin bit  (digital pin 10, register PB6)
#define OCDDR   DDRB    // WRITEDATA pin port (digital pin  9, register PB5)
#define OCBIT   5       // WRITEDATA pin bit  (digital pin  9, register PB5)


#elif defined(__AVR_ATmega2560__)

// ------------------------------  Pin assignments for Arduino Mega (Atmega2560)  -----------------------------

#define PIN_STEP      53  // can be changed to different pin
#define PIN_STEPDIR   52  // can be changed to different pin
#define PIN_MOTORA    51  // can be changed to different pin
#define PIN_SELECTA   50  // can be changed to different pin
#define PIN_SIDE      49  // can be changed to different pin
#define PIN_INDEX     47  // accessed via IDXPORT/IDXBIT #defines below
#define PIN_READDATA  48  // must be pin 48 (ICP for timer5)
#define PIN_WRITEDATA 46  // must be pin 46 (OCP for timer5)
#define PIN_WRITEGATE 45  // accessed via WGPORT/WGBIT #defines below
#define PIN_TRACK0    44  // can be changed to different pin
#define PIN_WRITEPROT 43  // can be changed to different pin or commented out
#define PIN_DENSITY   42  // can be changed to different pin or commented out
#define PIN_MOTORB    41  // can be changed to different pin or commented out (together with PIN_SELECTB)
#define PIN_SELECTB   40  // can be changed to different pin or commented out (together with PIN_MOTORB)


asm ("   .equ TIFR,    0x1A\n"  // timer 5 flag register
     "   .equ TOV,     0\n"     // overflow flag
     "   .equ OCF,     1\n"     // output compare flag
     "   .equ ICF,     5\n"     // input capture flag
     "   .equ TCCRC,   0x122\n" // timer 5 control register C
     "   .equ FOC,     0x80\n"  // force output compare flag
     "   .equ TCNTL,   0x124\n" // timer 5 counter (low byte)
     "   .equ ICRL,    0x126\n" // timer 5 input capture register (low byte)
     "   .equ OCRL,    0x128\n" // timer 5 output compare register (low byte)
     "   .equ IDXPORT, 0x109\n" // INDEX pin register (digital pin 47, register PL2)
     "   .equ IDXBIT,  2\n"     // INDEX pin bit (digital pin 47, register PL2)
     );

#define TIFR    TIFR5   // timer 5 flag register
#define TOV     TOV5    // overflow flag
#define OCF     OCF5A   // output compare flag
#define ICF     ICF5    // input capture flag
#define TCCRA   TCCR5A  // timer 5 control register A
#define COMA1   COM5A1  // timer 5 output compare mode bit 1
#define COMA0   COM5A0  // timer 5 output compare mode bit 0
#define TCCRB   TCCR5B  // timer 5 control register B
#define CS1     CS51    // timer 5 clock select bit 1
#define CS0     CS50    // timer 5 clock select bit 0
#define WGM2    WGM52   // timer 5 waveform mode bit 2
#define TCCRC   TCCR5C  // timer 5 control register C
#define FOC     FOC5A   // force output compare flag
#define OCR     OCR5A   // timer 5 output compare register
#define TCNT    TCNT5   // timer 5 counter
#define IDXPORT PINL    // INDEX pin port     (digital pin 47, register PL2)
#define IDXBIT  2       // INDEX pin bit      (digital pin 47, register PL2)
#define WGPORT  DDRL    // WRITEGATE pin port (digital pin 45, register PL4)
#define WGBIT   4       // WRITEGATE pin bit  (digital pin 45, register PL4)
#define OCDDR   DDRL    // DDR controlling WRITEDATA pin
#define OCBIT   3       // bit for WRITEDATA pin


#else

#error "ArduinoFDC library requires either an ATMega328P, Atmega32U4 or ATMega2560 processor (Arduino UNO, Leonardo or MEGA)"

#endif

#if F_CPU != 16000000
#error "ArduinoFDC library requires 16MHz clock speed"
#endif


struct DriveGeometryStruct
{
  byte numTracks;
  byte numSectors;
  byte dataGap;
  byte trackSpacing;
};


static struct DriveGeometryStruct geometry[5] =
  {
    {40,  9,  80, 1},  // 5.25" DD (360 KB)
    {40,  9,  80, 2},  // 5.25" DD disk in HD drive (360 KB)
    {80, 15,  85, 1},  // 5.25" HD (1.2 MB)
    {80,  9,  80, 1},  // 3.5"  DD (720 KB)
    {80, 18, 100, 1}   // 3.5"  HD (1.44 MB)
  };


// un-commenting this will write more detailed error information to Serial
//#define DEBUG

ArduinoFDCClass ArduinoFDC;
static byte header[7];


// digitalWrite function for simulating open-collector outputs.
// Each output pin must be initialized by digitalWrite(pin, LOW) and pinMode(PIN, INPUT)
// after that, switching the pinMode to INPUT will set the pin to high-Z state
// and switching to OUTPUT will pull the pin low.
// The floppy disk interface specification expects outputs to be open-collector
void digitalWriteOC(byte pin, byte state) 
{ if( state==LOW ) pinMode(pin, OUTPUT); else pinMode(pin, INPUT); }



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


static bool is_write_protected()
{
#if defined(PIN_WRITEPROT)
  return !digitalRead(PIN_WRITEPROT);
#else
  return false;
#endif
}

static bool check_pulse()
{
  // reset timer and capture/overrun flags
  TCNT = 0;
  TIFR = bit(ICF) | bit(TOV);

  // wait for either input capture or timer overrun
  while( !(TIFR & (bit(ICF) | bit(TOV))) );

  // if there was an input capture then we are ok
  bool res = (TIFR & bit(ICF))!=0;

  // reset input capture and timer overun flags
  TIFR = bit(ICF) | bit(TOV);

  return res;
}


static bool wait_index_hole()
{
  // reset timer and overrun flags
  TCNT = 0;
  TIFR = bit(TOV);
  byte ctr = 0;

  // wait for END of index hole (in case we're on the hole right now)
  while( !(IDXPORT & bit(IDXBIT)) )
    {
      if( TIFR & bit(TOV) )
        {
          // timer overflow happens every 4.096ms (65536 cycles at 16MHz)
          // meaning we haven't found a sync in that amount of time
           if( ++ctr == 0 )
            {
              // we have tried for 256 * 4.096ms = 1.048 seconds to find a index hole
              // one rotation is 166 or 200ms so we have tried for 5 or more rotations => give up
#ifdef DEBUG
              Serial.println(F("No index hole signal!")); Serial.flush();
#endif
              return false;
            }
          
          // clear overflow flag
          TIFR = bit(TOV);
        }
    }

  // wait for START of index hole (same as above)
  ctr = 0;
  while( (IDXPORT & bit(IDXBIT)) )
    {
      if( TIFR & bit(TOV) )
        {
          if( ++ctr == 0 ) 
            {
#ifdef DEBUG
              Serial.println(F("No index hole signal!")); Serial.flush();
#endif
              return false;
            }

          TIFR = bit(TOV);
        }
    }

  return true;
}



static byte read_data(byte bitlen, byte *buffer, unsigned int n, byte verify)
{
  byte status;

  // expect at least 10 bytes of 0x00 followed by three sync marks (0xA1 with one missing clock bit)
  // Data bits :     0 0 ...0  1 0 1 0 0*0 0 1  1 0 1 0 0*0 0 1  1 0 1 0 0*0 0 1
  // In MFM    : (0)1010...10 0100010010001001 0100010010001001 0100010010001001

  asm volatile 
    (
     // define READPULSE macro (wait for pulse)
     // macro arguments: 
     //         length: none => just wait for pulse, don't check         ( 9 cycles)
     //                 1    => wait for pulse and jump if NOT short  (12/14 cycles)
     //                 2    => wait for pulse and jump if NOT medium (14/16 cycles)
     //                 3    => wait for pulse and jump if NOT long   (12/14 cycles)
     //         dst:    label to jump to if DIFFERENT pulse found
     // 
     // on entry: r16 contains minimum length of medium pulse
     //           r17 contains minimum length of long   pulse
     //           r18 contains time of previous pulse
     // on exit:  r18 is updated to the time of this pulse
     //           r22 contains the pulse length in timer ticks (=processor cycles)     
     // CLOBBERS: r19
     ".macro READPULSE length=0,dst=undefined\n"
     "        sbis    TIFR, ICF\n"     // (1/2) skip next instruction if timer input capture seen
     "        rjmp    .-4\n"           // (2)   wait more 
     "        lds     r19, ICRL\n"     // (2)   get time of input capture (ICR1L, lower 8 bits only)
     "        sbi     TIFR, ICF\n "    // (2)   clear input capture flag
     "        mov     r22, r19\n"      // (1)   calculate time since previous capture...
     "        sub     r22, r18\n"      // (1)   ...into r22
     "        mov     r18, r19\n"      // (1)   set r18 to time of current capture
     "  .if \\length == 1\n"           //       waiting for short pule?
     "        cp      r22, r16\n"      // (1)   compare r22 to min medium pulse
     "        brlo   .+2\n"            // (1/2) skip jump if less
     "        rjmp   \\dst\n"          // (3)   not the expected pulse => jump to dst
     "  .else \n"
     "    .if \\length == 2\n"         // waiting for medium pulse?
     "        cp      r16, r22\n"      // (1)   min medium pulse < r22? => carry set if so
     "        brcc    .+2\n"           // (1/2) skip next instruction if carry is clear
     "        cp      r22, r17\n"      // (1)   r22 < min long pulse? => carry set if so
     "        brcs   .+2\n"            // (1/2) skip jump if greater
     "        rjmp   \\dst\n"          // (3)   not the expected pulse => jump to dst
     "    .else\n"
     "      .if \\length == 3\n" 
     "        cp      r22, r17\n"      // (1)   min long pulse < r22?
     "        brsh   .+2\n"            // (1/2) skip jump if greater
     "        rjmp   \\dst\n"          // (3)   not the expected pulse => jump to dst
     "      .endif\n"
     "    .endif\n"
     "  .endif\n"
     ".endm\n"

     // define STOREBIT macro for storing or verifying data bit 
     // storing  data  : 5/14 cycles for "1", 4/13 cycles for "0"
     // verifying data : 5/15 cycles for "1", 4/14 cycles for "0"
     ".macro STOREBIT data:req,done:req\n"
     "        lsl     r20\n"           // (1)   shift received data
     ".if \\data != 0\n"
     "        ori     r20, 1\n"        // (1)   store "1" bit
     ".endif\n"
     "        dec     r21\n"           // (1)   decrement bit counter
     "        brne    .+22\n"          // (1/2) skip if bit counter >0
     "        cpi     %1, 0\n"         // (1)   are we verifying?
     "        brne    .+4\n"           // (1/2) if yes, jump to verify
     "        st      Z+, r20\n"       // (2)   store received data byte
     "        rjmp    .+6\n"           // (2)   skip verify
     "        ld      r21, Z+\n"       // (2)   get next expected byte
     "        cpse    r20, r21\n"      // (1/2) compare to received byte
     "        rjmp    rddiff\n"        // (2)   jump if different

     "        ldi     r21, 8\n"        // (1)   re-initialize bit counter
     "        subi    r26, 1\n"        // (1)   subtract one from byte counter
     "        sbci    r27, 0\n"        // (1) 
     "        brmi    \\done\n"        // (1/2) done if byte counter<0
     ".endm\n"

     // prepare for reading SYNC
     "        mov         r16, %2\n"   // (1)   r16 = 2.5 * (MFM bit len) = minimum length of medium pulse
     "        lsr         r16\n"       // (1)
     "        add         r16, %2\n"   // (1)
     "        add         r16, %2\n"   // (1)
     "        mov         r17, r16\n"  // (1)   r17 = 3.5 * (MFM bit len) = minimum length of long pulse
     "        add         r17, %2\n"   // (1)
     "        ldi         %0, 0\n"     // (1)   default return status is S_OK
     "        mov         r15, %0\n"   // (1)   initialize timer overflow counter
     "        sbi         TIFR, TOV\n" // (2)   reset timer overflow flag

     // wait for at least 80x "10" (short) pulse followed by "100" (medium) pulse
     "ws0:    ldi         r20, 0\n"    // (1)   initialize "short pulse" counter
     "ws1:    sbis        TIFR, TOV\n" // (1/2) skip next instruction if timer overflow occurred
     "        rjmp        ws2\n"       // (2)   continue (no overflow)
     "        sbi         TIFR, TOV\n" // (2)   reset timer overflow flag
     "        dec         r15\n"       // (1)   overflow happens every 4.096ms, decrement overflow counter
     "        brne        ws2\n"       // (1/2) continue if fewer than 256 overflows
     "        ldi         %0, 3\n"     // (1)   no sync found in 1.048s => return status is is S_NOSYNC
     "        rjmp        rdend\n"     // (2)   done
     "ws2:    inc         r20\n"       // (1)   increment "short pulse" counter
     "        READPULSE\n"             // (9)   wait for pulse
     "        cp          r22, r16\n"  // (1)   pulse length < min medium pulse?
     "        brlo        ws1\n"       // (1/2) repeat if so
     "        cp          r22, r17\n"  // (1)   pulse length < min long pulse?
     "        brsh        ws0\n"       // (1/2) restart if this was a long pulse (expecting medium)
     "        cpi         r20, 80\n"   // (1)   did we see at least 80 short pulses?
     "        brlo        ws0\n"       // (1/2) restart if not

     // expect remaining part of first sync mark (..00010010001001)
     "        READPULSE   3,ws0\n"     // (12)  expect long pulse (0001)
     "        READPULSE   2,ws0\n"     // (14)  expect medium pulse (001)
     "        READPULSE   3,ws0\n"     // (12)  expect long pulse (0001)
     "        READPULSE   2,ws0\n"     // (14)  expect medium pulse (001)

     // expect second sync mark (0100010010001001)
     "        READPULSE   1,ws0\n"     // (12)  expect short pulse (01)
     "        READPULSE   3,ws0\n"     // (12)  expect long pulse (0001)
     "        READPULSE   2,ws0\n"     // (14)  expect medium pulse (001)
     "        READPULSE   3,ws0\n"     // (12)  expect long pulse (0001)
     "        READPULSE   2,ws0\n"     // (14)  expect medium pulse (001)

     // expect third sync mark (0100010010001001)
     "        READPULSE   1,ws0\n"     // (12)  expect short pulse (01)
     "        READPULSE   3,ws0\n"     // (12)  expect long pulse (0001)
     "        READPULSE   2,ws0\n"     // (14)  expect medium pulse (001)
     "        READPULSE   3,ws0\n"     // (12)  expect long pulse (0001)
     "        READPULSE   2,ws0\n"     // (14)  expect medium pulse (001)

     // found SYNC => prepare for reading data
     "        tst     r27\n"           // (1)   test byte count
     "        brpl    .+2\n"           // (1/2) skip following instruction if not negative
     "        rjmp    rdend\n"         // (2)   nothing to read (only waiting for sync) => end
     "        ldi     r21, 8\n"        // (1)   initialize bit counter (8 bits per byte)

     // odd section (previous data bit was "1", no unprocessed MFM bit)
     // shortest path: 19 cycles, longest path: 34 cycles
     // (longest path only happens when finishing a byte, about every 5-6 pulses)
     "rdo:    READPULSE\n"             // (9)   wait for pulse
     "        cp      r22, r16\n"      // (1)   pulse length >= min medium pulse?
     "        brlo    rdos\n"          // (1/2) jump if not
     "        cp      r22, r17\n"      // (1)   pulse length >= min long pulse?
     "        brlo    rdom\n"          // (1/2) jump if not

     // long pulse (0001) => read "01", still odd
     "        STOREBIT 0,rddone\n"      // (4/13) store "0" bit
     "        STOREBIT 1,rddone\n"      // (5/14) store "1" bit
     "        rjmp    rdo\n"            // (2)    back to start (still odd)

     // jump target for relative conditional jumps in STOREBIT macro
     "rddone:  rjmp    rdend\n"
     
     // medium pulse (001) => read "0", now even
     "rdom:   STOREBIT 0,rddone\n"      // (4/13) store "0" bit
     "        rjmp    rde\n"            // (2)   back to start (now even)

     // short pulse (01) => read "1", still odd
     "rdos:   STOREBIT 1,rddone\n"      // (5/14) store "1" bit
     "        rjmp    rdo\n"            // (2)    back to start (still odd)

     // even section (previous data bit was "0", previous MFM "1" bit not yet processed)
     // shortest path: 19 cycles, longest path: 31 cycles
     "rde:    READPULSE\n"             // (9)   wait for pulse
     "        cp      r22, r16\n"      // (1)   pulse length >= min medium pulse?
     "        brlo    rdes\n"          // (1/2) jump if not

     // either medium pulse (1001) or long pulse (10001) => read "01"
     // (a long pulse should never occur in this section but it may just be a 
     //  slightly too long medium pulse so count it as medium)
     "        STOREBIT 0,rdend\n"      // (4/13) store "0" bit
     "        STOREBIT 1,rdend\n"      // (5/14) store "1" bit
     "        rjmp    rdo\n"           // (2)    back to start (now odd)

     // short pulse (101) => read "0"
     "rdes:   STOREBIT 0,rdend\n"      // (5/14) store "0" bit
     "        rjmp    rde\n"           // (2)    back to start (still even)

     "rddiff: ldi     %0, 8\n"         // return status is S_VERIFY (verify error)
     "rdend:\n"
     
     : "=r"(status)                         // outputs
     : "r"(verify), "r"(bitlen), "x"(n-1), "z"(buffer)   // inputs  (x=r26/r27, z=r30/r31)
     : "r15", "r16", "r17", "r18", "r19", "r20", "r21", "r22");  // clobbers

  return status;
}


asm (// define WRITEPULSE macro (used in write_data and format_track)
     ".macro WRITEPULSE length=0\n"
     "  .if \\length==1\n"
     "          sts   OCRL, r16\n"       // (2)   set OCRxA to short pulse length
     "  .endif\n"
     "  .if \\length==2\n"
     "          sts   OCRL, r17\n"       // (2)   set OCRxA to medium pulse length
     "  .endif\n"
     "  .if \\length==3\n"
     "          sts   OCRL, r18\n"       // (2)   set OCRxA to long pulse length
     "  .endif\n"
     "          sbis  TIFR, OCF\n"       // (1/2) skip next instruction if OCFx is set
     "          rjmp  .-4\n"             // (2)   wait more
     "          ldi   r19,  FOC\n"       // (1)
     "          sts   TCCRC, r19\n"      // (2)   set OCP back HIGH (was set LOW when timer expired)
     "          sbi   TIFR, OCF\n"       // (2)   reset OCFx (output compare flag)
     ".endm\n");


static void write_data(byte bitlen, byte *buffer, unsigned int n)
{
  // make sure OC1A is high before we enable WRITE_GATE
  OCDDR  &= ~bit(OCBIT);             // disable OC1A pin
  TCCRA  = bit(COMA1) | bit(COMA0);  // set OC1A on compare match
  TCCRC |= bit(FOC);                 // force compare match
  TCCRA  = 0;                        // disable OC1A control by timer
  OCDDR |= bit(OCBIT);               // enable OC1A pin

  // wait through beginning of header gap (22 bytes of 0x4F)
  TCCRB |= bit(WGM2);             // WGMx2:10 = 010 => clear-timer-on-compare (CTC) mode 
  TCNT = 0;                       // reset timer
  OCR = 352 * bitlen - 256;       // 352 MFM bit lengths (22 bytes * 8 bits/byte * 2 MFM bits/data bit) * cycles/MFM bit - 16us (overhead)
  TIFR = bit(OCF);                // clear OCFx
  while( !(TIFR & bit(OCF)) );    // wait for OCFx
  OCR = 255;                      // clear OCRH byte (we only modify OCRL below)
  TIFR = bit(OCF);                // clear OCFx

  // set WRITEGATE to OUTPUT (pulls it low)
  WGPORT |= bit(WGBIT);

  // enable OC1A output pin control by timer (WRITE_DATA), initially high
  TCCRA  = bit(COMA0); // COMxA1:0 =  01 => toggle OC1A on compare match

  asm volatile
    (// define GETNEXTBIT macro for getting next data bit into carry (4/9 cycles)
     // on entry: R20         contains the current byte 
     //           R21         contains the bit counter
     //           X (R26/R27) contains the byte counter
     //           Z (R30/R31) contains pointer to data buffer
     ".macro GETNEXTBIT\n"
     "          dec     r21\n"           // (1)   decrement bit counter
     "          brpl    .+10\n"          // (1/2) skip the following if bit counter >=  0
     "          subi    r26, 1\n"        // (1)   subtract one from byte counter
     "          sbci    r27, 0\n"        // (1) 
     "          brmi    wdone\n"         // (1/2) done if byte counter <0
     "          ld	r20, Z+\n"       // (2)   get next byte
     "          ldi     r21, 7\n"        // (1)   reset bit counter (7 more bits after this first one)
     "          rol     r20\n"           // (1)   get next data bit into carry
     ".endm\n"

     // initialize pulse-length registers (r16, r17, r18)
     "          mov   r16, %0\n"         //       r16 = (2*bitlen)-1 = time for short ("01") pulse         
     "          add   r16, %0\n"
     "          dec   r16\n"
     "          mov   r17, r16\n"        //       r17 = (3*bitlen)-1 = time for medium ("001") pulse
     "          add   r17, %0\n"
     "          mov   r18, r17\n"        //       r18 = (4*bitlen)-1 = time for long ("0001") pulse
     "          add   r18, %0\n"

     // write 12 bytes (96 bits) of "0" (i.e. 96 "10" sequences, i.e. short pulses)
     "          ldi     r20, 0\n"        
     "          sts     TCNTL, r20\n"    //       reset timer
     "          ldi     r20, 96\n"       //       initialize counter
     "wri:      WRITEPULSE 1\n"          //       write short pulse
     "          dec     r20\n"           //       decremet counter
     "          brne    wri\n"           //       repeat until 0

     // first sync "A1": 00100010010001001
     "          WRITEPULSE 2\n"          //       write medium pulse
     "          WRITEPULSE 3\n"          //       write long pulse
     "          WRITEPULSE 2\n"          //       write medium pulse
     "          WRITEPULSE 3\n"          //       write long pulse (this is the missing clock bit)
     "          WRITEPULSE 2\n"          //       write medium pulse
     
     // second sync "A1": 0100010010001001
     "          WRITEPULSE 1\n"          //       write short pulse
     "          WRITEPULSE 3\n"          //       write long pulse
     "          WRITEPULSE 2\n"          //       write medium pulse
     "          WRITEPULSE 3\n"          //       write long pulse (this is the missing clock bit)
     "          WRITEPULSE 2\n"          //       write medium pulse

     // third sync "A1": 0100010010001001
     "          WRITEPULSE 1\n"          //       write short pulse
     "          WRITEPULSE 3\n"          //       write long pulse
     "          WRITEPULSE 2\n"          //       write medium pulse
     "          WRITEPULSE 3\n"          //       write long pulse (this is the missing clock bit)
     "          WRITEPULSE 2\n"          //       write medium pulse

     // start writing data
     "          sts     OCRL, r16\n"     // (2)   set up timer for "01" sequence
     "          ldi     r21, 0\n"        // (1)   initialize bit counter to fetch next byte

     // just wrote a "1" bit => must be followed by either "01" (for "1" bit) or "00" (for "0" bit)
     // (have time to fetch next bit during the leading "0")
     "wro:      GETNEXTBIT\n"            // (4/9) fetch next data bit into carry
     "          brcs    wro1\n"          // (1/2) jump if "1"
     // next bit is "0" => write "00"
     "          lds     r19,  OCRL\n"    // (2)   get current OCRxAL value
     "          add     r19,  %0\n"      // (2)   add one-bit time
     "          sts     OCRL, r19\n"     // (2)   set new OCRxAL value
     "          rjmp    wre\n"           // (2)   now even
     // next bit is "1" => write "01"
     "wro1:     WRITEPULSE\n"            // (7)   wait and write pulse
     "          sts     OCRL, r16\n"     // (2)   set up timer for another "01" sequence
     "          rjmp    wro\n"           // (2)   still odd

     // just wrote a "0" bit, (i.e. either "10" or "00") where time for the trailing "0" was already added
     // to the pulse length (have time to fetch next bit during the already-added "0")
     "wre:      GETNEXTBIT\n"            // (4/9) fetch next data bit into carry
     "          brcs    wre1\n"          // (1/2) jump if "1"
     // next bit is "0" => write "10"
     "          WRITEPULSE\n"            // (7)   wait and write pulse
     "          sts     OCRL, r16\n"     // (2)   set up timer for another "10" sequence
     "          rjmp    wre\n"           // (2)   still even
     // next bit is "1" => write "01"
     "wre1:     lds     r19,  OCRL\n"    // (2)   get current OCRxAL value
     "          add     r19,  %0\n"      // (2)   add one-bit time
     "          sts     OCRL, r19\n"     // (2)   set new OCRxAL value
     "          WRITEPULSE\n"            // (7)   wait and write pulse
     "          sts     OCRL, r16\n"     // (2)   set up timer for "01" sequence
     "          rjmp    wro\n"           // (2)   now odd

     // done writing
     "wdone:    WRITEPULSE\n"            // (9)   wait for and write final pulse

     :                                     // no outputs
     : "r"(bitlen), "x"(n), "z"(buffer) // inputs  (x=r26/r27, z=r30/r31)
     : "r16", "r17", "r18", "r19", "r20", "r21"); // clobbers

  // set WRITEGATE back to input (releases it HIGH)
  WGPORT &= ~bit(WGBIT);

  // COMxA1:0 = 00 => disconnect OC1A (will go high)
  TCCRA = 0;

  // WGMx2:10 = 000 => Normal timer mode
  TCCRB &= ~bit(WGM2);
}



static byte format_track(byte *buffer, byte driveType, byte bitlen, byte track, byte side)
{
  // 3.5" DD disk:
  //   writing 95 + 1 + 65 + (7 + 37 + 515 + 69) * 8 + (7 + 37 + 515) bytes
  //   => 5744 bytes per track = 45952 bits
  //   data rate 250 kbit/second, rotation rate 300 RPM (0.2s per rotation)
  //   => 50000 bits unformatted capacity per track

  // 3.5" HD disk:
  //   writing 95 + 1 + 65 + (7 + 37 + 515 + 69) * 17 + (7 + 37 + 515) bytes
  //   => 5744 bytes per track = 45952 bits
  //   data rate 500 kbit/second, rotation rate 300 RPM (0.2s per rotation)
  //   => 100000 bits unformatted capacity per track
  byte i;

  byte numsec     = geometry[driveType].numSectors;
  byte datagaplen = geometry[driveType].dataGap;

  // pre-compute ID records
  byte *ptr = buffer;
  for(i=0; i<numsec; i++)
    {
      *ptr++ = 0xFE;      // ID mark
      *ptr++ = track;     // cylinder number
      *ptr++ = side;      // side number
      *ptr++ = i+1;       // sector number
      *ptr++ = 2;         // sector length
      uint16_t crc = calc_crc(ptr-5, 5);
      *ptr++ = crc / 256; // CRC
      *ptr++ = crc & 255; // CRC
      *ptr++ = 0x4E;      // first byte of post-data gap
    }

  noInterrupts();

  // make sure OC1A is high before we enable WRITE_GATE
  OCDDR  &= ~bit(OCBIT);             // disable OC1A pin
  TCCRA  = bit(COMA1) | bit(COMA0);  // set OC1A on compare match
  TCCRC |= bit(FOC);                 // force compare match
  TCCRA  = 0;                        // disable OC1A control by timer
  OCDDR |= bit(OCBIT);               // enable OC1A pin

  // reset timer and overrun flags
  TCNT = 0;
  TIFR = bit(TOV);

  // wait for start of index hole
  if( !wait_index_hole() ) { interrupts(); return S_NOTREADY; }

  TCCRB |= bit(WGM2);   // WGMx2:10 = 010 => clear-timer-on-compare (CTC) mode 
  TCNT = 0;             // reset timer
  OCR = 32;             // clear OCRxH byte (we only modify OCRxL below)
  TIFR = bit(OCF);      // clear OCFx

  // set WRITEGATE to OUTPUT (pulls it low)
  WGPORT |= bit(WGBIT);

  // enable OC1A output pin control by timer (WRITE_DATA), initially high
  TCCRA  = bit(COMA0); // COMxA1:0 =  01 => toggle OC1A on compare match

  asm volatile
    (".macro    WRTPS\n"
     "          sts   OCRL, r16\n"
     "          call  waitp\n"    
     ".endm\n"
     ".macro    WRTPM\n"
     "          sts   OCRL, r17\n"
     "          call  waitp\n"
     ".endm\n"
     ".macro    WRTPL\n"
     "          sts   OCRL, r18\n"
     "          call  waitp\n"
     ".endm\n"

     // initialize
     "          mov    r16, %0\n"         //       r16 = (2*bitlen)-1 = time for short ("01") pulse         
     "          add    r16, %0\n"
     "          dec    r16\n"
     "          mov    r17, r16\n"        //       r17 = (3*bitlen)-1 = time for medium ("001") pulse
     "          add    r17, %0\n"
     "          mov    r18, r17\n"        //       r18 = (4*bitlen)-1 = time for long ("0001") pulse
     "          add    r18, %0\n"

     // 1) ---------- 56x 0x4E (pre-index gap)
     //
     // 0x4E             0x4E             ...
     //  0 1 0 0 1 1 1 0  0 1 0 0 1 1 1 0 ...
     // 1001001001010100 1001001001010100 ...
     // M  M  M  M S S   M  M  M  M S S   ...
     // => (MMMMSS)x56
     "          ldi    r20, 56\n"          // (1) write 56 gap bytes
     "          call   wrtgap\n"           //     returns 20 cycles after final pulse was written

     // 2) ---------- 12x 0x00
     //
     // 0x4E             0x00             0x00             ...
     //  0 1 0 0 1 1 1 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0 ...
     // 1001001001010100 1010101010101010 1010101010101010 ...
     // S  M  M  M S S   M S S S S S S S  S S S S S S S S  ...
     // => MSx95
     "          WRTPM\n"                   // write medium pulse
     "          ldi    r20, 95\n"          // write 95 short pulses
     "          call   wrtshort\n"         // returns 20 cycles after final pulse was written
     
     // 3) ---------- 3x SYNC 0xC2
     //
     // 0x00              0xC2            0xC2             0xC2
     //  0 0 0 0 0 0 0 0  1 1 0 0*0 0 1 0  1 1 0 0*0 0 1 0  1 1 0 0*0 0 1 0
     // 1010101010101010 0101001000100100 0101001000100100 0101001000100100
     // S S S S S S S S   M S  M   L  M    L S  M   L  M    L S  M   L  M
     // => MSMLM(LSMLM)x2
     "          ldi    r20, 3\n"
     "          WRTPM\n"                   // write medium pulse (returns 14 cycles after pulse)
     "          rjmp   iskip\n"            // (2)
     "iloop:    WRTPL\n"                   // write long   pulse
     "iskip:    WRTPS\n"                   // write short  pulse
     "          WRTPM\n"                   // write medium pulse
     "          WRTPL\n"                   // write long   pulse
     "          WRTPM\n"                   // write medium pulse
     "          dec    r20\n"              // (1)
     "          brne   iloop\n"            // (1/2)

     // 4) ---------- index record (0xFC)
     //
     // 0xC2             0xFC            
     //  1 1 0 0*0 0 1 0  1 1 1 1 1 1 0 0
     // 0101001000100100 0101010101010010
     //  L S  M   L  M    L S S S S S  M 
     // => LSSSSSM
     "          WRTPL\n"                   // write long pulse (returns 14 cycles after pulse)
     "          ldi    r20, 5\n"           // (1) write 5 short pulses
     "          call   wrtshort\n"         // 6 cycles until timer update, 20 cycles after pulse
     "          WRTPM\n"                   // write medium pulse

     // 5) ---------- 50x 0x4E (post-index gap)
     //
     // 0xFC             0x4E             0x4E             ...
     //  1 1 1 1 1 1 0 0  0 1 0 0 1 1 1 0  0 1 0 0 1 1 1 0 ...
     // 0101010101010010 1001001001010100 1001001001010100 ...
     //  L S S S S S  M  S  M  M  M S S   M  M  M  M S S   ...
     // => SMMMSS (MMMMSS)x49
     "          ldi    r20, 49\n"           // (1) write 49 gap bytes
     "          WRTPS\n"                    // write short  pulse
     "          call   wrtgap2\n"           //     returns 20 cycles after final pulse was written

     // 6) ---------- 12x 0x00
     //
     // 0x4E             0x00             0x00             ...
     //  0 1 0 0 1 1 1 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0 ...
     // 1001001001010100 1010101010101010 1010101010101010 ...
     // S  M  M  M S S   M S S S S S S S  S S S S S S S S  ...
     // => MSx95
     "secstart: WRTPM\n"                   // write medium pulse
     "          ldi    r20, 95\n"          // write 95 short pulses
     "          call   wrtshort\n"         // returns 20 cycles after final pulse was written
     
     // 7) ---------- 3x SYNC 0xA1
     //
     //  0x00             0xA1             0xA1             0xA1
     //  0 0 0 0 0 0 0 0  1 0 1 0 0*0 0 1  1 0 1 0 0*0 0 1  1 0 1 0 0*0 0 1
     // 1010101010101010 0100010010001001 0100010010001001 0100010010001001
     // S S S S S S S S   M   L  M   L  M  S   L  M   L  M  S   L  M   L  M
     // => MLMLM(SLMLM)x2

     // do not have sufficient time after final pulse from "wrtsync" call
     // => only write two bytes in "wrtsync", write final pulses directly to save time
     "          ldi   r20, 2\n"             // only write first two bytes of sync
     "          call  wrtsync\n"            // returns 20 cycles after final pulse was written
     "          WRTPS\n"
     "          WRTPL\n"
     "          WRTPM\n"
     "          WRTPL\n"
     "          WRITEPULSE 2\n"             // write medium pulse, returns 10 cycles after pulse was written

     // 8) ---------- ID record plus first 0x4E: 0xFE (cylinder) (side) (sector) (length) (CRC1) (CRC2) 0x4E)
     //
     // 0xA1               ...  0x4E
     //  1 0 1 0 0*0 0 1   ...   0 1 0 0 1 1 1 0
     // 0100010010001001   ...  ??01001001010100
     //  S   L  M   L  M   ...  ?  ?  M  M S S
     // => (write pre-calculated bytes, starting odd)
     // worst case needs 20 cycles before timer is initialized
     "          sts     OCRL, r16\n"     // (2)   set up timer for "01" sequence
     "          ldi     r21, 0\n"        // (1)   initialize bit counter to fetch next byte
     "          ldi     r26, 8\n"        // (1)   initialize byte counter (8 bytes to write)
     // just wrote a "1" bit => must be followed by either "01" (for "1" bit) or "00" (for "0" bit)
     // (have time to fetch next bit during the leading "0")
     "fio:      dec     r21\n"           // (1)   decrement bit counter
     "          brpl    fio0\n"          // (1/2) skip the following if bit counter >=  0
     "          subi    r26, 1\n"        // (1)   subtract one from byte counter
     "          brmi    fidone\n"        // (1/2) done if byte counter <0
     "          ld	r20, Z+\n"       // (2)   get next byte
     "          ldi     r21, 7\n"        // (1)   reset bit counter (7 more bits after this first one)
     "fio0:     rol     r20\n"           // (1)   get next data bit into carry
     "          brcs    fio1\n"          // (1/2) jump if "1"
     // next bit is "0" => write "00"
     "          lds     r19,  OCRL\n"    // (2)   get current OCRxAL value
     "          add     r19,  %0\n"      // (2)   add one-bit time
     "          sts     OCRL, r19\n"     // (2)   set new OCRxAL value
     "          rjmp    fie\n"           // (2)   now even
     // next bit is "1" => write "01"
     "fio1:     WRITEPULSE\n"            // (7)   wait and write pulse
     "          sts     OCRL, r16\n"     // (2)   set up timer for another "01" sequence
     "          rjmp    fio\n"           // (2)   still odd
     // just wrote a "0" bit, (i.e. either "10" or "00") where time for the trailing "0" was already added
     // to the pulse length (have time to fetch next bit during the already-added "0")
     "fie:      dec     r21\n"           // (1)   decrement bit counter
     "          brpl    fie0\n"          // (1/2) skip the following if bit counter >=  0
     "          subi    r26, 1\n"        // (1)   subtract one from byte counter
     "          brmi    fidone\n"        // (1/2) done if byte counter <0
     "          ld	r20, Z+\n"       // (2)   get next byte
     "          ldi     r21, 7\n"        // (1)   reset bit counter (7 more bits after this first one)
     "fie0:     rol     r20\n"           // (1)   get next data bit into carry
     "          brcs    fie1\n"          // (1/2) jump if "1"
     // next bit is "0" => write "10"
     "          WRITEPULSE\n"            // (7)   wait and write pulse
     "          sts     OCRL, r16\n"     // (2)   set up timer for another "10" sequence
     "          rjmp    fie\n"           // (2)   still even
     // next bit is "1" => write "01"
     "fie1:     lds     r19,  OCRL\n"    // (2)   get current OCRxAL value
     "          add     r19,  %0\n"      // (2)   add one-bit time
     "          sts     OCRL, r19\n"     // (2)   set new OCRxAL value
     "          WRITEPULSE\n"            // (7)   wait and write pulse
     "          sts     OCRL, r16\n"     // (2)   set up timer for "01" sequence
     "          rjmp    fio\n"           // (2)   now odd
     "fidone:   \n"

     // 9) ---------- 21x 0x4E (post-ID gap)
     //
     // 0x4E             0x4E             ...
     //  0 1 0 0 1 1 1 0  0 1 0 0 1 1 1 0 ...
     // 1001001001010100 1001001001010100 ...
     // S  M  M  M S S   M  M  M  M S S   ...
     // => (MMMMSS)x21
     "          ldi    r20, 21\n"          // (1) write 21 gap bytes
     "          call   wrtgap\n"           //     returns 20 cycles after final pulse was written

     // 10) ---------- 12x 0x00
     //
     // 0x4E             0x00             0x00             ...
     //  0 1 0 0 1 1 1 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0 ...
     // 1001001001010100 1010101010101010 1010101010101010 ...
     // S  M  M  M S S   M S S S S S S S  S S S S S S S S  ...
     // => MSx95
     "          WRTPM\n"                   // write medium pulse
     "          ldi    r20, 95\n"          // write 95 short pulses
     "          call   wrtshort\n"         // returns 20 cycles after final pulse was written
     
     // 11) ---------- 3x SYNC 0xA1
     //
     //  0x00             0xA1             0xA1             0xA1
     //  0 0 0 0 0 0 0 0  1 0 1 0 0*0 0 1  1 0 1 0 0*0 0 1  1 0 1 0 0*0 0 1
     // 1010101010101010 0100010010001001 0100010010001001 0100010010001001
     // S S S S S S S S   M   L  M   L  M  S   L  M   L  M  S   L  M   L  M
     // => MLMLM(SLMLM)x2
     "          ldi   r20, 3\n"            // write three sync bytes
     "          call  wrtsync\n"           // returns 20 cycles after final pulse was written

     // 12) ---------- data record 0xFB
     //
     //  0xA1               0xFB
     //  1 0 1 0 0*0 0 1  1 1 1 1 1 0 1 1 
     // 0100010010001001 0101010101000101
     //  S   L  M   L  M  S S S S S   L S
     // => SSSSSLS
     "          ldi    r20, 5\n"           // write 5 short pulses
     "          call   wrtshort\n"         // returns 20 cycles after final pulse was written
     "          WRTPL\n"                   // write long pulse
     "          WRTPS\n"                   // write short pulse

     // 13) ---------- data (512x 0xF6)
     //
     //  0xFB             0xF6             0xF6            ...
     //  1 1 1 1 1 0 1 1  1 1 1 1 0 1 1 0  1 1 1 1 0 1 1 0 ...
     // 0101010101000101 0101010100010100 0101010100010100 ...
     //  S S S S S   L S  S S S S   L S    L S S S   L S   ...
     // => SSSSLS LSSSLS
     "          ldi   r26, 0\n"            // write 2*256+0 = 512 bytes
     "          ldi   r27, 2\n"
     "          WRTPS\n"                   // write short pulse
     "          rjmp dskip\n"
     "dloop:    WRTPL\n"                   // write long  pulse
     "dskip:    WRTPS\n"                   // write short pulse
     "          WRTPS\n"                   // write short pulse
     "          WRTPS\n"                   // write short pulse
     "          WRTPL\n"                   // write long  pulse
     "          WRTPS\n"                   // write short pulse
     "          dec  r26\n"                // decrement byte counter (low)
     "          brne   dloop\n"            // loop until 0
     "          dec  r27\n"                // decrement byte counter (high)
     "          brne   dloop\n"            // loop until 0

     // 14) ---------- data checksum (0x2B, 0xF6)
     //
     //  0xF6             0x2B             0xF6
     //  1 1 1 1 0 1 1 0  0 0 1 0 1 0 1 1  1 1 1 1 0 1 1 0
     // 0101010100010100 1010010001000101 0101010100010100
     //  L S S S   L S   M S  M   L   L S  S S S S   L S
     // => MSMLSLSSSSLS
     "          WRTPM\n"                   // write medium pulse
     "          WRTPS\n"                   // write short  pulse
     "          WRTPM\n"                   // write medium pulse
     "          WRTPL\n"                   // write long   pulse
     "          WRTPL\n"                   // write short  pulse
     "          WRTPS\n"                   // write long   pulse
     "          WRTPS\n"                   // write short  pulse
     "          WRTPS\n"                   // write short  pulse
     "          WRTPS\n"                   // write short  pulse
     "          WRTPS\n"                   // write short  pulse
     "          WRTPL\n"                   // write long   pulse
     "          WRTPS\n"                   // write short  pulse
     
     // if this was the last sector then the track is done
     "          dec    %1\n"              // (1)    decrement sector counter
     "          breq   ftdone\n"          // (1/2)  jump if done

     // 15) ---------- 54/102x 0x4E (post-data gap)
     //
     //  0xF6             0x4E             0x4E             ...
     //  1 1 1 1 0 1 1 0  0 1 0 0 1 1 1 0  0 1 0 0 1 1 1 0 ...
     // 0101010100010100 1001001001010100 1001001001010100 ...
     //  S S S S   L S   M  M  M  M S S   ...
     // => (MMMMSS) x datagaplen
     "          mov    r20, %2\n"         // (1) write "datagaplen" gap bytes
     "          call   wrtgap\n"          //     returns 20 cycles after final pulse was written
     "          rjmp   secstart\n"        // (2) repeat for next sector

     // ---------- track format done => write GAP bytes (0x4E) until INDEX hole seen
     "ftdone:   ldi    r20, 1\n"           // (1)   write one byte
     "          call   wrtgap\n"           //       returns 20 cycles after final pulse was written
     "          lds    r20, IDXPORT\n"     // (2)   read INDEX signal
     "          sbrc   r20, IDXBIT\n"      // (1/2) skip next instruction if PIND7 (INDEX) is LOW
     "          rjmp   ftdone\n"           // (2)   write more gap bytes
     "          rjmp   ftend\n"            //       done

     // -------------- subroutines

     // write short pulses
     //  r20 contains number of short pulses to write
     //  => takes 6 cycles until timer is initialized (including call)
     //  => returns 20 cycles (max) after final pulse is written (including return statement)
     "wrtshort: WRTPS\n"
     "          dec r20\n"                 // (1)
     "          brne wrtshort\n"           // (1/2)
     "          ret\n"                     // (4)

     // write gap (0x4E) => (MMMMSS) x r20
     //  r20 contains number of gap bytes to write
     //  => takes 6 cycles until timer is initialized (including call)
     //  => returns 20 cycles (max) after final pulse is written (including return statement)
     "wrtgap:   WRTPM\n"
     "wrtgap2:  WRTPM\n"
     "          WRTPM\n"
     "          WRTPM\n"
     "          WRTPS\n"
     "          WRTPS\n"                
     "          dec r20\n"                 // (1)
     "          brne wrtgap\n"             // (1/2)
     "          ret\n"                     // (4)

     // write SYNC (0xA1 with missing clock bit) => MLMLM (SLMLM) x r20
     //  r20 contains nyumber of SYNC bytes to write
     //  => takes 7 cycles until timer is initialized (including call)
     //  => returns 20 cycles (max) after final pulse is written (including return statement)
     "wrtsync:  WRTPM\n"                   // write medium pulse (returns 14 cycles after pulse)
     "          rjmp   sskip\n"            // (2)
     "sloop:    WRTPS\n"                   // write short  pulse
     "sskip:    WRTPL\n"                   // write long   pulse
     "          WRTPM\n"                   // write medium pulse
     "          WRTPL\n"                   // write long   pulse
     "          WRTPM\n"                   // write medium pulse
     "          dec    r20\n"              // (1)
     "          brne   sloop\n"            // (1/2)
     "          ret\n"                     // (4)   return
     
     // wait for pulse to be written
     // => returns 14 cycles (max) after pulse is written (including return statement)
     "waitp:    sbis  TIFR, OCF\n"         // (1/2) skip next instruction if OCFx is set
     "          rjmp  .-4\n"               // (2)   wait more
     "          ldi   r19,  FOC\n"         // (1)
     "          sts   TCCRC, r19\n"        // (2)   set OCP back HIGH (was set LOW when timer expired)
     "          sbi   TIFR, OCF\n"         // (2)   reset OCFx (output compare flag)
     "          ret\n"                     // (4)   return

     "ftend:\n"

     :                                            // no outputs
     : "r"(bitlen), "r"(numsec), "r"(datagaplen), "z"(buffer)      // inputs  (z=r30/r31)
     : "r16", "r17", "r18", "r19", "r20", "r21", "r26", "r27"); // clobbers

  // set WRITEGATE back to input (releases it HIGH)
  WGPORT &= ~bit(WGBIT);

  // COMxA1:0 = 00 => disconnect OC1A (will go high)
  TCCRA = 0;

  // WGMx2:10 = 000 => Normal timer mode
  TCCRB &= ~bit(WGM2);

  interrupts();

  return S_OK;
}


static byte wait_header(byte bitlen, byte track, byte side, byte sector)
{
  byte attempts = 50;

  // check whether we can see any data pulses from the drive at all
  if( !check_pulse() )
    {
#ifdef DEBUG
      Serial.println(F("Drive not ready!")); Serial.flush();
#endif
      return S_NOTREADY;
    }
  
  do
    {
      // wait for sync sequence and read 7 bytes of data
      byte status = read_data(bitlen, header, 7, false);

      if( status==S_OK )
        {
          // make sure this is an ID record and check whether it contains the
          // expected track/side/sector information and the CRC is ok
          if( header[0]==0xFE && (track==0xFF || track==header[1]) && side==header[2] && sector==header[3] )
            {
              if( calc_crc(header, 5) == 256u*header[5]+header[6] )
                return S_OK;
#ifdef DEBUG
              else
                { Serial.println(F("Header CRC error!")); Serial.flush(); }
#endif
            }
#ifdef DEBUG
          else
            {
              static const char hex[17] = "0123456789ABCDEF";
              Serial.write('H');
              for(byte i=0; i<5; i++) { Serial.write(hex[header[i]/16]); Serial.write(hex[header[i]&15]); }
              Serial.write(calc_crc(header, 5) == 256u*header[5]+header[6] ? '+' : '-');
              Serial.write(10);
            }
#endif
        }
      else
        return status;
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
  digitalWriteOC(PIN_STEP, LOW);
  delay(1);
  digitalWriteOC(PIN_STEP, HIGH);
  delay(10);
}


static bool step_to_track0()
{
  byte n = 82;

  // step outward until TRACK0 line goes low
  digitalWriteOC(PIN_STEPDIR, HIGH);
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


static void step_tracks(byte driveType, int tracks)
{
  // if tracks<0 then step outward (outward towards track 0) otherwise step inward
  digitalWriteOC(PIN_STEPDIR, tracks<0 ? HIGH : LOW);
  tracks = abs(tracks);
  tracks = tracks * geometry[driveType].trackSpacing;
  while( tracks-->0 ) step_track();
  delay(100); 
}


static byte find_sector(byte driveType, byte bitLength, byte track, byte side, byte sector)
{
  // select side
  digitalWriteOC(PIN_SIDE, side>0 ? LOW : HIGH);

  // wait for sector header
  byte res = wait_header(bitLength, -1, side, sector);

  // if we found the sector header but it's not on the correct track then step to correct track and check again
  if( res==S_OK && header[1]!=track )
    {
      // make sure that the track number in the header we read is sensible
      if( header[1]<geometry[driveType].numTracks )
        {
          // need interrupts for delay() when stepping
          interrupts();
          step_tracks(driveType, track-header[1]);
          noInterrupts();

          res = wait_header(bitLength, track, side, sector);
        }
      else
        res = S_NOHEADER;
    }

  // if we couldn't find the header then step to correct track by going to track 0 and then stepping out and check again
  if( res!=S_OK )
    {
      // need interrupts for delay() when stepping
      interrupts();
      if( step_to_track0() )
        {
          step_tracks(driveType, track);
          noInterrupts();
          res = wait_header(bitLength, track, side, sector);
        }
      else
        {
          noInterrupts();
          res = S_NOTRACK0;
        }
    }

  return res;
}


// --------------------------------------------------------------------------------------------------


ArduinoFDCClass::ArduinoFDCClass() 
{ 
  m_initialized   = false;
  m_currentDrive  = 0;
  m_motorState[0] = false;
  m_motorState[1] = false; 
  m_driveType[0]  = DT_3_HD;
  m_driveType[1]  = DT_3_HD;
  m_bitLength[0]  = 0;
  m_bitLength[1]  = 0;

#if defined(PIN_DENSITY)
  m_densityPinMode[0] = DP_DISCONNECT;
  m_densityPinMode[1] = DP_DISCONNECT;
#endif
}


void ArduinoFDCClass::begin(enum DriveType driveAType, enum DriveType driveBType)
{
  // make sure all outputs pins are HIGH when we switch them to output mode
  digitalWrite(PIN_STEP,      LOW);
  digitalWrite(PIN_STEPDIR,   LOW);
  digitalWrite(PIN_MOTORA,    LOW);
  digitalWrite(PIN_SELECTA,   LOW);
#if defined(PIN_MOTORB) && defined(PIN_SELECTB)
  digitalWrite(PIN_MOTORB,    LOW);
  digitalWrite(PIN_SELECTB,   LOW);
#endif
  digitalWrite(PIN_SIDE,      LOW);
  digitalWrite(PIN_WRITEGATE, LOW);
  digitalWrite(PIN_WRITEDATA, HIGH);

  // set pins to input/output mode
  pinMode(PIN_STEP,      INPUT);
  pinMode(PIN_STEPDIR,   INPUT);
  pinMode(PIN_SELECTA,   INPUT);
  pinMode(PIN_MOTORA,    INPUT);
#if defined(PIN_MOTORB) && defined(PIN_SELECTB)
  pinMode(PIN_SELECTB,   INPUT);
  pinMode(PIN_MOTORB,    INPUT);
#endif
  pinMode(PIN_SIDE,      INPUT);
  pinMode(PIN_WRITEGATE, INPUT);
  pinMode(PIN_WRITEDATA, OUTPUT);
  pinMode(PIN_READDATA,  INPUT_PULLUP);
  pinMode(PIN_INDEX,     INPUT_PULLUP);
  pinMode(PIN_TRACK0,    INPUT_PULLUP);
#if defined(PIN_WRITEPROT)
  pinMode(PIN_WRITEPROT, INPUT_PULLUP);
#endif

#if defined(PIN_DENSITY)
  digitalWrite(PIN_DENSITY, LOW);
  pinMode(PIN_DENSITY, INPUT);
#endif

  m_bitLength[0] = 0;
  m_bitLength[1] = 0;
  m_motorState[0] = false;
  m_motorState[1] = false;

  m_currentDrive  = 1;
  setDriveType(driveBType);
  m_currentDrive  = 0;
  setDriveType(driveAType);
  m_initialized   = true;
}


void ArduinoFDCClass::end()
{
  m_initialized = false;
  m_motorState[0] = false;
  m_motorState[1] = false;
  digitalWriteOC(PIN_MOTORA, LOW);
#if defined(PIN_MOTORB) && defined(PIN_SELECTB)
  digitalWriteOC(PIN_MOTORB, LOW);
#endif
}


void ArduinoFDCClass::setDriveType(enum DriveType type)
{
  if( type != m_driveType[m_currentDrive] )
    {
      m_driveType[m_currentDrive] = type;

      // by default: 3.5"     drives do not use DENSITY pin (disconnect)
      //             5.25  DD drives do not use DENSITY pin (disconnect)
      //             5.25" HD drives expect DENSITY to be LOW for low density
      if( type==DT_5_DDonHD || type==DT_5_HD )
        setDensityPinMode(DP_OUTPUT_LOW_FOR_DD);
      else
        setDensityPinMode(DP_DISCONNECT);
      
      // bit length will be determined at first read/write operation
      m_bitLength[m_currentDrive] = 0;
    }
}


enum ArduinoFDCClass::DriveType ArduinoFDCClass::getDriveType() const
{
  return m_driveType[m_currentDrive];
}


void ArduinoFDCClass::setDensityPinMode(enum DensityPinMode mode)
{
#if defined(PIN_DENSITY)
  m_densityPinMode[m_currentDrive] = mode;
  setDensityPin();
#endif
}


void ArduinoFDCClass::setDensityPin()
{
#if defined(PIN_DENSITY)
  byte isHD = m_driveType[m_currentDrive]==DT_3_HD || m_driveType[m_currentDrive]==DT_5_HD;
  switch( m_densityPinMode[m_currentDrive] )
    {
    case DP_OUTPUT_LOW_FOR_DD:
      digitalWriteOC(PIN_DENSITY, isHD ? HIGH : LOW);
      break;
      
    case DP_OUTPUT_LOW_FOR_HD:
      digitalWriteOC(PIN_DENSITY, isHD ? LOW : HIGH);
      break;

    default:
      digitalWriteOC(PIN_DENSITY, HIGH);
    }
#endif 
}


void ArduinoFDCClass::driveSelect(bool state) const
{
#if defined(PIN_MOTORB) && defined(PIN_SELECTB)
  digitalWriteOC(m_currentDrive==0 ? PIN_SELECTA : PIN_SELECTB, state);
#else
  digitalWriteOC(PIN_SELECTA, state);
#endif
}


byte ArduinoFDCClass::getBitLength()
{
  if( m_bitLength[m_currentDrive] == 0 )
    {
      byte bitLength;

      switch( m_driveType[m_currentDrive] )
        {
        case DT_3_HD: bitLength = 16; break;
        case DT_3_DD: bitLength = 32; break;
        case DT_5_HD: bitLength = 16; break;
        case DT_5_DD: bitLength = 32; break;

        case DT_5_DDonHD:
          {
            TCCRA = 0;
            TCCRB = bit(CS0);  // start timer with /1 prescaler
            TCCRC = 0;

            // return with error if index hole can't be found
            if( !wait_index_hole() ) return 0;

            // switch timer to /64 prescaler
            TCCRB = bit(CS0) | bit(CS1);

            // build average tick count (4us/tick) over 4 revolutions
            unsigned long l = 0;
            for(byte i=0; i<4; i++)
              {
                if( !wait_index_hole() ) return 0;
                l += TCNT;
              }

            TCCRB = 0; // turn off timer

            // for 300 RPM (200 ms/rotation) data rate is 250 mbps => 32 cycles/bit
            // for 360 RPM (166 ms/rotation) data rate is 300 mbps => 27 cycles/bit
            bitLength = l > 180000 ? 32 : 27;
            break;
          }
        }

      m_bitLength[m_currentDrive] = bitLength;
    }

  return m_bitLength[m_currentDrive];
}


byte ArduinoFDCClass::readSector(byte track, byte side, byte sector, byte *buffer)
{
  byte res = S_OK;
  byte driveType = m_driveType[m_currentDrive];

  // do some sanity checks
  if( !m_initialized )
    return S_NOTINIT;
  else if( track>=geometry[driveType].numTracks || sector<1 || sector>geometry[driveType].numSectors || side>1 )
    return S_NOHEADER;

  // if motor is not running then turn it on now
  bool turnMotorOff = false;
  if( !motorRunning() )
    {
      turnMotorOff = true;
      motorOn();
    }

  // assert DRIVE_SELECT
  driveSelect(LOW);

  // get MFM bit length (in processor cycles, motor must be running for this)
  byte bitLength = getBitLength();

  if( bitLength==0 )
    res = S_NOTREADY;
  else
    {
      // set up timer
      TCCRA = 0;
      TCCRB = bit(CS0); // falling edge input capture, prescaler 1, no output compare
      TCCRC = 0;
      
      // reading data is time sensitive so we can't have interrupts
      noInterrupts();

      // find the requested sector
      res = find_sector(driveType, bitLength, track, side, sector);
      
      // if we found the sector then read the data
      if( res==S_OK )
        {
          // wait for data sync mark and read data
          if( read_data(bitLength, buffer, 515, false)==S_OK )
            {
              if( buffer[0]!=0xFB )
                { 
#ifdef DEBUG
                  Serial.println(F("Unexpected record identifier")); 
                  for(int i=0; i<7; i++) { Serial.print(buffer[i], HEX); Serial.write(' '); }
                  Serial.println();
#endif
                  res = S_INVALIDID;
                }
              else if( calc_crc(buffer, 513) != 256u*buffer[513]+buffer[514] )
                { 
#ifdef DEBUG
                  Serial.print(F("Data CRC error. Found: ")); Serial.print(256u*buffer[513]+buffer[514], HEX); Serial.print(", expected: "); Serial.println(calc_crc(buffer, 513), HEX);
#endif
                  res = S_CRC; 
                }
            }
        }

      // interrupts are ok again
      interrupts();

      // stop timer
      TCCRB = 0;
    }

  // de-assert DRIVE_SELECT
  driveSelect(HIGH);

  // if we turned the motor on then turn it off again
  if( turnMotorOff ) motorOff();

  return res;
}


byte ArduinoFDCClass::writeSector(byte track, byte side, byte sector, byte *buffer, bool verify)
{
  byte res = S_OK;
  byte driveType = m_driveType[m_currentDrive];

  // do some sanity checks
  if( !m_initialized )
    return S_NOTINIT;
  else if( track>=geometry[driveType].numTracks || sector<1 || sector>geometry[driveType].numSectors || side>1 )
    return S_NOHEADER;

  // if motor is not running then turn it on now
  bool turnMotorOff = false;
  if( !motorRunning() )
    {
      turnMotorOff = true;
      motorOn();
    }

  // assert DRIVE_SELECT
  driveSelect(LOW);

  // get MFM bit length (in processor cycles, motor must be running for this)
  byte bitLength = getBitLength();

  // set up timer
  TCCRA = 0;
  TCCRB = bit(CS0); // falling edge input capture, prescaler 1, no output compare
  TCCRC = 0;

  // check write protect (drive must be selected for this)
  if( bitLength==0 )
    res = S_NOTREADY;
  else if( is_write_protected() )
    res = wait_index_hole() ? S_READONLY : S_NOTREADY;
  else
    {
      // calculate CRC for the sector data
      buffer[0]   = 0xFB; // "data" id
      uint16_t crc = calc_crc(buffer, 513);
      buffer[513] = crc/256;
      buffer[514] = crc&255;
      buffer[515] = 0x4E; // first byte of post-data gap
  
      // writing data is time sensitive so we can't have interrupts
      noInterrupts();

      // find the requested sector
      res = find_sector(driveType, bitLength, track, side, sector);

      // if we found the sector then write the data
      if( res==S_OK )
        {
          // write the sector data
          write_data(bitLength, buffer, 516);
          
          // if we are supposed to verify the data then do so now
          if( verify )
            {
              // wait for sector to come around again
              res = wait_header(bitLength, track, side, sector);
              
              // wait for data sync mark and compare the data
              if( res==S_OK ) res = read_data(bitLength, buffer, 515, true);
            }
        }

      // interrupts are ok again
      interrupts();
    }

  // de-assert DRIVE_SELECT
  driveSelect(HIGH);

  // stop timer
  TCCRB = 0;

  // if we turned the motor on then turn it off again
  if( turnMotorOff ) motorOff();

  return res;
}


byte ArduinoFDCClass::formatDisk(byte *buffer, byte fromTrack, byte toTrack)
{
  byte res = S_OK;
  byte driveType = m_driveType[m_currentDrive];
  byte numTracks = geometry[driveType].numTracks;

  // do some sanity checks
  if( !m_initialized )
    return S_NOTINIT;
  else if( fromTrack>toTrack || fromTrack >= numTracks )
    return S_OK;

  // if motor is not running then turn it on now
  bool turnMotorOff = false;
  if( !motorRunning() )
    {
      turnMotorOff = true;
      motorOn();
    }

  // assert DRIVE_SELECT
  driveSelect(LOW);

  // get MFM bit length (in processor cycles, motor must be running for this)
  byte bitLength = getBitLength();

  // set up timer
  TCCRA = 0;
  TCCRB = bit(CS0); // prescaler 1
  TCCRC = 0;

  if( bitLength==0 || !wait_index_hole() )
    res = S_NOTREADY;
  else if( is_write_protected() )
    res = S_READONLY;
  else if( !step_to_track0() )
    res = S_NOTRACK0;
  else
    {
      if( fromTrack>0 ) step_tracks(driveType, fromTrack);
      if( toTrack>=numTracks ) toTrack = numTracks-1;
      for(byte track=fromTrack; track<=toTrack; track++)
        {
          digitalWriteOC(PIN_SIDE, HIGH);
          res = format_track(buffer, driveType, bitLength, track, 0); if( res!=S_OK ) break;
          digitalWriteOC(PIN_SIDE, LOW);
          res = format_track(buffer, driveType, bitLength, track, 1); if( res!=S_OK ) break;
          if( track+1<=toTrack ) step_tracks(driveType, 1);
        }
    }

  // de-assert DRIVE_SELECT
  driveSelect(HIGH);

  // if we turned the motor on then turn it off again
  if( turnMotorOff ) motorOff();

  // stop timer
  TCCRB = 0;

  return res;
}


void ArduinoFDCClass::motorOn()
{
  if( !motorRunning() )
    {
#if defined(PIN_MOTORB) && defined(PIN_SELECTB)
      digitalWriteOC(m_currentDrive==0 ? PIN_MOTORA : PIN_MOTORB, LOW);
#else
      digitalWriteOC(PIN_MOTORA, LOW);
#endif
      
      m_motorState[m_currentDrive] = true;
      
      // allow some time for the motor to spin up
      delay(1000);
    }
}


void ArduinoFDCClass::motorOff()
{
#if defined(PIN_MOTORB) && defined(PIN_SELECTB)
  digitalWriteOC(m_currentDrive==0 ? PIN_MOTORA : PIN_MOTORB, HIGH);
#else
  digitalWriteOC(PIN_MOTORA, HIGH);
#endif

  m_motorState[m_currentDrive] = false;
}


bool ArduinoFDCClass::motorRunning() const
{ 
  return m_motorState[m_currentDrive];
}



bool ArduinoFDCClass::selectDrive(byte drive)
{
  if( drive == m_currentDrive )
    return true;
  else
    {
#if defined(PIN_MOTORB) && defined(PIN_SELECTB)
      m_currentDrive = (drive==0) ? 0 : 1;
      setDensityPin();
      return true;
#else
#ifdef DEBUG
      if( drive!=0 ) Serial.println(F("PIN_MOTORB and/or PIN_SELECTB not defined - Can only control drive A"));
#endif
      return false;
#endif
    }
}


bool ArduinoFDCClass::haveDisk() const
{
  bool res = false;

  // set up and start timer (prescaler 1), select drive
  TCCRA = 0;
  TCCRB = bit(CS0);
  TCCRC = 0;
  driveSelect(LOW);

  if( motorRunning() )
    res = wait_index_hole();
  else
    {
#if defined(PIN_MOTORB) && defined(PIN_SELECTB)
      digitalWriteOC(m_currentDrive==0 ? PIN_MOTORA : PIN_MOTORB, LOW);
      res = wait_index_hole();
      digitalWriteOC(m_currentDrive==0 ? PIN_MOTORA : PIN_MOTORB, HIGH);
#else
      digitalWriteOC(PIN_MOTORA, LOW);
      res = wait_index_hole();
      digitalWriteOC(PIN_MOTORA, HIGH);
#endif
    }

  // de-select drive, stop timer
  driveSelect(HIGH);
  TCCRB = 0;

  return res;
}


bool ArduinoFDCClass::isWriteProtected() const
{
  bool res = false;

#if defined(PIN_WRITEPROT)
  driveSelect(LOW);
  res = is_write_protected();
  driveSelect(HIGH);
#endif

  return res;
}


byte ArduinoFDCClass::selectedDrive() const
{
  return m_currentDrive;
}


byte ArduinoFDCClass::numTracks() const
{
  return geometry[m_driveType[m_currentDrive]].numTracks;
}


byte ArduinoFDCClass::numSectors() const
{
  return geometry[m_driveType[m_currentDrive]].numSectors;
}
