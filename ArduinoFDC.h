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
#define S_NOTRACK0   7  // No track0 signal
#define S_VERIFY     8  // Verify after write failed
#define S_READONLY   9  // Attempt to write to a write-protected disk

class ArduinoFDCClass
{
 public:

  enum DriveType {
    DT_5_DD,     // 5.25" double density (360 KB) 
    DT_5_DDonHD, // 5.25" double density disk in high density drive (360 KB)
    DT_5_HD,     // 5.25" high density (1.2 MB)
    DT_3_DD,     // 3.5"  double density (720 KB)
    DT_3_HD      // 3.5"  high density (1.44 MB)
  };

  enum DensityPinMode {
    DP_DISCONNECT = 0,    // density pin disconnected (set to INPUT mode)
    DP_OUTPUT_LOW_FOR_HD, // density pin goes LOW for high density disk
    DP_OUTPUT_LOW_FOR_DD  // density pin goes LOW for double density disk
  };
  
  ArduinoFDCClass();
  
  // Initialize pins used for controlling the disk drive
  void begin(enum DriveType driveAType = DT_3_HD, enum DriveType driveBType = DT_3_HD);

  // Release pins used for controlling the disk drive
  void end();

  // Select drive A(0) or B(1). 
  bool selectDrive(byte drive);
  
  // Returns which drive is currently selected, A (0) or B (1)
  byte selectedDrive() const;

  // set the drive type for the currently selected drive
  void setDriveType(enum DriveType type);

  // get the type of the currently selected drive
  enum DriveType getDriveType() const;

  // returns true if a disk is detected in the drive
  bool haveDisk() const;
  
  // returns true if the disk is write protected
  bool isWriteProtected() const;

  // get number of tracks for the currently selected drive
  byte numTracks() const;

  // get number of sectors for the currently selected drive
  byte numSectors() const;

  // set the density pin mode for the currently selected drive
  void setDensityPinMode(enum DensityPinMode mode);
  
  // Read a sector from disk,
  // buffer MUST have a size of at least 516 bytes. 
  // IMPORTANT: On successful return, the 512 bytes of sector data 
  //            read will be in buffer[1..512] (NOT: 0..511!)
  // See error codes above for possible return values
  byte readSector(byte track, byte side, byte sector, byte *buffer);

  // Write a sector to disk,
  // buffer MUST have a size of at least 516 bytes.
  // IMPORTANT: The 512 bytes of sector data to be written
  //            must be in buffer[1..512] (NOT: 0..511!)
  // if "verify" is true then the data will be re-read after writing
  // and compared to the data just written.
  // See error codes above for possible return values
  byte writeSector(byte track, byte side, byte sector, byte *buffer, bool verify);

  // Formats a disk
  // buffer is needed to store temporary data while formatting and MUST have
  // a size of at least 144 bytes.
  // All sector data is initialized with 0xF6.
  // See error codes above for possible return values
  // IMPORTANT: No DOS file system is initialized, i.e. DOS or Windows
  //            will NOT recognize this as a valid disk
  byte formatDisk(byte *buffer, byte fromTrack=0, byte toTrack=255);

  // Turn the disk drive motor on. The readSector/writeSector/formatDisk 
  // functions will turn on the motor automatically if it is not running 
  // yet. In that case (and ONLY then) they will also turn it off when finished.
  void motorOn();

  // Turn the disk drive motor off
  void motorOff();

  // Returns true if the disk drive motor is currently running
  bool motorRunning() const;

 private:
  void driveSelect(bool state) const;
  void setDensityPin();
  byte getBitLength();

  enum DriveType m_driveType[2];
  enum DensityPinMode m_densityPinMode[2];
  byte m_currentDrive, m_bitLength[2];
  bool m_initialized, m_motorState[2];
};


extern ArduinoFDCClass ArduinoFDC;

#endif
