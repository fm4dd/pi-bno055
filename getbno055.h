/* ------------------------------------------------------------ *
 * file:        getbno055.h                                     *
 * purpose:     header file for getbno055.c and i2c_bno055.c    *
 *                                                              *
 * author:      05/04/2018 Frank4DD                             *
 * ------------------------------------------------------------ *
 * Global variables and defaults                                *
 * ------------------------------------------------------------ */
struct bnover{
   char chip_id;  // default 0xA0
   char acc_id;   // default 0xFB
   char mag_id;   // default 0x32
   char gyr_id;   // default 0x0F
   char sw_lsb;   // default 0x08
   char sw_msb;   // default 0x03
   char bl_rev;   // no default
   char opr_mode; // default 0x1C
};

/* ------------------------------------------------------------ *
 * BNO055 calibration data struct. The offset ranges depend on  *
 * the component operation range. For example, the accelerometer*
 * range can be set as 2G, 4G, 8G, and 16G. I.e. the offset for *
 * the accelerometer at 16G has a range of +/- 16000mG. Offset  *
 * is stored on the sensor in two bytesi with max value of 32768.*
 * ------------------------------------------------------------ */
struct bnocal{
   char scal_st;  // system calibration state
   char gcal_st;  // gyroscope calibration state
   char acal_st;  // accelerometer calibration state
   char mcal_st;  // magnetometer calibration state
   int  aoff_x;   // accelerometer offset, X-axis
   int  aoff_y;   // accelerometer offset, Y-axis
   int  aoff_z;   // accelerometer offset, Z-axis
   int  moff_x;   // magnetometer offset, X-axis
   int  moff_y;   // magnetometer offset, Y-axis
   int  moff_z;   // magnetometer offset, Z-axis
   int  goff_x;   // gyroscope offset, X-axis
   int  goff_y;   // gyroscope offset, Y-axis
   int  goff_z;   // gyroscope offset, Z-axis
};

/* ------------------------------------------------------------ *
 * external function prototypes for sensor-type specific code
 * ------------------------------------------------------------ */
extern int stat_cal(char*,  struct bnocal*, int);
extern int read_cal(char*,  struct bnocal*, int);
extern int read_inf(char*,  struct bnover*, int);
extern int read_mag(char*,  float*,  float*, float*, int);
