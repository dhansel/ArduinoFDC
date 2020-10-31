# ArduinoFDC
Library for using an Arduino UNO as a 3.5" double density floppy disk controller


This is a small library containing low-level functions which enable an
Arduino UNO (or any ATMega328P) to control a 3.5" floppy disk drive.
It allows to read and write disks at the sector level as well as 
formatting disks.

Note that this controller only works as a double density (DD) controller,
reading/writing up to 720KB per disk. It does not work with disks formatted
as HD (1.44MB). It can however format a HD disk with DD format and then
write to and read from it.

## Functions:

After adding ArduinoFDC.h and ArduinoFDC.cpp to your Arduino sketch you
can use the following functions:

#### `void ArduinoFDC.begin()`
Initializes the Arduino pins used by the controller. Those pins are defined
at the top of the ArduinoFDC.cpp file. Some of them can easily be changed
whereas others are hard-coded in the controller code. Refer to the comments
at the top of the ArduinoFDC.cpp file if you want to change pin assignments.

#### `void ArduinoFDC.end()`
Releases the pins initialized by ArduinoFDC.begin()

#### `bool ArduinoFDC.readSector(byte track, byte side, byte sector, byte *buffer)`
Reads data from a sector from the flopy disk. Always reads a full sector (512 bytes).

* The "track" parameter must be in range 0..79
* The "side" parameter must either be 0 or 1
* The "sector" paramter must be in range 1..9
* The "buffer" parameter must be a pointer to a byte array of size (at least) 515.

The function returns *true* if reading succeeded and *false* if it failed.

**IMPORTANT:** On successful return, the sector data that was read will be in buffer[1..512] (**NOT** buffer[0..511])

#### `bool ArduinoFDC.writeSector(byte track, byte side, byte sector, byte *buffer, bool verify)`
Writes data to a sector on the floppy disk. Always writes a full sector (512 bytes).

* The "track" parameter must be in range 0..79
* The "side" parameter must either be 0 or 1
* The "sector" paramter must be in range 1..9
* The "buffer" parameter must be a pointer to a byte array of size (at least) 515.
* If the "verify" parameter is *true*, the data written will be read back and compared to what was written.
If a difference is detected then the function will return *false*.
If the "verify" parameter is *false* then no verification is done. The function may still return *false*
if the proper sector location on disk can not be found before writing.

The function returns *true* if writing succeeded and *false* if it failed.

**IMPORTANT:** The sector data to be written must be in buffer[1..512] (**NOT** buffer[0..511])

#### `bool ArduinoFDC.formatDisk()`
Formats a floppy disk with a double density (DD) 720K format.

The function returns *false* if the "Track 0" signal or the "Index" signal from
the floppy drive are not detected. Otherwise it always return *true*.
No verification of the formatting process is performed. You can use the `readSector`
function to verify that data can be read properly.

#### `void ArduinoFDC.motorOn()`
Turns the disk drive motor on. 

The `readSector`/`writeSector`/`formatDisk` functions will turn  the motor on **and** off 
automatically if it is not already running. Note that turning on the motor also includes
a one second delay to allow it to spin up.  If you are reading/writing multiple sectors
you may want to use the `motorOn` and `motorOff` functions to manually turn the motor on
and off.

#### `void ArduinoFDC.motorOff()`
Turns the disk drive motor off. 

#### `bool ArduinoFDC.motorRunning()`
Returns *true* if the disk drive motor is currently running and *false* if not.
