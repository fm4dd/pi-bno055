/* ------------------------------------------------------------ *
 * file:        getbno055.c                                     *
 * purpose:     Sensor control and data extraction program for  *
 *              the Bosch BNO055 absolute orientation sensor    *
 *                                                              *
 * return:      0 on success, and -1 on errors.                 *
 *                                                              *
 * requires:	I2C headers, e.g. sudo apt install libi2c-dev   *
 *                                                              *
 * compile:	gcc -o getbno055 i2c_bno055.c getbno055.c       *
 *                                                              *
 * example:	./getbno055 -t eul  -o bno055.htm               *
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
   static char const usage[] = "Usage: getbno055 [-a hex i2cr-addr] [-m <opr_mode>] [-t acc|gyr|mag|eul|qua|lin|gra|inf|cal] [-r] [-w calfile] [-l calfile] [-o htmlfile] [-v]\n\
\n\
Command line parameters have the following format:\n\
*  -a   sensor I2C bus address in hex, Example: -a 0x28 (default)\n\
*  -m   set sensor operational mode. mode arguments:\n\
           config   = configuration mode\n\
           acconly  = accelerometer only\n\
           magonly  = magnetometer only\n\
           gyronly  = gyroscope only\n\
           accmag   = accelerometer + magnetometer\n\
           accgyro  = accelerometer + gyroscope\n\
           maggyro  = magetometer + gyroscope\n\
           amg      = accelerometer + magnetoscope + gyro\n\
           imu      = accelerometer + gyro + rel. orientation\n\
           compass  = accelerometer + magnetometer + abs. orientation\n\
           m4g      = accelerometer + magnetometer + rel. orientation\n\
           ndof     = accel + magnetometer + gyro + abs. orientation\n\
           ndof_fmc = ndof with fast magnetometer calibration (FMC)\n\
*  -r   reset sensor\n\
   -t   read and output sensor data. data type arguments:\n\
*          acc = Accelerometer (3 values for X-Y-Z axis)\n\
           gyr = Gyroscope (3 values for X-Y-X axis)\n\
*          mag = Magnetometer (3 values for X-Y-Z axis)\n\
*          eul = Orientation E (3 values for H-R-P as Euler angles)\n\
*          qua = Orientation Q (4 values for W-X-Y-Z as Quaternation)\n\
           lin = Linear Accel (3 values for X-Y-Z axis)\n\
           gra = GravityVector (3 values for X-Y-Z axis)\n\
*          inf = Sensor info (7 values version and state)\n\
*          cal = Calibration data (9 values for each X-Y-Z)\n\
   -l   load sensor calibration data from file, Example -l ./bno055.cal\n\
   -w   write sensor calibration data to file, Example -w ./bno055.cal\n\
*  -o   output sensor data to HTML table file, requires -t, Example: -o ./getsensor.html\n\
*  -h   display this message\n\
*  -v   enable debug output\n\
\n\
Note: The sensor is executing calibration in the background, but only in fusion mode.\n\
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

   while ((arg = (int) getopt (argc, argv, "a:m:rt:l:w:o:hv")) != -1) {
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

         // arg -m + sets operations mode, type: string
         case 'm':
            if(verbose == 1) printf("Debug: arg -m, value %s\n", optarg);
            strncpy(opr_mode, optarg, sizeof(opr_mode));
            break;

         // arg -r
         // optional, resets sensor
         case 'r':
            if(verbose == 1) printf("Debug: arg -r, value %s\n", optarg);
            resflag = 1;
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

         // arg -l + calibration file name, type: string
         // loads the sensor calibration from file. example: ./bno055.cal
         case 'l':
            calflag = 1;
            if(verbose == 1) printf("Debug: arg -l, value %s\n", optarg);
            strncpy(calfile, optarg, sizeof(calfile));
            break;

         // arg -w + calibration file name, type: string
         // writes sensor calibration to file. example: ./bno055.cal
         case 'w':
            calflag = 1;
            if(verbose == 1) printf("Debug: arg -w, value %s\n", optarg);
            strncpy(calfile, optarg, sizeof(calfile));
            break;

         // arg -o + dst HTML file, type: string, requires -t
         // writes the sensor output to file. example: /tmp/sensor.htm
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

/* ----------------------------------------------------------- *
 *  print_calstat() - Read and print calibration status        *
 * ----------------------------------------------------------- */
void print_calstat() {
   struct bnocal bnoc;
   /* -------------------------------------------------------- *
    *  Check the sensors calibration state                     *
    * -------------------------------------------------------- */
   int res = get_calstatus(&bnoc);
   if(res != 0) {
      printf("Error: Cannot read calibration state.\n");
      exit(-1);
   }

   /* -------------------------------------------------------- *
    *  Convert the status code into a status message           *
    * -------------------------------------------------------- */
    printf("Sensor System Calibration = ");
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

   printf("    Gyroscope Calibration = ");
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

   printf("Accelerometer Calibration = ");
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

   printf(" Magnetometer Calibration = ");
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
}


int main(int argc, char *argv[]) {
   int res = -1;       // res = function retcode: 0=OK, -1 = Error

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
    * "-a" open the I2C bus and connect to the sensor i2c address *
    * ----------------------------------------------------------- */
   get_i2cbus(senaddr);

   /* ----------------------------------------------------------- *
    *  "-r" reset the sensor and exit the program                 *
    * ----------------------------------------------------------- */
    if(resflag == 1) {
      res = bno_reset();
      if(res != 0) {
         printf("Error: could not reset the sensor.\n");
         exit(-1);
      }
      exit(0);
   }

   /* ----------------------------------------------------------- *
    *  "-m" set the sensor operational mode and exit the program  *
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
      
      res = set_mode(newmode);
      if(res != 0) {
         printf("Error: could not set sensor mode %s [0x%02X].\n", opr_mode, newmode);
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
      res = get_calstatus(&bnoc);
      if(res != 0) {
         printf("Error: Cannot read calibration state.\n");
         exit(-1);
      }
      /* -------------------------------------------------------- *
       *  Read the sensors calibration offset                     *
       * -------------------------------------------------------- */
      res = get_caloffset(&bnoc);
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
    * -t "cal"  print the sensor calibration data                 *
    * ----------------------------------------------------------- */
   if(strcmp(datatype, "cal") == 0) {
      struct bnocal bnoc;
      /* -------------------------------------------------------- *
       *  Read the sensors calibration state                      *
       * -------------------------------------------------------- */
      res = get_calstatus(&bnoc);
      if(res != 0) {
         printf("Error: Cannot read calibration state.\n");
         exit(-1);
      }
      /* -------------------------------------------------------- *
       *  Read the sensors calibration offset                     *
       * -------------------------------------------------------- */
      res = get_caloffset(&bnoc);
      if(res != 0) {
         printf("Error: Cannot read calibration data.\n");
         exit(-1);
      }

      /* -------------------------------------------------------- *
       *  Print the calibration data line                         *
       * -------------------------------------------------------- */
      printf("Calibration state: %d", bnoc.scal_st);
      printf(" acc [S:%d ", bnoc.acal_st);
      printf("X:%d Y:%d Z:%d", bnoc.aoff_x, bnoc.aoff_y, bnoc.aoff_z);
      printf(" R:%d]", bnoc.acc_rad);

      printf(" mag [S:%d ", bnoc.mcal_st);
      printf("X:%d Y:%d Z:%d", bnoc.moff_x, bnoc.moff_y, bnoc.moff_z);
      printf(" R:%d]", bnoc.mag_rad);

      printf(" gyr [S:%d ", bnoc.gcal_st);
      printf("X:%d Y:%d Z:%d]\n", bnoc.goff_x, bnoc.goff_y, bnoc.goff_z);

      exit(0);
   }

   /* ----------------------------------------------------------- *
    * -t "inf"  print the sensor configuration                    *
    * ----------------------------------------------------------- */
   if(strcmp(datatype, "inf") == 0) {
      struct bnoinf bnoi;
      res = get_inf(&bnoi);
      if(res != 0) {
         printf("Error: Cannot read sensor version data.\n");
         exit(-1);
      }

      /* ----------------------------------------------------------- *
       * print the formatted output strings to stdout                *              
       * ----------------------------------------------------------- */
      printf("\nBN0055 Information at %s", ctime(&tsnow));
      printf("----------------------------------------------\n");
      printf("   Chip Version ID = 0x%02X\n", bnoi.chip_id);
      printf("  Accelerometer ID = 0x%02X\n", bnoi.acc_id);
      printf("      Gyroscope ID = 0x%02X\n", bnoi.gyr_id);
      printf("   Magnetoscope ID = 0x%02X\n", bnoi.mag_id);
      printf("  Software Version = %d.%d\n", bnoi.sw_msb, bnoi.sw_lsb);
      printf("   Operations Mode = "); print_mode(bnoi.opr_mode);
      printf("        Power Mode = "); print_power(bnoi.pwr_mode);
      printf("Axis Configuration = "); print_remap_conf(bnoi.axr_conf);
      printf("   Axis Remap Sign = "); print_remap_sign(bnoi.axr_sign);
      printf("System Status Code = "); print_sstat(bnoi.sys_stat);

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

      printf("\n----------------------------------------------\n");
      print_calstat();
      exit(0);
   }

   /* ----------------------------------------------------------- *
    *  "-t acc " reads accelerometer data from the sensor.        *
    * ----------------------------------------------------------- */
   if(strcmp(datatype, "acc") == 0) {
      struct bnodat bnod;
      res = get_mag(&bnod);
      if(res != 0) {
         printf("Error: Cannot read accelerometer data.\n");
         exit(-1);
      }

      /* ----------------------------------------------------------- *
       * print the formatted output string to stdout (Example below) *
       * ACC-X: 4094.50 ACC-Y: 25.56 ACC-Z: 4067.25                  *
       * ----------------------------------------------------------- */
      printf("ACC-X: %3.2f ACC-Y: %3.2f ACC-Z: %3.2f\n", bnod.adata_x, bnod.adata_y, bnod.adata_z);

      if(outflag == 1) {
         /* -------------------------------------------------------- *
          *  Open the html file for writing accelerometer data       *
          * -------------------------------------------------------- */
         FILE *html;
         if(! (html=fopen(htmfile, "w"))) {
            printf("Error open %s for writing.\n", htmfile);
            exit(-1);
         }
         fprintf(html, "<table><tr>\n");
         fprintf(html, "<td class=\"sensordata\">Accelerometer X:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.adata_x);
         fprintf(html, "<td class=\"sensorspace\"></td>\n");
         fprintf(html, "<td class=\"sensordata\">Accelerometer Y:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.adata_y);
         fprintf(html, "<td class=\"sensorspace\"></td>\n");
         fprintf(html, "<td class=\"sensordata\">Accelerometer Z:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.adata_z);
         fprintf(html, "</tr></table>\n");
         fclose(html);
      }
   } /* End reading Accelerometer */

   /* ----------------------------------------------------------- *
    *  "-t mag" reads magnetometer data from the sensor.          *
    * ----------------------------------------------------------- */
   if(strcmp(datatype, "mag") == 0) {
      struct bnodat bnod;
      res = get_mag(&bnod);
      if(res != 0) {
         printf("Error: Cannot read magnetometer data.\n");
         exit(-1);
      }

      /* ----------------------------------------------------------- *
       * Convert magnetometer data in microTesla. 1 microTesla = 16  *
       * ----------------------------------------------------------- */
      float mag_x = (float) bnod.mdata_x / 16.0;
      float mag_y = (float) bnod.mdata_y / 16.0;
      float mag_z = (float) bnod.mdata_z / 16.0;

      /* ----------------------------------------------------------- *
       * print the formatted output string to stdout (Example below) *              
       * MAG-X: 5.98 MAG-Y: 13.24 MAG-Z: -18.55                      *
       * ----------------------------------------------------------- */
      printf("MAG-X: %3.2f MAG-Y: %3.2f MAG-Z: %3.2f\n", mag_x, mag_y, mag_z);

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
         fprintf(html, "<td class=\"sensordata\">Magnetometer X:<span class=\"sensorvalue\">%3.2f</span></td>\n", mag_x);
         fprintf(html, "<td class=\"sensorspace\"></td>\n");
         fprintf(html, "<td class=\"sensordata\">Magnetometer Y:<span class=\"sensorvalue\">%3.2f</span></td>\n", mag_y);
         fprintf(html, "<td class=\"sensorspace\"></td>\n");
         fprintf(html, "<td class=\"sensordata\">Magentometer Z:<span class=\"sensorvalue\">%3.2f</span></td>\n", mag_z);
         fprintf(html, "</tr></table>\n");
         fclose(html);
      }
   } /* End reading Magnetometer data */

   /* ----------------------------------------------------------- *
    *  "-t eul" reads the Euler Orientation from the sensor.      *
    * This requires the sensor to be in fusion mode (mode > 7).   *
    * ----------------------------------------------------------- */
   if(strcmp(datatype, "eul") == 0) {

      int mode = get_mode();
      if(mode < 8) {
         printf("Error getting Euler data, sensor mode %d is not a fusion mode.\n", mode);
         exit(-1);
      }

      struct bnodat bnod;
      res = get_eul(&bnod);
      if(res != 0) {
         printf("Error: Cannot read Euler orientation data.\n");
         exit(-1);
      }

      /* ----------------------------------------------------------- *
       * print the formatted output string to stdout (Example below) *
       * EUL-H: 0.12 EUL-R: -3.31 EUL-P: -15.31 (degrees)            *
       * ----------------------------------------------------------- */
      printf("EUL-H: %3.2f EUL-R: %3.2f EUL-P: %3.2f\n", bnod.eul_head, bnod.eul_roll, bnod.eul_pitc);

      if(outflag == 1) {
         /* -------------------------------------------------------- *
          *  Open the html file for writing Euler Orientation data   *
          * -------------------------------------------------------- */
         FILE *html;
         if(! (html=fopen(htmfile, "w"))) {
            printf("Error open %s for writing.\n", htmfile);
            exit(-1);
         }
         fprintf(html, "<table><tr>\n");
         fprintf(html, "<td class=\"sensordata\">Euler Heading:<span class=\"sensorvalue\">%f</span></td>\n", bnod.eul_head);
         fprintf(html, "<td class=\"sensorspace\"></td>\n");
         fprintf(html, "<td class=\"sensordata\">Euler Roll:<span class=\"sensorvalue\">%f</span></td>\n", bnod.eul_roll);
         fprintf(html, "<td class=\"sensorspace\"></td>\n");
         fprintf(html, "<td class=\"sensordata\">Euler Pitch:<span class=\"sensorvalue\">%f</span></td>\n", bnod.eul_pitc);
         fprintf(html, "</tr></table>\n");
         fclose(html);
      }
   } /* End reading Euler Orientation */

   /* ----------------------------------------------------------- *
    *  "-t qua" reads the Quaternation data from the sensor.      *
    * This requires the sensor to be in fusion mode (mode > 7).   *
    * ----------------------------------------------------------- */
   if(strcmp(datatype, "qua") == 0) {

      int mode = get_mode();
      if(mode < 8) {
         printf("Error getting Quaternation, sensor mode %d is not a fusion mode.\n", mode);
         exit(-1);
      }

      struct bnodat bnod;
      res = get_qua(&bnod);
      if(res != 0) {
         printf("Error: Cannot read Quaternation data.\n");
         exit(-1);
      }

      /* ----------------------------------------------------------- *
       * print the formatted output string to stdout (Example below) *
       * QUA-W: -0.91 QUA-X: -0.34 QUA-Y: -0.13 QUA-Z: -0.22         *
       * ----------------------------------------------------------- */
      printf("QUA-W: %3.2f QUA-X: %3.2f QUA-Y: %3.2f QUA-Z: %3.2f\n", bnod.quater_w, bnod.quater_x, bnod.quater_y, bnod.quater_z);

      if(outflag == 1) {
         /* -------------------------------------------------------- *
          *  Open the html file for writing Quaternation data        *
          * -------------------------------------------------------- */
         FILE *html;
         if(! (html=fopen(htmfile, "w"))) {
            printf("Error open %s for writing.\n", htmfile);
            exit(-1);
         }
         fprintf(html, "<table><tr>\n");
         fprintf(html, "<td class=\"sensordata\">Quaternation W:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.quater_w);
         fprintf(html, "<td class=\"sensorspace\"></td>\n");
         fprintf(html, "<td class=\"sensordata\">Quaternation X:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.quater_x);
         fprintf(html, "<td class=\"sensorspace\"></td>\n");
         fprintf(html, "<td class=\"sensordata\">Quaternation Y:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.quater_y);
         fprintf(html, "<td class=\"sensorspace\"></td>\n");
         fprintf(html, "<td class=\"sensordata\">Quaternation Z:<span class=\"sensorvalue\">%3.2f</span></td>\n", bnod.quater_z);
         fprintf(html, "</tr></table>\n");
         fclose(html);
      }
   } /* End reading Quaternation data */
   exit(0);
}
