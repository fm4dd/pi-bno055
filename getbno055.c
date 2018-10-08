/* ------------------------------------------------------------ *
 * file:        getbno055.c                                     *
 * purpose:     Read sensor data from BNO055 IMU sensor modules *
 *                                                              *
 * params:      -a = I2C address (default: 0x28)                *
 *              -t = read acc|gyr|mag|inf (default: inf)        *
 *              -s = save calibration data to file              *
 *              -o = write read results into html file          *
 *              -v = verbose output to stdout                   *
 * return:      0 on success, and -1 on errors.                 *
 *                                                              *
 * example:	./getbno055 -a 0x28 -t mag -o bno055.htm        *
 *                                                              *
 * author:      05/04/2018 Frank4DD                             *
 * ------------------------------------------------------------ */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "getbno055.h"

/* ------------------------------------------------------------ *
 * Global variables and defaults                                *
 * ------------------------------------------------------------ */
int verbose = 0;
int outflag = 0;
int calflag = 0;
int resflag = 0;
char opr_mode[9] = {0};
char datatype[256];
char senaddr[256] = "0x28";
char htmfile[256];
char calfile[256];

/* ------------------------------------------------------------ *
 * print_usage() prints the programs commandline instructions.  *
 * ------------------------------------------------------------ */
void usage() {
   static char const usage[] = "Usage: getbno055 [-a hex i2c addr] -t acc|gyr|mag|inf|cal [-r] [-s <opr_mode>] [-w] [-o html-output] [-v]\n\
\n\
Command line parameters have the following format:\n\
   -a   sensor I2C bus address in hex, Example: -a 0x28 (default)\n\
   -t   read data from: Accelerometer = acc (3 values for X-Y-Z axis)\n\
                        Gyroscope     = gyr (3 values for X-Y-X axis)\n\
                        Magnetometer  = mag (3 values for X-Y-Z axis)\n\
                        Sensor Info   = inf (7 values version and state\n\
                        Calibration   = cal (9 values for each X-Y-Z)\n\
   -r   optional, reset sensor\n\
   -s   set sensor operational mode. mode arguments:\n\
                        config = configuration mode\n\
                        acconly = accelerometer only\n\
			magonly = magnetometer only\n\
			gyronly = gyroscope only\n\
			accmag = accelerometer + mangetometer\n\
			accgyro = accelerometer + gyroscope\n\
			maggyro = magetometer + gyroscope\n\
			amg = accelerometer + magnetoscope + gyro\n\
			imu = accelerometer + gyro + rel. orientation\n\
			compass = accelerometer + magnetometer + abs. orientation\n\
			m4g = accelerometer + magnetometer + rel. orientation\n\
			ndof = accel + magnetometer + gyro + abs. orientation\n\
			ndof_fmc = ndof with fast magnetometer calibration (FMC)\n\
   -w   optional, write sensor calibration data to file, Example -c ./bno055.cal\n\
   -o   optional, write sensor data to HTML file, Example: -o ./getsensor.html\n\
   -h   optional, display this message\n\
   -v   optional, enables debug output\n\
\n\
Usage examples:\n\
./getbno055 -a 0x28 -t inf -v\n\
./getbno055 -t cal -v\n\
./getbno055 -t mag -o ./bno055.html -v\n\
./getbno055 -s ndof -v\n\
./getbno055 -w ./bno055.cal -v\n";
   printf(usage);
}

/* ------------------------------------------------------------ *
 * parseargs() checks the commandline arguments with C getopt   *
 * ------------------------------------------------------------ */
void parseargs(int argc, char* argv[]) {
   int arg;
   opterr = 0;

   if(argc == 1) { usage(); exit(-1); }

   while ((arg = (int) getopt (argc, argv, "a:t:rs:w:o:hv")) != -1) {
      switch (arg) {
         // arg -v verbose, type: flag, optional
         case 'v':
            verbose = 1; break;

         // arg -a + sensor address, type: string
         // mandatory, example: 0x29
         case 'a':
            if(verbose == 1) printf("Debug: arg -a, value %s\n", optarg);
            if (strlen(optarg) != 4) {
               printf("Error: Cannot get valid -a sensor address argument.\n");
               exit(-1);
            }
            strncpy(senaddr, optarg, sizeof(senaddr));
            break;

         // arg -t + sensor component, type: string
         // mandatory, example: mag (magnetometer)
         case 't':
            if(verbose == 1) printf("Debug: arg -t, value %s\n", optarg);
            if (strlen(optarg) != 3) {
               printf("Error: Cannot get valid -t data type argument.\n");
               exit(-1);
            }
            strncpy(datatype, optarg, sizeof(datatype));
            break;

         // arg -r
         // optional, resets sensor
         case 'r':
            if(verbose == 1) printf("Debug: arg -r, value %s\n", optarg);
            resflag = 1;
            break;

         // arg -s + sets operations mode, type: string
         case 's':
            if(verbose == 1) printf("Debug: arg -s, value %s\n", optarg);
            strncpy(opr_mode, optarg, sizeof(opr_mode));
            break;

         // arg -w + calibration file name, type: string
         // instead of -t, writes sensor calibration to file. example: ./bno055.cal
         case 'w':
            calflag = 1;
            if(verbose == 1) printf("Debug: arg -w, value %s\n", optarg);
            strncpy(calfile, optarg, sizeof(calfile));
            break;

         // arg -o + dst HTML file, type: string
         // optional, example: /tmp/sensor.htm
         case 'o':
            outflag = 1;
            if(verbose == 1) printf("Debug: arg -o, value %s\n", optarg);
            strncpy(htmfile, optarg, sizeof(htmfile));
            break;

         // arg -h usage, type: flag, optional
         case 'h':
            usage(); exit(0);
            break;

         case '?':
            if(isprint (optopt))
               printf ("Error: Unknown option `-%c'.\n", optopt);
            else
               printf ("Error: Unknown option character `\\x%x'.\n", optopt);
            usage();
            exit(-1);
            break;

         default:
            usage();
            break;
      }
   }
}

int main(int argc, char *argv[]) {
   /* ---------------------------------------------------------- *
    * Process the cmdline parameters                             *
    * ---------------------------------------------------------- */
   parseargs(argc, argv);

   /* ----------------------------------------------------------- *
    * get current time (now), write program start if verbose      *
    * ----------------------------------------------------------- */
   time_t tsnow = time(NULL);
   if(verbose == 1) printf("Debug: ts=[%lld] date=%s", (long long) tsnow, ctime(&tsnow));

   /* ----------------------------------------------------------- *
    * Open the I2C bus and connect to the sensor                  *
    * ----------------------------------------------------------- */
   int res = -1;       // res = function retcode: 0=OK, -1 = Error
   res = get_i2cbus(senaddr, verbose);

   /* ----------------------------------------------------------- *
    * Set defaults ops_mode=NDOF and power_mode=normal            *
    * ----------------------------------------------------------- */
   //res = set_defaults(verbose);

   /* ----------------------------------------------------------- *
    *  "-r" reset the sensor and exit the program                 *
    * ----------------------------------------------------------- */
    if(resflag == 1) {
      res = bno_reset(verbose);
      if(res != 0) {
         printf("Error: could not reset the sensor.\n");
         exit(-1);
      }
      exit(0);
   }

   /* ----------------------------------------------------------- *
    *  "-s" set the sensor operational mode and exit the program  *
    * ----------------------------------------------------------- */
   if(strlen(opr_mode) > 0) {
      opmode_t newmode;
      if(strcmp(opr_mode, "config")   == 0) newmode = config;
      else if(strcmp(opr_mode, "acconly")  == 0) newmode = acconly;
      else if(strcmp(opr_mode, "magonly")  == 0) newmode = magonly;
      else if(strcmp(opr_mode, "gyronly")  == 0) newmode = gyronly;
      else if(strcmp(opr_mode, "accmag")   == 0) newmode = accmag;
      else if(strcmp(opr_mode, "accgyro")  == 0) newmode = accgyro;
      else if(strcmp(opr_mode, "maggyro")  == 0) newmode = maggyro;
      else if(strcmp(opr_mode, "amg")      == 0) newmode = amg;
      else if(strcmp(opr_mode, "imu")      == 0) newmode = imu;
      else if(strcmp(opr_mode, "compass")  == 0) newmode = compass;
      else if(strcmp(opr_mode, "m4g")      == 0) newmode = m4g;
      else if(strcmp(opr_mode, "ndof")     == 0) newmode = ndof;
      else if(strcmp(opr_mode, "dnof_fmc") == 0) newmode = ndof_fmc;
      else {
         printf("Error: invalid operations mode %s.\n", opr_mode);
         exit(-1);
      }
      
      res = set_mode(newmode, verbose);
      if(res != 0) {
         printf("Error: could not reset the sensor.\n");
         exit(-1);
      }
      exit(0);
   }

   /* ----------------------------------------------------------- *
    *  "-w" writes sensor calibration data to file.               *
    * ----------------------------------------------------------- */
    if(calflag == 1) {
      struct bnocal bnoc;
      /* -------------------------------------------------------- *
       *  Check the sensors calibration state                     *
       * -------------------------------------------------------- */
      res = stat_cal(&bnoc, verbose);
      if(res != 0) {
         printf("Error: Cannot read calibration state.\n");
         exit(-1);
      }
      /* -------------------------------------------------------- *
       *  Read the sensors calibration data.                      *
       * -------------------------------------------------------- */
      res = read_cal(&bnoc, verbose);
      if(res != 0) {
         printf("Error: Cannot read calibration data.\n");
         exit(-1);
      }
      /* -------------------------------------------------------- *
       *  Open the calibration data file for writing.             *
       * -------------------------------------------------------- */
      FILE *calib;
      if(! (calib=fopen(calfile, "w"))) {
         printf("Error open %s for writing.\n", calfile);
         exit(-1);
      }
      exit(0);
   }

   /* ----------------------------------------------------------- *
    *  Read and print calibration state and calibration data      *
    * ----------------------------------------------------------- */
   if(strcmp(datatype, "cal") == 0) {
      struct bnocal bnoc;
      /* -------------------------------------------------------- *
       *  Check the sensors calibration state                     *
       * -------------------------------------------------------- */
      res = stat_cal(&bnoc, verbose);
      if(res != 0) {
         printf("Error: Cannot read calibration state.\n");
         exit(-1);
      }

      printf("\nBN0055 Calibration at %s", ctime(&tsnow));
      printf("----------------------------------------------\n");
      printf("Sensor System Calibration State = ");
      switch(bnoc.scal_st) {
         case 0:
            printf("Uncalibrated\n");
            break;
	 case 1:
            printf("Minimal Calibrated\n");
            break;
	 case 2:
            printf("Mostly Calibrated\n");
            break;
	 case 3:
            printf("Fully calibrated\n");
            break;
      }

      printf("    Gyroscope Calibration State = ");
      switch(bnoc.gcal_st) {
         case 0:
            printf("Uncalibrated\n");
            break;
         case 1:
            printf("Minimal Calibrated\n");
            break;
         case 2:
            printf("Mostly Calibrated\n");
            break;
         case 3:
            printf("Fully calibrated\n");
            break;
      }

      printf("Accelerometer Calibration State = ");
      switch(bnoc.acal_st) {
         case 0:
            printf("Uncalibrated\n");
            break;
         case 1:
            printf("Minimal Calibrated\n");
            break;
         case 2:
            printf("Mostly Calibrated\n");
            break;
         case 3:
            printf("Fully calibrated\n");
            break;
      }

      printf(" Magnetometer Calibration State = ");
      switch(bnoc.mcal_st) {
         case 0:
            printf("Uncalibrated\n");
            break;
         case 1:
            printf("Minimal Calibrated\n");
            break;
         case 2:
            printf("Mostly Calibrated\n");
            break;
         case 3:
            printf("Fully calibrated\n");
            break;
      }

      /* -------------------------------------------------------- *
       *  Read the sensors calibration data.                      *
       * -------------------------------------------------------- */
      res = read_cal(&bnoc, verbose);
      if(res != 0) {
         printf("Error: Cannot read calibration data.\n");
         exit(-1);
      }

      printf("----------------------------------------------\n");
      printf("Accelerometer Calibration Offset = ");
      printf("X:%5d Y:%5d Z:%5d\n", bnoc.aoff_x, bnoc.aoff_y, bnoc.aoff_z);

      printf(" Magnetometer Calibration Offset = ");
      printf("X:%5d Y:%5d Z:%5d\n", bnoc.moff_x, bnoc.moff_y, bnoc.moff_z);

      printf("    Gyroscope Calibration Offset = ");
      printf("X:%5d Y:%5d Z:%5d\n", bnoc.goff_x, bnoc.goff_y, bnoc.goff_z);

      exit(0);
   }

   /* ----------------------------------------------------------- *
    *  Read and print componenent versions and operations mode    *
    * ----------------------------------------------------------- */
   if(strcmp(datatype, "inf") == 0) {
      struct bnoinf bnoi;
      res = read_inf(&bnoi, verbose);
      if(res != 0) {
         printf("Error: Cannot read sensor version data.\n");
         exit(-1);
      }

      /* ----------------------------------------------------------- *
       * print the formatted output string to stdout (Example below) *              
       * 1498385783 CHIPID=24 ACCID=55 GYRID=9 MAGID=3               *
       * ----------------------------------------------------------- */
      printf("\nBN0055 Information at %s", ctime(&tsnow));
      printf("----------------------------------------------\n");
      printf("   Chip Version ID = 0x%02X\n", bnoi.chip_id);
      printf("  Accelerometer ID = 0x%02X\n", bnoi.acc_id);
      printf("      Gyroscope ID = 0x%02X\n", bnoi.gyr_id);
      printf("   Magnetoscope ID = 0x%02X\n", bnoi.mag_id);
      printf("   Operations Mode = ");
      switch(bnoi.opr_mode) {
         case 0x00:
            printf("CONFIG\n");
            break;
	 case 0x01:
            printf("ACCONLY\n");
            break;
	 case 0x02:
            printf("MAGONLY\n");
            break;
	 case 0x03:
            printf("GYRONLY\n");
            break;
	 case 0x04:
            printf("ACCMAG\n");
            break;
	 case 0x05:
            printf("ACCGYRO\n");
            break;
	 case 0x06:
            printf("MAGGYRO\n");
            break;
	 case 0x07:
            printf("AMG\n");
            break;
	 case 0x08:
            printf("IMU\n");
            break;
	 case 0x09:
            printf("COMPASS\n");
            break;
	 case 0x0A:
            printf("M4G\n");
            break;
	 case 0x0B:
            printf("NDOF_FMC_OFF\n");
            break;
	 case 0x0C:
            printf("NDOF\n");
            break;
      }

      printf("System Status Code = ");
      switch(bnoi.sys_stat) {
         case 0x00:
            printf("Idle\n");
            break;
         case 0x01:
            printf("System Error\n");
            break;
         case 0x02:
            printf("Initializing Peripherals\n");
            break;
         case 0x03:
            printf("System Initalization\n");
            break;
         case 0x04:
            printf("Executing Self-Test\n");
            break;
         case 0x05:
            printf("Sensor running with fusion algorithm\n");
            break;
         case 0x06:
            printf("System running without fusion algorithm\n");
            break;
      }

      printf("Accelerometer Test = ");
      if((bnoi.selftest >> 0) & 0x01) printf("OK\n");
      else printf("FAIL\n");

      printf(" Magnetometer Test = ");
      if((bnoi.selftest >> 1) & 0x01) printf("OK\n");
      else printf("FAIL\n");

      printf("    Gyroscope Test = ");
      if((bnoi.selftest >> 2) & 0x01) printf("OK\n");
      else printf("FAIL\n");

      printf("MCU Cortex M0 Test = ");
      if((bnoi.selftest >> 3) & 0x01) printf("OK\n");
      else printf("FAIL\n");

      printf(" System Error Code = ");
      switch(bnoi.sys_err) {
         case 0x00:
            printf("No Error\n");
            break;
         case 0x01:
            printf("Peripheral initialization error\n");
            break;
         case 0x02:
            printf("System initializion error\n");
            break;
         case 0x03:
            printf("Selftest result failed\n");
            break;
         case 0x04:
            printf("Register map value out of range\n");
            break;
         case 0x05:
            printf("Register map address out of range\n");
            break;
         case 0x06:
            printf("Register map write error\n");
            break;
         case 0x07:
            printf("BNO low power mode not available\n");
            break;
         case 0x08:
            printf("Accelerometer power mode not available\n");
            break;
         case 0x09:
            printf("Fusion algorithm configuration error\n");
            break;
         case 0x0A:
            printf("Sensor configuration error\n");
            break;
      }

      // Unit Selection bit-0
      printf("MCU Cortex M0 Test = ");
      printf("Accelerometer Unit = ");
      if((bnoi.unitsel >> 0) & 0x01) printf("mg\n");
      else printf("m/s2\n");

      // Unit Selection bit-1
      printf("    Gyroscope Unit = ");
      if((bnoi.unitsel >> 1) & 0x01) printf("rps\n");
      else printf("dps\n");

      // Unit Selection bit-2
      printf("        Euler Unit = ");
      if((bnoi.unitsel >> 2) & 0x01) printf("Radians\n");
      else printf("Degrees\n");

      // Unit Selection bit-3: unused
      // Unit Selection bit-4
      printf("  Temperature Unit = ");
      if((bnoi.unitsel >> 4) & 0x01) printf("Fahrenheit\n");
      else printf("Celsius\n");

      // Unit Selection bit-5: unused
      // Unit Selection bit-6: unused
      // Unit Selection bit-7
      printf("  Orientation Mode = ");
      if((bnoi.unitsel >> 7) & 0x01) printf("Android\n");
      else printf("Windows\n");

      printf("Sensor Temperature = %d", bnoi.temp_val);
      if((bnoi.unitsel >> 4) & 0x01) printf("°F\n");
      else printf("°C\n");

      exit(0);
   }

   /* ----------------------------------------------------------- *
    *  Read Magnetometer data from the sensor                     *
    * ----------------------------------------------------------- */
   if(strcmp(datatype, "mag") == 0) {
      float magx;
      float magy;
      float magz;
      res = read_mag(&magx, &magy, &magz, verbose);
      if(res != 0) {
         printf("Error: Cannot read %s data, return code %d.\n", datatype, res);
         exit(-1);
      }
      /* ----------------------------------------------------------- *
       * print the formatted output string to stdout (Example below) *              
       * 1498385783 MAG-X=27.34 MAG-Y=55.82 MAG-Z=92.00              *
       * ----------------------------------------------------------- */
      printf("%lld MAG-X=%.2f MAG-Y=%.2f MAG-Z=%.2f\n",
            (long long) tsnow, magx, magy, magz);

      if(outflag == 1) {
         /* -------------------------------------------------------- *
          *  Open the html file for writing Magnetometer data        *
          * -------------------------------------------------------- */
         FILE *html;
         if(! (html=fopen(htmfile, "w"))) {
            printf("Error open %s for writing.\n", htmfile);
            exit(-1);
         }
         fprintf(html, "<table><tr>\n");
         fprintf(html, "<td class=\"sensordata\">Magnetometer X:<span class=\"sensorvalue\">%.2f</span></td>\n", magx);
         fprintf(html, "<td class=\"sensorspace\"></td>\n");
         fprintf(html, "<td class=\"sensordata\">Magnetometer Y:<span class=\"sensorvalue\">%.2f</span></td>\n", magy);
         fprintf(html, "<td class=\"sensorspace\"></td>\n");
         fprintf(html, "<td class=\"sensordata\">Magentometer Z:<span class=\"sensorvalue\">%.2f</span></td>\n", magz);
         fprintf(html, "</tr></table>\n");
         fclose(html);
      }
   } /* End reading Magnetometer */
   exit(0);
}
