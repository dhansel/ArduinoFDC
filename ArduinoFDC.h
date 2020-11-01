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

#ifndef ARDUINOFDC_H
#define ARDUINOFDC_H

#include "Arduino.h"

// return status for readSector/writeSector and formatDisk functions
#define S_OK         0  // no error
#define S_NOTINIT    1  // ArduinoFDC.begin() was not called
#define S_NOTREADY   2  // Drive is not ready (no disk or power)
#define S_NOSYNC     3  // No sync marks found
#define S_NOHEADER   4  // Sector header not found
#define S_INVALIDID  5  // Sector data record has invalid id
#define S_CRC        6  // Sector data checksum error
#define S_NOINDEX    7  // No index hole signal
#define S_NOTRACK0   8  // No track0 signal
#define S_VERIFY     9  // Verify after write failed


class ArduinoFDCClass
{
 public:
  ArduinoFDCClass();
  
  // Initialize pins used for controlling the disk drive
  void begin();

  // Release pins used for controlling the disk drive
  void end();

  // Select drive A(0) or B(1). 
  void selectDrive(byte drive);

  // Returns which drive is currently selected, A (0) or B (1)
  byte selectedDrive();

  // Read a sector from disk (track 0..79, side 0..1, sector 1..9),
  // buffer MUST have a size of at least 515 bytes. 
  // IMPORTANT: On successful return, the 512 bytes of sector data 
  //            read will be in buffer[1..512] (NOT: 0..511!)
  // See error codes above for possible return values
  byte readSector(byte track, byte side, byte sector, byte *buffer);

  // Write a sector to disk (track 0..79, side 0..1, sector 1..9),
  // buffer MUST have a size of at least 515 bytes.
  // IMPORTANT: The 512 bytes of sector data to be written
  //            must be in buffer[1..512] (NOT: 0..511!)
  // if "verify" is true then the data will be re-read after writing
  // and compared to the data just written.
  // See error codes above for possible return values
  byte writeSector(byte track, byte side, byte sector, byte *buffer, bool verify);

  // Formats a disk (2 sides, 80 tracks per side, 9 sectors per track)
  // All sector data is initialized with 0xF6.
  // See error codes above for possible return values
  // IMPORTANT: No DOS file system is initialized, i.e. DOS or Windows
  //            will NOT recognize this as a valid disk
  byte formatDisk();

  // Turn on the disk drive motor. The readSector/writeSector/formatDisk 
  // functions will turn on the motor automatically if it is not running 
  // yet. In that case (and ONLY then) they will also turn it off when finished.
  void motorOn();

  // Turn off the disk drive motor
  void motorOff();

  // Returns true if the disk drive motor is currently running
  bool motorRunning();

 private:
  bool initialized, driveA, motorStateA, motorStateB;
};


extern ArduinoFDCClass ArduinoFDC;

#endif
