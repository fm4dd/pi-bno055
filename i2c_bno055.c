/* ------------------------------------------------------------ *
 * file:        i2c_bno055.c                                    *
 * purpose:     Extract sensor data from Bosch BNO055 modules.  *
 *              Connects through I2C bus and writes acc, gyro   *
 *              mag and sensor info data to global variables.   *
 *                                                              *
 * Parameters:  i2caddr is a string containing the hex address  *
 *              of the Bosch BNO055 sensor, connected to the    *
 *              I2C bus. Values are 0x28 or 0x29 per COM3 pin.  *
 *                                                              *
 *		verbose - enable extra debug output if needed.  *
 *                                                              *
 * Return Code:	Returns 0 on success, and -1 on error.          *
 *                                                              *
 * Requires:	I2C development packages                        *
 *                                                              *
 * author:      07/14/2018 Frank4DD                             *
 *                                                              *
 * compile: gcc i2c_bno055.c i2c_bno055.o                       *
 * ------------------------------------------------------------ */
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include "getbno055.h"

int read_inf(char *i2caddr, struct bnover *bno_ptr,  int verbose) {
/* ------------------------------------------------------------ *
 * Get the I2C bus. Raspberry Pi 2 uses i2c-1, RPI 1 used i2c-0
 * ------------------------------------------------------------ */
   int file;
   char *bus = "/dev/i2c-1";
   if((file = open(bus, O_RDWR)) < 0) {
      printf("Error failed to open I2C bus [%s].\n", bus);
      return(-1);
   }
/* ------------------------------------------------------------ *
 * Set I2C device (BNO055 I2C address is either 0x28 or 0x29)
 * ------------------------------------------------------------ */
   int addr = (int)strtol(i2caddr, NULL, 16);
   if(verbose == 1) printf("Debug: Sensor I2C address: [0x%02X]\n", addr);

   ioctl(file, I2C_SLAVE, addr);

   char reg = 0x00;
   char data = 0;
/* ------------------------------------------------------------ *
 * Read 1-byte chip ID from register 0x00 (default: 0xA0)
 * ------------------------------------------------------------ */
   write(file, &reg, 1);
   if(read(file, &data, 1) != 1) {
      printf("Error: Input/Output error while reading from sensor\n");
      return(-1);
   }
   bno_ptr->chip_id = data;
   if(verbose == 1) printf("Debug: Sensor CHIPID: [0x%02X]\n", bno_ptr->chip_id);

/* ------------------------------------------------------------ *
 * Read 1-byte Accelerometer ID from register 0x01 (default: 0xFB)
 * ------------------------------------------------------------ */
   reg = 0x01;
   data = 0;
   write(file, &reg, 1);
   if(read(file, &data, 1) != 1) {
      printf("Error: Input/Output error while reading from sensor\n");
      return(-1);
   }
   bno_ptr->acc_id = data;
   if(verbose == 1) printf("Debug: Sensor ACC ID: [0x%02X]\n", bno_ptr->acc_id);

/* ------------------------------------------------------------ *
 * Read 1-byte Magnetometer ID from register 0x02 (default 0x32)
 * ------------------------------------------------------------ */
   reg = 0x02;
   data = 0;
   write(file, &reg, 1);
   if(read(file, &data, 1) != 1) {
      printf("Error: Input/Output error while reading from sensor\n");
      return(-1);
   }
   bno_ptr->mag_id = data;
   if(verbose == 1) printf("Debug: Sensor MAG ID: [0x%02X]\n", bno_ptr->mag_id);

/* ------------------------------------------------------------ *
 * Read 1-byte Gyroscope ID from register 0x03 (default: 0x0F)
 * ------------------------------------------------------------ */
   reg = 0x03;
   data = 0;
   write(file, &reg, 1);
   if(read(file, &data, 1) != 1) {
      printf("Error: Input/Output error while reading from sensor\n");
      return(-1);
   }
   bno_ptr->gyr_id = data;
   if(verbose == 1) printf("Debug: Sensor GYR ID: [0x%02X]\n", bno_ptr->gyr_id);

/* ------------------------------------------------------------ *
 * Read 1-byte SW Rev ID LSB from register 0x04 (default: 0x08)
 * ------------------------------------------------------------ */
   reg = 0x04;
   data = 0;
   write(file, &reg, 1);
   if(read(file, &data, 1) != 1) {
      printf("Error: Input/Output error while reading from sensor\n");
      return(-1);
   }
   bno_ptr->sw_lsb = data;
   if(verbose == 1) printf("Debug: SW Rev ID LSB: [0x%02X]\n", bno_ptr->sw_lsb);

/* ------------------------------------------------------------ *
 * Read 1-byte SW Rev ID MSB from register 0x05 (default: 0x03)
 * ------------------------------------------------------------ */
   reg = 0x05;
   data = 0;
   write(file, &reg, 1);
   if(read(file, &data, 1) != 1) {
      printf("Error: Input/Output error while reading from sensor\n");
      return(-1);
   }
   bno_ptr->sw_msb = data;
   if(verbose == 1) printf("Debug: SW Rev ID MSB: [0x%02X]\n", bno_ptr->sw_msb);

/* ------------------------------------------------------------ *
 * Read 1-byte BL Rev ID from register 0x06 (no default)
 * ------------------------------------------------------------ */
   reg = 0x06;
   data = 0;
   write(file, &reg, 1);
   if(read(file, &data, 1) != 1) {
      printf("Error: Input/Output error while reading from sensor\n");
      return(-1);
   }
   bno_ptr->bl_rev = data;
   if(verbose == 1) printf("Debug: BL Rev ID: [0x%02X]\n", bno_ptr->bl_rev);

   return(0);
}
int read_mag(char *i2caddr, float *magx_ptr, float *magy_ptr, float *magz_ptr,  int verbose) {
   return(0);
}
