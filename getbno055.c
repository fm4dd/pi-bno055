/* ------------------------------------------------------------ *
 * file:        getbno055.c                                     *
 * purpose:     Read sensor data from BNO055 IMU sensor modules *
 *                                                              *
 * params:      -a = I2C address (default: 0x29)                *
 *              -t = read acc|gyr|mag|inf (default: inf)        *
 *              -s = save calibration data to file              *
 *              -o = write read results into html file          *
 *              -v = verbose output to stdout                   *
 * return:      0 on success, and -1 on errors.                 *
 *                                                              *
 * example:	./getbno055 -a 0x29 -t mag -o bno055.htm        *
 * 1493799157 Temp=24.46*C Humidity=35.82% Pressure=1007.84hPa  *
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
char datatype[256];
char senaddr[256];
char htmfile[256];
char calfile[256];

/* ------------------------------------------------------------ *
 * print_usage() prints the programs commandline instructions.  *
 * ------------------------------------------------------------ */
void usage() {
   static char const usage[] = "Usage: getbno055 -a [hex i2c addr] -t [acc|gyr|mag|inf] -o [html-output] [-v]\n\
\n\
Command line parameters have the following format:\n\
   -a   sensor I2C bus address in hex, Example: -a 0x28 (default)\n\
   -t   read data from: Accelerometer = acc (3 values for X-Y-Z axis)\n\
                        Gyroscope     = gyr (3 values for X-Y-X axis)\n\
                        Magnetometer  = mag (3 values for X-Y-Z axis)\n\
                        Sensor Info   = inf (7 values). Example: -t mag\n\
   -c   write sensor calibration data to file, Example -c ./bno055.cal\n\
   -s   set sensor operational mode. mode arguments:\n\
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
   -o   optional, write sensor data to HTML file, Example: -o ./getsensor.html\n\
   -h   optional, display this message\n\
   -v   optional, enables debug output\n\
\n\
Usage examples:\n\
./getbno055 -a 0x28 -t inf -o ./bno055.html -v\n\
./getbno055 -a 0x28 -s ./bno055.cal -v\n";
   printf(usage);
}

/* ------------------------------------------------------------ *
 * parseargs() checks the commandline arguments with C getopt   *
 * ------------------------------------------------------------ */
void parseargs(int argc, char* argv[]) {
   int arg;
   opterr = 0;

   if(argc == 1) { usage(); exit(-1); }

   while ((arg = (int) getopt (argc, argv, "a:t:s:o:vh")) != -1) {
      switch (arg) {
         // arg -a + sensor address, type: string
         // mandatory, example: 0x29
         case 'a':
            if(verbose == 1) printf("Debug: arg -a, value %s\n", optarg);
            strncpy(senaddr, optarg, sizeof(senaddr));
            break;

         // arg -t + sensor component, type: string
         // mandatory, example: mag (magnetometer)
         case 't':
            if(verbose == 1) printf("Debug: arg -t, value %s\n", optarg);
            strncpy(datatype, optarg, sizeof(datatype));
            break;

         // arg -s + calibration file name, type: string
         // instead of -t, writes sensor calibration to file. example: ./bno055.cal
         case 's':
            calflag = 1;
            if(verbose == 1) printf("Debug: arg -s, value %s\n", optarg);
            strncpy(calfile, optarg, sizeof(calfile));
            break;

         // arg -o + dst HTML file, type: string
         // optional, example: /tmp/sensor.htm
         case 'o':
            outflag = 1;
            if(verbose == 1) printf("Debug: arg -o, value %s\n", optarg);
            strncpy(htmfile, optarg, sizeof(htmfile));
            break;

         // arg -v verbose, type: flag, optional
         case 'v':
            verbose = 1; break;

         // arg -h usage, type: flag, optional
         case 'h':
            usage(); exit(0);

         case '?':
            if(isprint (optopt))
               printf ("Error: Unknown option `-%c'.\n", optopt);
            else
               printf ("Error: Unknown option character `\\x%x'.\n", optopt);
            usage();
            exit(-1);

         default:
            usage();
      }
   }
   if (strlen(datatype) != 3) {
      printf("Error: Cannot get valid -t data type argument.\n");
      exit(-1);
   }
   if (strlen(senaddr) != 4) {
      printf("Error: Cannot get valid -a sensor address argument.\n");
      exit(-1);
   }
}

int main(int argc, char *argv[]) {
   /* ------------------------------------------------------------ *
    * Process the cmdline parameters                               *
    * ------------------------------------------------------------ */
   parseargs(argc, argv);

   /* ------------------------------------------------------------ *
    * get current time (now), write program start if verbose       *
    * ------------------------------------------------------------ */
   time_t tsnow = time(NULL);
   if(verbose == 1) printf("Debug: ts=[%lld] date=%s", (long long) tsnow, ctime(&tsnow));

   int res = -1;
   /* ----------------------------------------------------------- *
    *  "-s" reads sensor calibration data and writes it to file.  *
    * ----------------------------------------------------------- */
    if(calflag == 1) {
      struct bnocal bnoc;
      /* -------------------------------------------------------- *
       *  Check the sensors calibration state                     *
       * -------------------------------------------------------- */
      res = stat_cal(senaddr, &bnoc, verbose);
      /* -------------------------------------------------------- *
       *  Read the sensors calibration data.                      *
       * -------------------------------------------------------- */
      res = read_cal(senaddr, &bnoc, verbose);
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
    *  Read info data from the sensor                             *
    * ----------------------------------------------------------- */
   if(strcmp(datatype, "inf") == 0) {
      struct bnover bnov;
      res = read_inf(senaddr, &bnov, verbose);
      if(res != 0) {
         printf("Error: Cannot read sensor %s data, return code %d.\n", datatype, res);
         exit(-1);
      }
      /* ----------------------------------------------------------- *
       * print the formatted output string to stdout (Example below) *              
       * 1498385783 CHIPID=24 ACCID=55 GYRID=9 MAGID=3               *
       * ----------------------------------------------------------- */
      printf("%lld CHIPID=%x ACCID=%x GYRID=%x MAGID=%x OPMODE=%x\n",
            (long long) tsnow, bnov.chip_id, bnov.acc_id, bnov.gyr_id,
	     bnov.mag_id, bnov.opr_mode);

      /* ----------------------------------------------------------- *
       *  In verbose mode decode and display the operations state    *
       * ----------------------------------------------------------- */
      if(verbose == 1) {
      printf("OPMODE: ");
      switch(bnov.opr_mode) {
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
	 case 0x10:
            printf("M4G\n");
            break;
	 case 0x11:
            printf("NDOF_FMC_OFF\n");
            break;
	 case 0x12:
            printf("NDOF\n");
            break;
         }
      }
      exit(0);
   }

   /* ----------------------------------------------------------- *
    *  Read Magnetometer data from the sensor                     *
    * ----------------------------------------------------------- */
   if(strcmp(datatype, "mag") == 0) {
      float magx;
      float magy;
      float magz;
      res = read_mag(senaddr, &magx, &magy, &magz, verbose);
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
