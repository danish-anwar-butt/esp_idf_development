// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/gpio.h"
// #include "esp_log.h"

// #include "oled_driver.h"
// #include "bmp180_driver.h"

// #define LINE_SIZE 64
// char line[LINE_SIZE] = "";


// void vTaskTemperatureDisplay(void *pvParameters) {
//     float temperature = 0.0f;
    
//     oled_init();
//     bmp180_init();
//     sprintf(line,"Temperature:");
//     oled_set_line(1, line);
    
//     while (1) {
//         temperature = bmp180_read_temperature();
//         printf("Raw Temperature: %.1f °C\n", temperature);
//         sprintf(line, "%.1f C", temperature);
//         oled_set_line(3, line);
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }


// void app_main() {
	
//     xTaskCreatePinnedToCore(
//         (TaskFunction_t)vTaskTemperatureDisplay,
//         "TemperatureDisplayTask",
//         configMINIMAL_STACK_SIZE * 2,
//         NULL,
//         5,
//         NULL,
//         0
//     );
    
//     while (1)
//     {}
// }

#include <stdio.h> //for basic printf commands
#include <string.h> //for handling strings
#include "freertos/FreeRTOS.h" //for delay,mutexs,semphrs rtos operations
#include "esp_system.h" //esp_init funtions esp_err_t 
#include "esp_wifi.h" //esp_wifi_init functions and wifi operations
#include "esp_log.h" //for showing logs
#include "esp_event.h" //for wifi event
#include "nvs_flash.h" //non volatile storage
#include "lwip/err.h" //light weight ip packets error handling
#include "lwip/sys.h" //system applications for light weight ip apps

#define SSID "PTCL-BB"
#define PASS "33b58319"

int retry_num = 0;
static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_STA_START)
    {
        printf("WIFI CONNECTING....\n");
    }
    else if (event_id == WIFI_EVENT_STA_CONNECTED)
    {
        printf("WiFi CONNECTED\n");
    }
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        printf("WiFi lost connection\n");
        if (retry_num < 5)
        {
            esp_wifi_connect();
            retry_num++;
            printf("Retrying to Connect...\n");
        }
    }
    else if (event_id == IP_EVENT_STA_GOT_IP)
    {
        printf("Wifi got IP...\n\n");
    }
}

void wifi_connection()
{
    //                          s1.4
    // 2 - Wi-Fi Configuration Phase
    esp_netif_init();
    esp_event_loop_create_default();     // event loop                    s1.2
    esp_netif_create_default_wifi_sta(); // WiFi station                      s1.3
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); //
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = SSID,
            .password = PASS
        }

    };
    // esp_log_write(ESP_LOG_INFO, "Kconfig", "SSID=%s, PASS=%s", ssid, pass);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    // 3 - Wi-Fi Start Phase
    esp_wifi_start();
    esp_wifi_set_mode(WIFI_MODE_STA);
    // 4- Wi-Fi Connect Phase
    esp_wifi_connect();
}

void app_main(void)
{
    nvs_flash_init();
    wifi_connection();
}