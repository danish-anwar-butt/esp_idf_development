#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "oled_driver.h"
#include "bmp180_driver.h"

#define LINE_SIZE 64
char line[LINE_SIZE] = "";


void app_main() {
	
    int x = 0;
    float temperature = 0.0f;
    oled_init();
    bmp180_init();
    
    sprintf(line,"Temperature:");
    oled_set_line(1, line);
    while (1)
    {
        
        temperature = bmp180_read_temperature();
        printf("Raw Temperature: %.1f °C\n", temperature);
        sprintf(line,"%.1f C", temperature);
        oled_set_line(3, line);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}