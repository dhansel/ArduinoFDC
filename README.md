# ArduinoFDC

This is a small library providing low-level functions which enable an
Arduino UNO (or any ATMega328P) to control a 3.5" floppy disk drive.
It supports reading and writing disks at the sector level as well as 
formatting disks.

Note that this controller only works as a double density (DD) controller,
reading/writing up to 720KB per disk. It does not work with disks formatted
as HD (1.44MB). It can however format a HD disk with DD format and then
write to and read from it.

## Wiring:

The following table shows how to wire up the Arduino UNO pins to the 34-pin IDC
connector on the 3.5" floppy drive. 

The pin numbers are defined at the top of the ArduinoFDC.cpp file. Some of them can 
easily be changed whereas others are hard-coded in the controller code. Refer to 
the comments at the top of the ArduinoFDC.cpp file if you want to use different
pin assignments.

Arduino pin  | Floppy Drive pin | Function
-------------|------------------|-------------------------
2            | 20               | Head Step Pulse
3            | 18               | Head Step Direction
4            | 10/16            | Motor Enable (see note 1 below)
5            | 14/12            | Drive Select (see note 1 below)
6            | 32               | Side Select
7            | 8 	              | Index (see note 2 below)
8            | 30               | Read Data (see note 2 below)
9            | 22               | Write Data
10           | 24               | Write Enable
11           | 26               | Track 0 (see note 2 below)
GND          | 1,3,5,...,31,33  | GND (just pick one)

**Note 1:**
If you are wiring directly to the male connector on the floppy drive then
use pins 16 and 12. If you have a floppy cable connected to the drive then
use pins 10 and 14 if the drive is plugged into the cable in the "Drive A"
position and pins 16 and 12 if the drive is plugged into the "Drive B" position.
For more information on why this is necessary, search the web for "Floppy
cable twist".

**Note 2:**
It is **highly** recommended (but not entirely necessary) to add a 1k 
pull-up resistor to +5V to this signal. The Arduino's built-in pull-up
resistors are very weak and may not pull the signal up quickly enough. 
It worked for me without the resistors but results may vary for different
drives.

## Library functions:

After adding ArduinoFDC.h and ArduinoFDC.cpp to your Arduino sketch you
can use the following functions:

#### `void ArduinoFDC.begin()`
Initializes the Arduino pins used by the controller. 

#### `void ArduinoFDC.end()`
Releases the pins initialized by ArduinoFDC.begin()

#### `bool ArduinoFDC.readSector(byte track, byte side, byte sector, byte *buffer)`
Reads data from a sector from the flopy disk. Always reads a full sector (512 bytes).

* The "track" parameter must be in range 0..79
* The "side" parameter must either be 0 or 1
* The "sector" paramter must be in range 1..9
* The "buffer" parameter must be a pointer to a byte array of size (at least) 515 bytes.

The function returns *true* if reading succeeded and *false* if it failed.

**IMPORTANT:** On successful return, the sector data that was read will be in buffer[1..512] (**NOT** buffer[0..511])

#### `bool ArduinoFDC.writeSector(byte track, byte side, byte sector, byte *buffer, bool verify)`
Writes data to a sector on the floppy disk. Always writes a full sector (512 bytes).

* The "track" parameter must be in range 0..79
* The "side" parameter must either be 0 or 1
* The "sector" paramter must be in range 1..9
* The "buffer" parameter must be a pointer to a byte array of size (at least) 515 bytes.
* If the "verify" parameter is *true*, the data written will be read back and compared to what was written.
If a difference is detected then the function will return *false*.
If the "verify" parameter is *false* then no verification is done. The function may still return *false*
if the proper sector location on disk can not be found before writing.

The function returns *true* if writing succeeded and *false* if it failed.

**IMPORTANT:** The sector data to be written must be in buffer[1..512] (**NOT** buffer[0..511])

#### `bool ArduinoFDC.formatDisk()`
Formats a floppy disk with a low-level double density (DD) 720K format.

This function does **not** set up any file system on the disk. It only sets up the 
low-level sector structure that allows reading and writing of sectors (and fills all
sector data with 0xF6 bytes). In order to make the disk readable by DOS, Windows
or other OSs a file system must be initialized. For DOS/Windows that means writing 
certain data structures to sectors 1-9 on track 1 side 0. See the ArduinoFDC.ino 
example sketch for how to do so.

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
  computer after doing so. If the *0/1* parameter is left out it defaults to 0.
* `b` <br/>
  Show the current buffer content
* `B [n]` <br/>
  Fill the buffer with value *n*. If *n* is left out then fill the buffer with
  bytes 0,1,2,...255,0,1,2,...255.
* `m [0/1]` <br/>
  Turn the drive motor on/off. If the *0/1* argument is left out it defaults to 0.
* `r` <br/>
  Read ALL sectors on the disk and show status Ok/Error for each one.
* `w [0/1]` <br/>
  Write the current buffer content to ALL sectors on the disk. If the *0/1* parameter
  is 1 then  verify every sector after writing it (significantly slower).
  If the *0/1* parameter is left out it defaults to 0.

## Troubleshooting

By default the `readSector`, `writeSector` and `formatDisk` functions just return *false*
when something is not right. To get some more information about the error you can
un-comment the `#define DEBUG` line in the ArduinoFDC.cpp file. With that enabled
the code will write error messages to the serial port.

The following table shows possible causes for each error message. Pin numbers refer
to pins on the Arduino.

Message | Meaning | Possible causes
--------|---------|----------------
Not initialized | The ArduinoFDC.begin() function has not been called |
Drive not ready | No data at all is received from the disk drive | - no disk in drive <br/> - pins MOTOR (4), SELECT (5) or READ (8) not properly connected
No sync | Data is received but no sync mark can be found | - disk not formatted or not formatted as double density (DD)
Header CRC error | The sector header checksum is incorrect | - bad disk or unknown format
Unable to find header |  Sync marks are found but either no sector header or no header with the expected track/side/sector markings | - pins STEP (2), STEPDIR (3) or SIDE (6) not properly connected <br/> - bad disk or unknown format
Unexpected record identifier | The data record was not started by a 0xFB byte as expected | - bad disk or unknown format
Data CRC error | The sector data checksum is incorrect | - bad disk or unknown format
Verify after write failed | When reading back data that was just written, the data did not match | - pins WRITEGATE (10) or WRITEDATA (9) not properly connected<br/> - bad disk
No index hole signal | No index hole was detected with 1 second while the motor was on | - pins INDEX (7), MOTOR (4) or SELECT (5) not properly connected
No track0 signal | When trying to reset the read head to track 0, the TRACK0 signal was not seen, even after stepping more than 80 tracks. | - pins STEP (2), STEPDIR (3), SELECT (5) or TRACK0 (11) not properly connected
