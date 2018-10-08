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
pi@pi-ws01:~/pi-bno055 $ ./getbno055 -v -t inf
root@nanopi-neo2:/home/pi/pi-bno055# ./getbno055 -v -t inf
Debug: arg -t, value inf
Debug: ts=[1539005349] date=Mon Oct  8 22:29:09 2018
Debug: Sensor Address: [0x28]
Debug: Sensor CHIP ID: [0xA0]
Debug: Sensor  ACC ID: [0xFB]
Debug: Sensor  MAG ID: [0x32]
Debug: Sensor  GYR ID: [0x0F]
Debug: SW  Rev-ID LSB: [0x11]
Debug: SW  Rev-ID MSB: [0x03]
Debug: Bootloader Ver: [0x15]
Debug: Operation Mode: [0x0B] 4bit [0x0B]
Debug:  System Status: [0x05]
Debug: Self-Test Mode: [0x0F] 4bit [0x0F]
Debug: Internal Error: [0x00]
Debug: UnitDefinition: [0x80]
Debug:    Temperature: [0x1F] [31°C]

BN0055 Information at Mon Oct  8 22:29:09 2018
----------------------------------------------
   Chip Version ID = 0xA0
  Accelerometer ID = 0xFB
      Gyroscope ID = 0x0F
   Magnetoscope ID = 0x32
   Operations Mode = NDOF_FMC_OFF
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
Sensor Temperature = 31°C
```

Running the program, showing the sensor calibration state and offset values:
```
pi@pi-ws01:~/pi-bno055 $ ./getbno055 -v -t cal
Debug: arg -t, value cal
Debug: ts=[1539005563] date=Mon Oct  8 22:32:43 2018
Debug: Sensor Address: [0x28]
Debug: system calibration: 3
Debug: gyroscope calibration: 3
Debug: accelerometer calibration: 1
Debug: magnetometer calibration: 3

BN0055 Calibration at Mon Oct  8 22:32:43 2018
----------------------------------------------
Sensor System Calibration State = Fully calibrated
    Gyroscope Calibration State = Fully calibrated
Accelerometer Calibration State = Minimal Calibrated
 Magnetometer Calibration State = Fully calibrated
Debug: accelerometer offset: X [0] Y [0] Z [2]
Debug: magnetometer offset: X [65448] Y [65460] Z [28]
Debug: gyroscope offset: X [65535] Y [65535] Z [1]
----------------------------------------------
Accelerometer Calibration Offset = X:    0 Y:    0 Z:    2
 Magnetometer Calibration Offset = X:65448 Y:65460 Z:   28
    Gyroscope Calibration Offset = X:65535 Y:65535 Z:    1
```

Changing the operational mode, e.g. to CONFIG:
```
pi@pi-ws01:~/pi-bno055 $ ./getbno055 -v -s config
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

## Usage

Program usage:
```
Usage: getbno055 [-a hex i2c addr] -t acc|gyr|mag|inf|cal [-r] [-s <opr_mode>] [-w] [-o html-output] [-v]

Command line parameters have the following format:
   -a   sensor I2C bus address in hex, Example: -a 0x28 (default)
   -t   read data from: Accelerometer = acc (3 values for X-Y-Z axis)
                        Gyroscope     = gyr (3 values for X-Y-X axis)
                        Magnetometer  = mag (3 values for X-Y-Z axis)
                        Sensor Info   = inf (7 values version and state
                        Calibration   = cal (9 values for each X-Y-Z)
   -r   optional, reset sensor
   -s   set sensor operational mode. mode arguments:
                        config = configuration mode
                        acconly = accelerometer only
                        magonly = magnetometer only
                        gyronly = gyroscope only
                        accmag = accelerometer + mangetometer
                        accgyro = accelerometer + gyroscope
                        maggyro = magetometer + gyroscope
                        amg = accelerometer + magnetoscope + gyro
                        imu = accelerometer + gyro + rel. orientation
                        compass = accelerometer + magnetometer + abs. orientation
                        m4g = accelerometer + magnetometer + rel. orientation
                        ndof = accel + magnetometer + gyro + abs. orientation
                        ndof_fmc = ndof with fast magnetometer calibration (FMC)
   -w   optional, write sensor calibration data to file, Example -c ./bno055.cal
   -o   optional, write sensor data to HTML file, Example: -o ./getsensor.html
   -h   optional, display this message
   -v   optional, enables debug output

Usage examples:
./getbno055 -a 0x28 -t inf -v
./getbno055 -t cal -v
./getbno055 -t mag -o ./bno055.html -v
./getbno055 -s ndof -v
./getbno055 -w ./bno055.cal -v
```
