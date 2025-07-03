#ifndef OLED_DRIVER_H
#define OLED_DRIVER_H

#define TOTAL_LINES 3
#define LINE_SIZE 16


void oled_init();
void oled_set_line(int line, const char *text);
void oled_clear_line(int line) ;
void oled_clear_all_lines();

#endif // OLED_DRIVER_H