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

class ArduinoFDCClass
{
 public:
  ArduinoFDCClass();
  
  // Initialize pins used for controlling the disk drive
  void begin();

  // Release pins used for controlling the disk drive
  void end();

  // Read a sector from disk (track 0..79, side 0..1, sector 1..9),
  // buffer MUST have a size of at least 515 bytes. 
  // IMPORTANT: On successful return, the 512 bytes of sector data 
  //            read will be in buffer[1..512] (NOT: 0..511!)
  bool readSector(byte track, byte side, byte sector, byte *buffer);

  // Write a sector to disk (track 0..79, side 0..1, sector 1..9),
  // buffer MUST have a size of at least 515 bytes.
  // IMPORTANT: The 512 bytes of sector data to be written
  //            must be in buffer[1..512] (NOT: 0..511!)
  // if "verify" is true then the data will be re-read after writing
  // and compared to the data just written.
  bool writeSector(byte track, byte side, byte sector, byte *buffer, bool verify);

  // Formats a disk (2 sides, 80 tracks per side, 9 sectors per track)
  // All sector data is initialized with 0xF6.
  // IMPORTANT: No DOS file system is initialized, i.e. DOS or Windows
  //            will NOT recognize this as a valid disk
  bool formatDisk();

  // Turn on the disk drive motor. The readSector/writeSector/formatDisk 
  // functions will turn on the motor automatically if it is not running 
  // yet. In that case (and ONLY then) they will also turn it off when finished.
  void motorOn();

  // Turn off the disk drive motor
  void motorOff();

  // Returns true if the disk drive motor is currently running
  bool motorRunning() { return motorState; }

 private:
  bool motorState, initialized;
};


extern ArduinoFDCClass ArduinoFDC;

#endif
