
#include "oled_driver.h"
#include "boards.h"
#include "ssd1306.h"
#include <string.h>
#include <stdio.h>

char line1[16]= "";
char line2[16]= "";
char line3[16]= "";

SSD1306_t dev;

void oled_init(){
    i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
    ssd1306_init(&dev, 128, 32);
   	ssd1306_clear_screen(&dev, false);
	ssd1306_contrast(&dev, 0xff);

	oled_clear_all_lines();
}

void oled_set_line(int line, const char *text) {
    if (line < 1 || line > TOTAL_LINES) {
        return; // Invalid line number
    }
    
    if (line == 1) {
        strncpy(line1, text, LINE_SIZE);
        ssd1306_display_text(&dev, 1, line1, LINE_SIZE, false);
    } else if (line == 2) {
        strncpy(line2, text, LINE_SIZE);
        ssd1306_display_text(&dev, 2, line2, LINE_SIZE, false);
    } else if (line == 3) {
        strncpy(line3, text, LINE_SIZE);
        ssd1306_display_text(&dev, 3, line3, LINE_SIZE, false);
    }
}

void oled_clear_line(int line) {
    if (line < 1 || line > TOTAL_LINES) {
        return; // Invalid line number
    }
    
    if (line == 1) {
        memset(line1, 0, sizeof(line1));
        ssd1306_display_text(&dev, 1, line1, LINE_SIZE, false);
    } else if (line == 2) {
        memset(line2, 0, sizeof(line2));
        ssd1306_display_text(&dev, 2, line2, LINE_SIZE, false);
    } else if (line == 3) {
        memset(line3, 0, sizeof(line3));
        ssd1306_display_text(&dev, 3, line3, LINE_SIZE, false);
    }
}

void oled_clear_all_lines() {
    oled_clear_line(1);
    oled_clear_line(2);
    oled_clear_line(3);
}