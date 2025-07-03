#ifndef BMP180_DRIVER_H
#define BMP180_DRIVER_H

#include "driver/i2c_master.h"

#define BMP180_ADDRESS 0x77
#define I2C_MASTER_FREQ_HZ 400000 // I2C clock of SSD1306 can run at 400 kHz max.

// Calibration structure
typedef struct {
    int16_t ac1, ac2, ac3, b1, b2, mb, mc, md;
    uint16_t ac4, ac5, ac6;
} bmp180_calib_t;


#define I2C_TIMEOUT_MS  100

void bmp180_init() ;
bool bmp180_read_bytes(uint8_t reg, uint8_t *buf, uint8_t len);
int16_t bmp180_read_short(uint8_t reg);
uint16_t bmp180_read_ushort(uint8_t reg);
float bmp180_read_temperature();
int32_t bmp180_read_pressure();
void print_calibration_data(); 
#endif // BMP180_DRIVER_H