#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "bmp180_driver.h"

bmp180_calib_t calib;

extern i2c_master_bus_handle_t bus_handle;
i2c_master_dev_handle_t dev_handle;

// Add this where you initialize your I2C devices
i2c_device_config_t bmp180_cfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = BMP180_ADDRESS,
    .scl_speed_hz = I2C_MASTER_FREQ_HZ,  // Same as display
};

void bmp180_init() {    
	ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &bmp180_cfg, &dev_handle));

    calib.ac1 = bmp180_read_short(0xAA);
    calib.ac2 = bmp180_read_short(0xAC);
    calib.ac3 = bmp180_read_short(0xAE);
    calib.ac4 = bmp180_read_ushort(0xB0);
    calib.ac5 = bmp180_read_ushort(0xB2);
    calib.ac6 = bmp180_read_ushort(0xB4);
    calib.b1 = bmp180_read_short(0xB6);
    calib.b2 = bmp180_read_short(0xB8);
    calib.mb = bmp180_read_short(0xBA);
    calib.mc = bmp180_read_short(0xBC);
    calib.md = bmp180_read_short(0xBE);
    
    // Print calibration data for debugging
    print_calibration_data();
}


// Debug function to print calibration data
void print_calibration_data() {
    printf("Calibration Data:\n");
    printf("AC1: %d\n", calib.ac1);
    printf("AC2: %d\n", calib.ac2);
    printf("AC3: %d\n", calib.ac3);
    printf("AC4: %u\n", calib.ac4);
    printf("AC5: %u\n", calib.ac5);
    printf("AC6: %u\n", calib.ac6);
    printf("B1: %d\n", calib.b1);
    printf("B2: %d\n", calib.b2);
    printf("MB: %d\n", calib.mb);
    printf("MC: %d\n", calib.mc);
    printf("MD: %d\n", calib.md);
}

bool bmp180_read_bytes(uint8_t reg, uint8_t *buf, uint8_t len) {{
    esp_err_t ret;
    
    // --- Step 1: Write register address ---
    ret = i2c_master_transmit(dev_handle, &reg, 1, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    if (ret != ESP_OK) {
        return false;
    }

    // --- Step 2: Read data ---
    ret = i2c_master_receive(dev_handle, buf, len, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    if (ret != ESP_OK) {
        ESP_LOGE("BMP180", "I2C read failed: %s", esp_err_to_name(ret));
        return false;
    }

    return true;
}

    

    return true;
}
// Improved I2C read function with error checking

int16_t bmp180_read_short(uint8_t reg) {
    uint8_t buf[2];
    if(!bmp180_read_bytes(reg, buf, 2)) return 0;
    return (int16_t)((buf[0] << 8) | buf[1]);
}

uint16_t bmp180_read_ushort(uint8_t reg) {
    uint8_t buf[2];
    if(!bmp180_read_bytes(reg, buf, 2)) return 0;
    return (uint16_t)((buf[0] << 8) | buf[1]);
}


float bmp180_read_temperature() {
    // Request temperature measurement
    uint8_t cmd[2] = {0xF4, 0x2E};
    // Write register address
    i2c_master_transmit(
        dev_handle,  // Use the BMP180 device handle
        cmd,            // Pointer to command/data (e.g., {reg_addr, value})
        2,              // Number of bytes to write
        pdMS_TO_TICKS(100)
    );
    // Wait for conversion (4.5ms)
    vTaskDelay(pdMS_TO_TICKS(5));
    
    // Read raw temperature
    int32_t ut = bmp180_read_short(0xF6);
    
    // Calculate true temperature (B5 is needed for pressure calculation)
    int32_t x1 = ((ut - (int32_t)calib.ac6) * (int32_t)calib.ac5) >> 15;
    int32_t x2 = ((int32_t)calib.mc << 11) / (x1 + calib.md);
    int32_t b5 = x1 + x2;
    float temp = ((b5 + 8) >> 4) / 10.0f;
    
    return temp;
}