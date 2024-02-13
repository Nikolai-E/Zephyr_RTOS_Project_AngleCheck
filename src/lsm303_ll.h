#ifndef __bmp280_h
#define __bmp280_h
#define LSM303_ACCEL_ADDRESS (0x19)

struct All_Axes_Data
  {
     int X,Y,Z;
  };

int lsm303_ll_begin();
int Convert (int8_t H, int8_t L);
struct All_Axes_Data lsm303_ll_readAccel();
#endif