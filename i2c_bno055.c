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

int get_i2cbus(char *i2caddr, int verbose) {
/* ------------------------------------------------------------ *
 * Get the I2C bus. Raspberry Pi 2 uses i2c-1, RPI 1 used i2c-0 *
 * NanoPi also uses i2c-0.                                      *
 * ------------------------------------------------------------ */
   if((i2cfd = open(I2CBUS, O_RDWR)) < 0) {
      printf("Error failed to open I2C bus [%s].\n", I2CBUS);
      return(-1);
   }
/* ------------------------------------------------------------ *
 * Set I2C device (BNO055 I2C address is either 0x28 or 0x29)   *
 * ------------------------------------------------------------ */
   int addr = (int)strtol(i2caddr, NULL, 16);
   if(verbose == 1) printf("Debug: Sensor Address: [0x%02X]\n", addr);

   if(ioctl(i2cfd, I2C_SLAVE, addr) == 0) return(0);
   else return(-1);
}

int bno_reset(int verbose) {
   char data[2];
   data[0] = BNO055_SYS_TRIGGER_ADDR;
   data[1] = 0x20;
   if(write(i2cfd, data, 2) != 2) {
      printf("Error: I2C write failure for register 0x%02X\n", data[0]);
      return(-1);
   }
   if(verbose == 1) printf("Debug: BNO055 Sensor Reset complete\n");
   usleep(50 * 1000);
   return(0);
}

int set_defaults(int verbose) {
/* ------------------------------------------------------------ *
 * Test i2c read to ensure sensor is connected and working      *
 * ------------------------------------------------------------ */
   char reg = BNO055_CHIP_ID_ADDR;
   if(write(i2cfd, &reg, 1) != 1) {
      printf("Error: I2C write failure for register 0x%02X\n", reg);
      return(-1);
   }

   char data[2] = {0};
   if(read(i2cfd, data, 1) != 1) {
      printf("Error: I2C read failure for register 0x%02X\n", reg);
      return(-1);
   }
/* ------------------------------------------------------------ *
 * If there was no result, wait a second for boot and try again *
 * ------------------------------------------------------------ */
   if(data[0] != BNO055_ID) {
      printf("Error: I2C data is [0x%02X], expected [0x%02X]. 2nd Try.\n", data[0], BNO055_ID);
      sleep(1);
   }
   if(write(i2cfd, &reg, 1) != 1) {
      printf("Error: I2C write failure for register 0x%02X\n", reg);
      return(-1);
   }
   if(read(i2cfd, data, 1) != 1) {
      printf("Error: I2C read failure for register 0x%02X\n", reg);
      return(-1);
   }
/* ------------------------------------------------------------ *
 * If it failed again, we have a hard error and must quit here  *
 * ------------------------------------------------------------ */
   if(data[0] != BNO055_ID) {
      printf("Error: 2nd I2C data is [0x%02X], expected [0x%02X]. Terminating.\n", data[0], BNO055_ID);
      exit(-1);
   }

/* ------------------------------------------------------------ *
 * I2C is OK. We set the operations mode next.                  *
 * ------------------------------------------------------------ */
   if(set_mode(ndof, verbose) != 0) {
      exit(-1);
   }

/* ------------------------------------------------------------ *
 * Finally set the power mode to normal.                        *
 * ------------------------------------------------------------ */
   data[0] = BNO055_PWR_MODE_ADDR;
   data[1] = POWER_MODE_NORMAL;
   if(verbose == 1) printf("Debug: Write pwr_mode: [0x%02X] to register [0x%02X]\n", data[1], data[0]);
   if(write(i2cfd, data, 2) != 2) {
      printf("Error: I2C write failure for register 0x%02X\n", data[0]);
      return(-1);
   }
   usleep(10 * 1000);

   data[0] = BNO055_PAGE_ID_ADDR;
   data[1] = 0;
   if(verbose == 1) printf("Debug: Write  page_id: [0x%02X] to register [0x%02X]\n", data[1], data[0]);
   if(write(i2cfd, data, 2) != 2) {
      printf("Error: I2C write failure for register 0x%02X\n", data[0]);
      return(-1);
   }
   return(0);
}

/* ------------------------------------------------------------ *
 * stat_cal() gets current calibration state from the sensor.   *
 * Calibration status has 4 values, encoded as 2bit in reg 0x35 *
 * ------------------------------------------------------------ */
int stat_cal(struct bnocal *bno_ptr,  int verbose) {
   char reg = BNO055_CALIB_STAT_ADDR;
   if(write(i2cfd, &reg, 1) != 1) {
      printf("Error: I2C write failure for register 0x%02X\n", reg);
      return(-1);
   }

   char data = 0;
   if(read(i2cfd, &data, 1) != 1) {
      printf("Error: I2C read failure for register 0x%02X\n", reg);
      return(-1);
   }

   bno_ptr->scal_st = (data & 0b11000000) >> 6; // system calibration status
   if(verbose == 1) printf("Debug: system calibration: %d\n", bno_ptr->scal_st);
   bno_ptr->gcal_st = (data & 0b00110000) >> 4; // gyro calibration
   if(verbose == 1) printf("Debug: gyroscope calibration: %d\n", bno_ptr->gcal_st);
   bno_ptr->acal_st = (data & 0b00001100) >> 2; // accel calibration status
   if(verbose == 1) printf("Debug: accelerometer calibration: %d\n", bno_ptr->acal_st);
   bno_ptr->mcal_st = (data & 0b00000011);      // magneto calibration status
   if(verbose == 1) printf("Debug: magnetometer calibration: %d\n", bno_ptr->mcal_st);
   return(0);
}

int read_cal(struct bnocal *bno_ptr,  int verbose) {
/* ------------------------------------------------------------ *
 * Calibration data is stored in 3x6 (18) registers 0x55 ~ 0x66 *
 * ------------------------------------------------------------ */
   char reg = ACCEL_OFFSET_X_LSB_ADDR;
   if(write(i2cfd, &reg, 1) != 1) {
      printf("Error: I2C write failure for register 0x%02X\n", reg);
      return(-1);
   }

   char data[18] = {0};
   if(read(i2cfd, data, 18) != 18) {
      printf("Error: I2C calibration data read from 0x%02X\n", reg);
      return(-1);
   }
   // assigning accelerometer X-Y-Z offset
   bno_ptr->aoff_x = (data[0] + data[1] * 256);
   bno_ptr->aoff_y = (data[2] + data[3] * 256);
   bno_ptr->aoff_z = (data[4] + data[5] * 256);
   if(verbose == 1) printf("Debug: accelerometer offset: X [%d] Y [%d] Z [%d]\n",
		           bno_ptr->aoff_x, bno_ptr->aoff_y, bno_ptr->aoff_z);

   // assigning magnetometer X-Y-Z offset
   bno_ptr->moff_x = (data[6] + data[7] * 256);
   bno_ptr->moff_y = (data[8] + data[9] * 256);
   bno_ptr->moff_z = (data[10] + data[11] * 256);
   if(verbose == 1) printf("Debug: magnetometer offset: X [%d] Y [%d] Z [%d]\n",
		           bno_ptr->moff_x, bno_ptr->moff_y, bno_ptr->moff_z);

   // assigning gyroscope X-Y-Z offset
   bno_ptr->goff_x = (data[12] + data[13] * 256);
   bno_ptr->goff_y = (data[14] + data[15] * 256);
   bno_ptr->goff_z = (data[16] + data[17] * 256);
   if(verbose == 1) printf("Debug: gyroscope offset: X [%d] Y [%d] Z [%d]\n",
		           bno_ptr->goff_x, bno_ptr->goff_y, bno_ptr->goff_z);
   return(0);
}

/* ------------------------------------------------------------ *
 * Read 1-byte chip ID from register 0x00 (default: 0xA0)
 * ------------------------------------------------------------ */
int decode_units(int unit_sel) {
   // bit-0
   printf("Acceleration Unit: ");
   if((unit_sel >> 0) & 0x01) printf("mg\n");
   else printf("m/s2\n");

   // bit-1
   printf("Gyroscope Unit: ");
   if((unit_sel >> 1) & 0x01) printf("rps\n");
   else printf("dps\n");

   // bit-2
   printf("Euler Unit: ");
   if((unit_sel >> 2) & 0x01) printf("Radians\n");
   else printf("Degrees\n");

   // bit-3: unused

   // bit-4
   printf("Temperature Unit: ");
   if((unit_sel >> 4) & 0x01) printf("Fahrenheit\n");
   else printf("Celsius\n");

   // bit-5: unused
   // bit-6: unused

   // bit-7
   printf("Orientation Mode: ");
   if((unit_sel >> 3) & 0x01) printf("Android\n");
   else printf("Windows\n");

   return(0);
}

/* ------------------------------------------------------------ *
 * read_inf() queries the BNO055 and write the info data into
 * the global struct bnoinf defined in getbno055.h
 * ------------------------------------------------------------ */
int read_inf(struct bnoinf *bno_ptr,  int verbose) {
   char reg = 0x00;
   if(write(i2cfd, &reg, 1) != 1) {
      printf("Error: I2C write failure for register 0x%02X\n", reg);
      return(-1);
   }

   char data[7] = {0};
   if(read(i2cfd, data, 7) != 7) {
      printf("Error: I2C read failure for register data 0x00-0x06\n");
      return(-1);
   }
/* ------------------------------------------------------------ *
 * Read 1-byte chip ID from register 0x00 (default: 0xA0)
 * ------------------------------------------------------------ */
   if(verbose == 1) printf("Debug: Sensor CHIP ID: [0x%02X]\n", data[0]);
   bno_ptr->chip_id = data[0];

/* ------------------------------------------------------------ *
 * Read 1-byte Accelerometer ID from register 0x01 (default: 0xFB)
 * ------------------------------------------------------------ */
   if(verbose == 1) printf("Debug: Sensor  ACC ID: [0x%02X]\n", data[1]);
   bno_ptr->acc_id = data[1];

/* ------------------------------------------------------------ *
 * Read 1-byte Magnetometer ID from register 0x02 (default 0x32)
 * ------------------------------------------------------------ */
   if(verbose == 1) printf("Debug: Sensor  MAG ID: [0x%02X]\n", data[2]);
   bno_ptr->mag_id = data[2];

/* ------------------------------------------------------------ *
 * Read 1-byte Gyroscope ID from register 0x03 (default: 0x0F)
 * ------------------------------------------------------------ */
   if(verbose == 1) printf("Debug: Sensor  GYR ID: [0x%02X]\n", data[3]);
   bno_ptr->gyr_id = data[3];

/* ------------------------------------------------------------ *
 * Read 1-byte SW Rev ID LSB from register 0x04 (default: 0x08)
 * ------------------------------------------------------------ */
   if(verbose == 1) printf("Debug: SW  Rev-ID LSB: [0x%02X]\n", data[4]);
   bno_ptr->sw_lsb = data[4];

/* ------------------------------------------------------------ *
 * Read 1-byte SW Rev ID MSB from register 0x05 (default: 0x03)
 * ------------------------------------------------------------ */
   if(verbose == 1) printf("Debug: SW  Rev-ID MSB: [0x%02X]\n", data[5]);
   bno_ptr->sw_msb = data[5];

/* ------------------------------------------------------------ *
 * Read 1-byte BL Rev ID from register 0x06 (no default)
 * ------------------------------------------------------------ */
   if(verbose == 1) printf("Debug: Bootloader Ver: [0x%02X]\n", data[6]);
   bno_ptr->bl_rev = data[6];

/* ------------------------------------------------------------ *
 * Read 1-byte from Operations Mode from register 0x3d, and use *
 * only the lowest 4 bits. Bits 4-7 unused and get stripped off *
 * ------------------------------------------------------------ */
   reg = BNO055_OPR_MODE_ADDR;
   if(write(i2cfd, &reg, 1) != 1) {
      printf("Error: I2C write failure for register 0x%02X\n", reg);
      return(-1);
   }

   data[0] = 0;
   if(read(i2cfd, data, 1) != 1) {
      printf("Error: I2C read failure for register 0x%02X\n", reg);
      return(-1);
   }
   if(verbose == 1) printf("Debug: Operation Mode: [0x%02X] 4bit [0x%02X]\n", data[0], data[0] & 0x0F);
   bno_ptr->opr_mode = data[0] & 0x0F; // only get the lowest 4 bits

/* ------------------------------------------------------------ *
 * Read 1-byte system status from register 0x39 (no default)
 * ------------------------------------------------------------ */
   reg = BNO055_SYS_STAT_ADDR;
   if(write(i2cfd, &reg, 1) != 1) {
      printf("Error: I2C write failure for register 0x%02X\n", reg);
      return(-1);
   }

   data[0] = 0;
   if(read(i2cfd, data, 1) != 1) {
      printf("Error: I2C read failure for register data 0x%02X\n", reg);
      return(-1);
   }
   if(verbose == 1) printf("Debug:  System Status: [0x%02X]\n", data[0]);
   bno_ptr->sys_stat = data[0];

/* ------------------------------------------------------------ *
 * Read 1-byte Self Test Result from register 0x36 (0x0F=pass)
 * ------------------------------------------------------------ */
   reg = BNO055_SELFTSTRES_ADDR;
   if(write(i2cfd, &reg, 1) != 1) {
      printf("Error: I2C write failure for register 0x%02X\n", reg);
      return(-1);
   }

   data[0] = 0;
   if(read(i2cfd, data, 1) != 1) {
      printf("Error: I2C read failure for register data 0x%02X\n", reg);
      return(-1);
   }
   if(verbose == 1) printf("Debug: Self-Test Mode: [0x%02X] 4bit [0x%02X]\n", data[0], data[0] & 0x0F);
   bno_ptr->selftest = data[0] & 0x0F; // only get the lowest 4 bits

/* ------------------------------------------------------------ *
 * Read 1-byte System Error from register 0x3A (0=OK)
 * ------------------------------------------------------------ */
   reg = BNO055_SYS_ERR_ADDR;
   if(write(i2cfd, &reg, 1) != 1) {
      printf("Error: I2C write failure for register 0x%02X\n", reg);
      return(-1);
   }

   data[0] = 0;
   if(read(i2cfd, data, 1) != 1) {
      printf("Error: I2C read failure for register data 0x%02X\n", reg);
      return(-1);
   }
   if(verbose == 1) printf("Debug: Internal Error: [0x%02X]\n", data[0]);
   bno_ptr->sys_err = data[0];

/* ------------------------------------------------------------ *
 * Read 1-byte Unit definition from register 0x3B (0=OK)
 * ------------------------------------------------------------ */
   reg = BNO055_UNIT_SEL_ADDR;
   if(write(i2cfd, &reg, 1) != 1) {
      printf("Error: I2C write failure for register 0x%02X\n", reg);
      return(-1);
   }

   data[0] = 0;
   if(read(i2cfd, data, 1) != 1) {
      printf("Error: I2C read failure for register data 0x%02X\n", reg);
      return(-1);
   }
   if(verbose == 1) printf("Debug: UnitDefinition: [0x%02X]\n", data[0]);
   bno_ptr->unitsel = data[0];

   // extract the temperature unit
   char t_unit;
   if((data[0] >> 4) & 0x01) t_unit = 'F';
   else  t_unit = 'C';


/* ------------------------------------------------------------ *
 * Read sensor temperature from register 0x34 (no default)
 * ------------------------------------------------------------ */
   reg = BNO055_TEMP_ADDR;
   if(write(i2cfd, &reg, 1) != 1) {
      printf("Error: I2C write failure for register 0x%02X\n", reg);
      return(-1);
   }

   data[0] = 0;
   if(read(i2cfd, data, 1) != 1) {
      printf("Error: I2C read failure for register data 0x%02X\n", reg);
      return(-1);
   }
   if(verbose == 1) printf("Debug:    Temperature: [0x%02X] [%d°%c]\n", data[0], data[0], t_unit);
   bno_ptr->temp_val = data[0];

   return(0);
}

int read_mag(float *magx_ptr, float *magy_ptr, float *magz_ptr,  int verbose) {
   return(0);
}

int set_mode(opmode_t mode,  int verbose) {
/* ------------------------------------------------------------ *
 *  write the operational mode into the opr_mode register       *
 * ------------------------------------------------------------ */
   char data[2] = {0};
   data[0] = BNO055_OPR_MODE_ADDR;
   data[1] = mode;
   if(verbose == 1) printf("Debug: Write opr_mode: [0x%02X] to register [0x%02X]\n", data[1], data[0]);
   if(write(i2cfd, data, 2) != 2) {
      printf("Error: I2C write failure for register 0x%02X\n", data[0]);
      return(-1);
   }
   usleep(30 * 1000);
   return(0);
}
