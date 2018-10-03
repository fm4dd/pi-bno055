/* ------------------------------------------------------------ *
 * file:        getbno055.h                                     *
 * purpose:     header file for getbno055.c and i2c_bno055.c    *
 *                                                              *
 * author:      05/04/2018 Frank4DD                             *
 * ------------------------------------------------------------ */

typedef enum {
   /* Operation mode settings*/
   OPMODE_CONFIG           = 0X00,
   OPMODE_ACCONLY          = 0X01,
   OPMODE_MAGONLY          = 0X02,
   OPMODE_GYRONLY          = 0X03,
   OPMODE_ACCMAG           = 0X04,
   OPMODE_ACCGYRO          = 0X05,
   OPMODE_MAGGYRO          = 0X06,
   OPMODE_AMG              = 0X07,
   OPMODE_IMUPLUS          = 0X08,
   OPMODE_COMPASS          = 0X09,
   OPMODE_M4G              = 0X0A,
   OPMODE_NDOF_FMC_OFF     = 0X0B,
   OPMODE_NDOF             = 0X0C
} bno055_opmode_t;

#define BNO055_OPR_MODE_ADDR 0x3D
#define BNO055_PWR_MODE_ADDR 0x3E
#define BNO055_CHIP_ID_ADDR  0x00
#define BNO055_SYS_TRIG_ADDR 0X3F
#define BNO055_PAGE_ID_ADDR  0X07

#define I2CBUS               "/dev/i2c-0"
#define BNO055_ID            0xA0
#define POWER_MODE_NORMAL    0X00
/* ------------------------------------------------------------ *
 * global variables                                             *
 * ------------------------------------------------------------ */
int i2cfd;       // I2C file descriptor

/* ------------------------------------------------------------ *
 * BNO055 versions, status data and other infos struct          *
 * ------------------------------------------------------------ */
struct bnover{
   char chip_id;  // reg 0x00 default 0xA0
   char acc_id;   // reg 0x01 default 0xFB
   char mag_id;   // reg 0x02 default 0x32
   char gyr_id;   // reg 0x03 default 0x0F
   char sw_lsb;   // reg 0x04 default 0x08
   char sw_msb;   // reg 0x05 default 0x03
   char bl_rev;   // reg 0x06 no default
   char opr_mode; // reg 0x3D default 0x1C
   char sys_stat; // reg 0x39 system error status, range 0-6
   char temp_val; // reg 0x34 sensor temperature value
};

/* ------------------------------------------------------------ *
 * BNO055 calibration data struct. The offset ranges depend on  *
 * the component operation range. For example, the accelerometer*
 * range can be set as 2G, 4G, 8G, and 16G. I.e. the offset for *
 * the accelerometer at 16G has a range of +/- 16000mG. Offset  *
 * is stored on the sensor in two bytes with max value of 32768.*
 * ------------------------------------------------------------ */
struct bnocal{
   char scal_st;  // reg 0x35 system calibration state, range 0-3
   char gcal_st;  // gyroscope calibration state, range 0-3
   char acal_st;  // accelerometer calibration state, range 0-3
   char mcal_st;  // magnetometer calibration state, range 0-3
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
 * BNO055 measurement data struct. Data gets filled in based on *
 * on the sensor component type that was requested for reading. *
 * ------------------------------------------------------------ */
struct bnodat{
   int  adata_x;   // accelerometer data, X-axis
   int  adata_y;   // accelerometer data, Y-axis
   int  adata_z;   // accelerometer data, Z-axis
   int  mdata_x;   // magnetometer data, X-axis
   int  mdata_y;   // magnetometer data, Y-axis
   int  mdata_z;   // magnetometer data, Z-axis
   int  gdata_x;   // gyroscope data, X-axis
   int  gdata_y;   // gyroscope data, Y-axis
   int  gdata_z;   // gyroscope data, Z-axis
   int eul_head;   // Euler heading data
   int eul_roll;   // Euler roll data
   int eul_pitc;   // Euler picth data
   int quater_w;   // Quaternation data W
   int quater_x;   // Quaternation data X
   int quater_y;   // Quaternation data Y
   int quater_z;   // Quaternation data Z
};

/* ------------------------------------------------------------ *
 * external function prototypes for sensor-type specific code
 * ------------------------------------------------------------ */
extern int get_i2cbus(char*, int);
extern int stat_cal(struct bnocal*, int);
extern int read_cal(struct bnocal*, int);
extern int read_inf(struct bnover*, int);
extern int read_mag(float*,  float*, float*, int);
extern int set_mode(bno055_opmode_t,  int);
extern int set_defaults(int);

