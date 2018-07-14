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
};

/* ------------------------------------------------------------ *
 * external function prototypes for sensor-type specific code
 * ------------------------------------------------------------ */
extern int read_inf(char*,  struct bnover*, int);
extern int read_mag(char*,  float*,  float*, float*, int);
