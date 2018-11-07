# Pi-BNO055

## Background

This is a C driver program for operating a Bosch BNO055 IMU sensor via I2C on a Raspberry Pi. I used it with a GY-BNO055 and a Adafruit BNO055. On the GY-BNO055, I had to bridge two solder pads for enabling I2C mode, because serial mode was default.  Later I switched to Adafruit for the superior quality and the onboard 5V-level support.

<img src="ada-bno055.png" height="320px" width="273px">

## I2C bus connection

Connecting the GY-BNO055 sensor to the Raspberry Pi I2C bus, the sensor responds with the slave address 0x29. The Adafruit sensor responds by default under 0x28.

```
root@pi-ws01:/home/pi# i2cdetect -y 1
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- 29 -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --
```

## Code compilation

Compiling the test program:
````
root@pi-ws01:/home/pi/bno055# make
cc -O3 -Wall -g   -c -o i2c_bno055.o i2c_bno055.c
cc -O3 -Wall -g   -c -o getbno055.o getbno055.c
cc i2c_bno055.o getbno055.o -o getbno055
````

## Example output

Running the program, extracting the sensor verion and configuration information (verbose):
```
pi@nanopi-neo2:~/pi-bno055 $ ./getbno055 -t inf -v
Debug: ts=[1540717435] date=Sun Oct 28 18:03:55 2018
Debug: Sensor address: [0x28]
Debug: Sensor CHIP ID: [0xA0]
Debug: Sensor  ACC ID: [0xFB]
Debug: Sensor  MAG ID: [0x32]
Debug: Sensor  GYR ID: [0x0F]
Debug: SW  Rev-ID LSB: [0x11]
Debug: SW  Rev-ID MSB: [0x03]
Debug: Bootloader Ver: [0x15]
Debug: Operation Mode: [0x0B]
Debug:     Power Mode: [0x00] 2bit [0x00]
Debug: Axis Remap 'c': [0x24]
Debug: Axis Remap 's': [0x00]
Debug:  System Status: [0x05]
Debug: Self-Test Mode: [0x0F] 4bit [0x0F]
Debug: Internal Error: [0x00]
Debug: UnitDefinition: [0x80]
Debug:    Temperature: [0x1C] [28°C]

BN0055 Information at Sun Oct 28 18:03:55 2018
----------------------------------------------
   Chip Version ID = 0xA0
  Accelerometer ID = 0xFB
      Gyroscope ID = 0x0F
   Magnetoscope ID = 0x32
  Software Version = 3.17
   Operations Mode = NDOF_FMC_OFF
        Power Mode = NORMAL
Axis Configuration = X==X Y==Y Z==Z (ENU)
   Axis Remap Sign = X+ Y+ Z+
System Status Code = Sensor running with fusion algorithm
Accelerometer Test = OK
 Magnetometer Test = OK
    Gyroscope Test = OK
MCU Cortex M0 Test = OK
 System Error Code = No Error
MCU Cortex M0 Test = Accelerometer Unit = m/s2
    Gyroscope Unit = dps
        Euler Unit = Degrees
  Temperature Unit = Celsius
  Orientation Mode = Android
Sensor Temperature = 28°C

----------------------------------------------
Debug: sensor system calibration: [3]
Debug:     gyroscope calibration: [3]
Debug: accelerometer calibration: [0]
Debug:  magnetometer calibration: [3]
Sensor System Calibration = Fully calibrated
    Gyroscope Calibration = Fully calibrated
Accelerometer Calibration = Uncalibrated
 Magnetometer Calibration = Fully calibrated

```

Running the program, showing the sensor calibration state and offset values:
```
pi@nanopi-neo2:~/pi-bno055 $ ./getbno055 -t cal -v
Debug: ts=[1540717492] date=Sun Oct 28 18:04:52 2018
Debug: Sensor address: [0x28]
Debug: sensor system calibration: [3]
Debug:     gyroscope calibration: [3]
Debug: accelerometer calibration: [0]
Debug:  magnetometer calibration: [3]
Debug: I2C read 22 bytes starting at register 0x55
Debug: accelerometer data: X [0][0] Y [0][0] Z [0][0]
Debug: accelerometer offset: X [0] Y [0] Z [0]
Debug:  magnetometer offset, range +/-6400: X [0] Y [0] Z [0]
Debug: gyroscope offset: X [0] Y [0] Z [0]
Debug: accelerometer radius, range +/-1000: [0]
Debug:  magnetometer radius, range +/- 960: [480]

Calibration state: 3 acc [S:0 X:0 Y:0 Z:0 R:0] mag [S:3 X:0 Y:0 Z:0 R:480] gyr [S:3 X:0 Y:0 Z:0]
```

Changing the operational mode, e.g. to CONFIG:
```
pi@pi-ws01:~/pi-bno055 $ ./getbno055 -v -m config
Debug: arg -s, value config
Debug: ts=[1539005771] date=Mon Oct  8 22:36:11 2018
Debug: Sensor Address: [0x28]
Debug: Write opr_mode: [0x00] to register [0x3D]
```

Resetting the sensor:
```
pi@pi-ws01:~/pi-bno055 $ ./getbno055 -v -r
Debug: arg -r, value (null)
Debug: ts=[1539005864] date=Mon Oct  8 22:37:44 2018
Debug: Sensor Address: [0x28]
Debug: BNO055 Sensor Reset complete
```

NDOF fusion mode, Euler angles
```
pi@nanopi-neo2:~/pi-bno055 $ ./getbno055 -t eul -v
Debug: ts=[1540717576] date=Sun Oct 28 18:06:16 2018
Debug: Sensor address: [0x28]
Debug: Operation Mode: [0x0B]
Debug: I2C read 6 bytes starting at register 0x1A
Debug: Euler Orientation H:[0xF9][0x02] P:[0x78][0xFF] R:[0x75][0xFF]
Debug: bnod.eul_head [761]
Debug: bnod.eul_roll [65400]
Debug: bnod.eul_pitc [65397]
EUL-H: 0.12 EUL-R: -3.31 EUL-P: -15.31
```

Writing calibration data to file
```
pi@nanopi-neo2:~/pi-bno055 $ ./getbno055 -t cal -w bno.cfg
Calibration state: 3 acc [S:1 X:1 Y:65532 Z:65522 R:1000] mag [S:3 X:65484 Y:65496 Z:65476 R:584] gyr [S:3 X:65535 Y:65535 Z:1]
```

Reading the saved calibration data file content
```
pi@nanopi-neo2:~/pi-bno055 $ od -A x -t x1z -v bno.cfg
000000 01 00 fc ff f2 ff cc ff d8 ff c4 ff ff ff ff ff  >................<
000010 01 00 e8 03 48 02                                >....H.<
000016
```
## Usage

Program usage:
```
pi@nanopi-neo2:~/pi-bno055 $ ./getbno055
Usage: getbno055 [-a hex i2cr-addr] [-m <opr_mode>] [-t acc|gyr|mag|eul|qua|lin|gra|inf|cal] [-r] [-w calfile] [-l calfile] [-o htmlfile] [-v]

Command line parameters have the following format:
   -a   sensor I2C bus address in hex, Example: -a 0x28 (default)
   -m   set sensor operational mode. mode arguments:
           config   = configuration mode
           acconly  = accelerometer only
           magonly  = magnetometer only
           gyronly  = gyroscope only
           accmag   = accelerometer + magnetometer
           accgyro  = accelerometer + gyroscope
           maggyro  = magetometer + gyroscope
           amg      = accelerometer + magnetoscope + gyro
           imu      = accelerometer + gyro + rel. orientation
           compass  = accelerometer + magnetometer + abs. orientation
           m4g      = accelerometer + magnetometer + rel. orientation
           ndof     = accel + magnetometer + gyro + abs. orientation
           ndof_fmc = ndof with fast magnetometer calibration (FMC)
   -r   reset sensor
   -t   read and output sensor data. data type arguments:
           acc = Accelerometer (3 values for X-Y-Z axis)
           gyr = Gyroscope (3 values for X-Y-X axis)
           mag = Magnetometer (3 values for X-Y-Z axis)
           eul = Orientation E (3 values for H-R-P as Euler angles)
           qua = Orientation Q (4 values for W-X-Y-Z as Quaternation)
           lin = Linear Accel (3 values for X-Y-Z axis)
           gra = GravityVector (3 values for X-Y-Z axis)
           inf = Sensor info (7 values version and state)
           cal = Calibration data (9 values for each X-Y-Z)
   -l   load sensor calibration data from file, Example -l ./bno055.cal
   -w   write sensor calibration data to file, Example -w ./bno055.cal
   -o   output sensor data to HTML table file, requires -t, Example: -o ./getsensor.html
   -h   display this message
   -v   enable debug output

Note: The sensor is executing calibration in the background, but only in fusion mode.

Usage examples:
./getbno055 -a 0x28 -t inf -v
./getbno055 -t cal -v
./getbno055 -t mag -o ./bno055.html -v
./getbno055 -s ndof -v
./getbno055 -w ./bno055.cal -v
```
