# ArduinoFDC

This is a small library providing low-level functions which enable an
Arduino UNO (or any ATMega328P) to control a 3.5" or 5.25" floppy disk drive.
It supports reading and writing disks at the sector level as well as 
formatting disks.

This controller works with double density (DD) as well as high density (HD)
disk drives. It can read/write 5.25" DD (360KB), 5.25" HD (1.2MB), 3.5" DD (720KB)
and 3.5" HD (1.44MB) disks.

## Wiring:

The following table shows how to wire up the Arduino UNO pins to the 34-pin IDC
connector on the 3.5" floppy drive. 

The pin numbers are defined at the top of the ArduinoFDC.cpp file. Some of them can 
easily be changed whereas others are hard-coded in the controller code. Refer to 
the comments at the top of the ArduinoFDC.cpp file if you want to use different
pin assignments.

Arduino pin  | Floppy Drive pin | Notes  | Function
-------------|------------------|--------|----------------
2            | 20               | 3      | Head Step Pulse
3            | 18               | 3      | Head Step Direction
4            | 10               | 1,3    | Motor Enable A
5            | 14               | 1,3    | Drive Select A
6            | 32               | 3      | Side Select
7            | 8 	              | 2      | Index
8            | 30               | 2      | Read Data
9            | 22               |        | Write Data
10           | 24               |        | Write Gate (write enable)
11           | 26               | 2,3    | Track 0
12           | 28               | 3,4    | Write Protect signal
13           | 2                | 3,4,5  | Density select signal
A0           | 16               | 1,3,4  | Motor Enable B
A1           | 12               | 1,3,4  | Drive Select B
GND          | 1,3,5,...,31,33  |        | GND (just pick one)

**Note 1:**
The pin numbers for the SELECT/MOTOR signals assume you are wiring to
the controller end of a floppy drive cable. If you are wiring directly
to the floppy drive, the A/B pins will be reversed (search the web
for "Floppy drive twist" for more information).

**Note 2:**
It is **highly** recommended (but not entirely necessary) to add a 1k 
pull-up resistor to +5V to this signal. The Arduino's built-in pull-up
resistors are very weak and may not pull the signal up quickly enough. 
It worked for me without the resistors but results may vary for different
drives.

**Note 3:**
This signal can easily be moved to a different pin by re-defining the corresponding
`#define PIN_XXX ...` statement at the top of ArduinoFDC.cpp

**Note 4:**
This signal is not essential for the functionality of the controller.
The corresponding pin can be freed by commenting out the `#define PIN_XXX ...`
statement at the top of ArduinoFDC.cpp

**Note 5:**
See section "DENSITY control signal" below.

## Supported disk/drive types:

To properly read/write data, the library must be configured for the drive/disk
combination that is being used. The drive type can be passed into the `begin` functions
or set afterwards by calling the `setDriveType` function. Supported types are:
* **ArduinoFDC::DT_5_DD**: Double-density disk in a 5.25" double-density drive
* **ArduinoFDC::DT_5_DDonHD**: Double-density disk in a 5.25" high-density drive
* **ArduinoFDC::DT_5_HD**: High-density disk in a 5.25" high-density drive
* **ArduinoFDC::DT_3_DD**: Double-density disk in a 3.5" double- or high-density drive
* **ArduinoFDC::DT_3_HD**: High-density disk in a 3.5" high-density drive

## DENSITY control signal

The function of the DENSITY control signal line between the controller and the
floppy drive is not well defined and varies between drives. Furthermore most drives
can be configured by jumpers (also called "straps"). You may want to consult the
documentation for your drive for details.

In their default configuration most 3.5" drives do not use this signal. The drive
itself determines the type of disk (DD or HD) by the presence of a hole in the disk
(on the opposite edge from the "write protect" hole). The controller must be configured
separately for the correct type (DD or HD), otherwise reading/writing will fail.

For 5.25" drives this signal is generally an input from the controller to the drive.
If the signal is LOW then low density mode is selected, otherwise high density is used.
However, for some drives the opposite is true. 

The controller can be configured what to do with this signal by calling the "setDensityPinMode"
function. The following modes are supported:
* **ArduinoFDC::DP_DISCONNECT**: This configures the DENSITY pin as INPUT. It does not actually read the pin.
* **ArduinoFDC::DP_OUTPUT_LOW_FOR_HD**: This configures the DENSITY pin as an OUTPUT and sets it LOW if the disk type is HD.
* **ArduinoFDC::DP_OUTPUT_LOW_FOR_DD**: This configures the DENSITY pin as an OUTPUT and sets it LOW if the disk type is DD.

By default, the mode is set to DP_DISCONNECT for 3.5" disks and DP_OUTPUT_LOW_FOR_DD for 5.25" disks.

Another way to handle the DENSITY signal is to comment out the `#define PIN_DENSITY` line
at the top of ArduinoFDC.cpp and hard-wire the DENSITY signal from the disk drive cable
to the proper level.

## Library functions:

After adding ArduinoFDC.h and ArduinoFDC.cpp to your Arduino sketch you
can use the following functions:

#### `void ArduinoFDC.begin(driveAtype, driveBtype)`
Initializes the Arduino pins used by the controller. For possible drive types see
the "Supported disk/drive types" section above. If left out both types default to
ArduinoFDC::DT_3_HD.

#### `void ArduinoFDC.end()`
Releases the pins initialized by ArduinoFDC.begin()

#### `bool ArduinoFDC.selectDrive(byte drive)`
Selects drive A (0) or B (1) to be used for subsequent calls to 
readSector/writeSector/formatDisk. Calling `begin()` selects drive A.
Returns 'false' if trying to select drive 1 when the corresponding control
pins are commented out in ArduinoFDC.cpp

#### `void ArduinoFDC.setDriveType(driveType)`
Sets the disk/drive type for the currently selected drive. For possible drive types see
the "Supported disk/drive types" section above. 

#### `byte ArduinoFDC.getDriveType(driveType)`
Returns the drive type of the currently selected drive.

#### `void ArduinoFDC.setDensityPinMode(mode)`
Sets the function of the DENSITY pin for the currently selected drive.
See section "Density signal" above.

#### `byte ArduinoFDC.numTracks()`
Returns the number of tracks for the drive type of the currently selected drive.

#### `byte ArduinoFDC.numSectors()`
Returns the number of sectors per track for the drive type of the currently selected drive.


#### `byte ArduinoFDC.readSector(byte track, byte side, byte sector, byte *buffer)`
Reads data from a sector from the flopy disk. Always reads a full sector (512 bytes).

* The "track" parameter must be in range 0..(numTracks()-1)
* The "side" parameter must either be 0 or 1
* The "sector" paramter must be in range 1..numSectors()
* The "buffer" parameter must be a pointer to a byte array of size (at least) 516 bytes.

The function returns 0 if reading succeeded. Otherwise an error code is returned
(see Troubleshooting section below)

**IMPORTANT:** On successful return, the sector data that was read will be in buffer[1..512] (**NOT** buffer[0..511])

#### `byte ArduinoFDC.writeSector(byte track, byte side, byte sector, byte *buffer, bool verify)`
Writes data to a sector on the floppy disk. Always writes a full sector (512 bytes).

* The "track" parameter must be in range 0..(numTracks()-1)
* The "side" parameter must either be 0 or 1
* The "sector" paramter must be in range 1..numSectors()
* The "buffer" parameter must be a pointer to a byte array of size (at least) 516 bytes.
* If the "verify" parameter is *true*, the data written will be read back and compared to what was written.
If a difference is detected then the function will return *false*.
If the "verify" parameter is *false* then no verification is done. The function may still return *false*
if the proper sector location on disk can not be found before writing.

The function returns 0 if writing succeeded. Otherwise an error code is returned
(see Troubleshooting section below)

**IMPORTANT:** The sector data to be written must be in buffer[1..512] (**NOT** buffer[0..511])

#### `bool ArduinoFDC.formatDisk()`
Formats a floppy disk according to the format specified by the "setDriveType()" function.

This function does **not** set up any file system on the disk. It only sets up the 
low-level sector structure that allows reading and writing of sectors (and fills all
sector data with 0xF6 bytes). In order to make the disk readable by DOS, Windows
or other OSs a file system must be initialized. For DOS/Windows that means writing 
certain data structures to sectors on track 1 side 0. See the ArduinoFDC.ino 
example sketch for how to do so.

The function returns 0 if formatting succeeded. Otherwise an error code is returned
(see Troubleshooting section below). Note that no verification of the formatted disk 
is performed. The only possible error conditions are missing track 0 or index hole signals.
You can use the `readSector`function to verify that data can be read properly
after formatting.

#### `void ArduinoFDC.motorOn()`
Turns the disk drive motor on. 

The `readSector`/`writeSector`/`formatDisk` functions will turn the motor on **and** off 
automatically if it is not already running. Note that turning on the motor also includes
a one second delay to allow it to spin up.  If you are reading/writing multiple sectors
you may want to use the `motorOn` and `motorOff` functions to manually turn the motor on
and off.

#### `void ArduinoFDC.motorOff()`
Turns the disk drive motor off. 

#### `bool ArduinoFDC.motorRunning()`
Returns *true* if the disk drive motor is currently running and *false* if not.

## Testing

In addition to the `ArduinoFDC.h` and `ArduinoFDC.cpp` library files, a small sketch
`ArduinoFDC.ino` is provided to demonstrate and test the library. The sketch provides
a minimal user interface to interact with the library and read/write disks. It is easy
to use with the Arduino serial monitor. Set the monitor to 115200 baud to connect.
When connected, the sketch will show a command prompt. Enter your command in the
serial monitor's input line and press *Enter* to execute the command.

The following commands are supported:
* `r track, sector[,side]` <br/>
  Read the sector specified by track/sector/side, copy its contents to an internal 
  buffer and show the buffer content. If the *side* parameter is left out it defaults
  to zero.
* `w track, sector[,side]` <br/>
  Write the current buffer contents to the sector specified by track/sector/side
  and verify the data after writing. Shows "Ok" or "Error" status after execution.
  If the *side* parameter is left out it defaults to zero.
* `f [0/1]` <br/>
  Format the disk. If the *0/1* parameter is 1 then also initialize an empty DOS
  file system on the disk. The disk should be readable and writable on a DOS/Windows 
  computer after doing so. If the parameter is 0 then no filesystem is written. Sectors
  on the disk will be readable/writable but the disk will not be recognized by DOS/Windows.
  If the parameter is left out it defaults to 0.
* `b` <br/>
  Show the current buffer content
* `B [n]` <br/>
  Fill the buffer with value *n*. If *n* is left out then fill the buffer with
  bytes 0,1,2,...255,0,1,2,...255.
* `m [0/1]` <br/>
  Turn the motor of the currently selected drive off/on. If the *0/1* parameter is left out 
  then the current motor status is shown.
* `r` <br/>
  Read ALL sectors on the disk and show status Ok/Error for each one.
* `w [0/1]` <br/>
  Write the current buffer content to ALL sectors on the disk. If the *0/1* parameter
  is 1 then  verify every sector after writing it (significantly slower).
  If the *0/1* parameter is left out it defaults to 0.
* `s [0/1]` <br/>
  Select drive A (0) or B (1). If the *0/1* parameter is left out then the currently
  selected drive is shown.
* `t 0/1/2/3/4` <br/>
  Set the drive type of the current drive, where 0/1/2/3/4 stands for the drive type
  as listed (in the same order) in section "Supported disk/drive types" above.
* `c [0/1]` <br/>
  Copy content of drive A to drive B. If the *0/1* parameter is 1 then verify the
  written data (much slower). If the parameter is left out it defaults to 0.

## Troubleshooting

The following table lists the error codes returned by the `readSector`, `writeSector` 
and `formatDisk` functions including possible causes for each error, Pin numbers refer
to pins on the Arduino.

If you have trouble reading/writing a freshly formatted disk make sure that *both*
holes on the corners of the disk itself are *covered*. One is "write protect", the
other is the disk type (DD/HD). If the disk is write protected then some drives
won't write to it but the Arduino doesn't know because the "write protect" signal
is not read (I tried to not use too many pins). If the DD/HD hole is open then
the disk is identified HD and the drive quitely refuses to format it as DD.

\# | Code        | Meaning | Possible causes
--|-------------|---------|----------------
0 | S_OK        | No error, the operation succeeded | 
1 | S_NOTINIT   | The ArduinoFDC.begin() function has not been called |
2 | S_NOTREADY  | No data at all is received from the disk drive | - no disk in drive <br/> - drive does not have power <br/> - pins MOTOR (4/12), SELECT (5/13), READ (8) or GND not properly connected
3 | S_NOSYNC    | Data is received but no sync mark can be found | - disk not formatted or not formatted for the correct density <br/> - GND pin not properly connected
4 | S_NOHEADER  | Sync marks are found but either no sector header or no header with the expected track/side/sector markings | - pins STEP (2), STEPDIR (3), SIDE (6) or GND not properly connected <br/> - bad disk or unknown format <br/> - invalid track, sector or head number given
5 | S_INVALIDID | The data record was not started by a 0xFB byte as expected | - bad disk or unknown format
6 | S_CRC       | The sector data checksum is incorrect | - bad disk or unknown format <br/> - pullup resistors too weak (see note 2 in wiring section)
7 | S_NOINDEX   | No index hole was detected within 1 second with the motor running | - pins INDEX (7), MOTOR (4/12) or SELECT (5/13) not properly connected
8 | S_NOTRACK0  | When trying to move the read head to track 0, the TRACK0 signal was not seen, even after stepping more than 80 tracks. | - pins STEP (2), STEPDIR (3), SELECT (5/13) or TRACK0 (11) not properly connected
9 | S_VERIFY    | When reading back data that was just written, the data did not match | - pins WRITEGATE (10) or WRITEDATA (9) not properly connected<br/> - bad disk
10 | S_READONLY | Attempting to write to a write-protected disk | - disk is write protected
