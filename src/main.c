#include <stdlib.h>

#include <stdio.h> //for basic printf commands
#include <string.h> //for handling strings
#include "freertos/FreeRTOS.h" //for delay,mutexs,semphrs rtos operations
#include "freertos/task.h"// 
#include "esp_system.h" //esp_init funtions esp_err_t 
#include "esp_wifi.h" //esp_wifi_init functions and wifi operations
#include "esp_log.h" //for showing logs
#include "esp_event.h" //for wifi event
#include "nvs_flash.h" //non volatile storage
#include "lwip/err.h" //light weight ip packets error handling
#include "lwip/sys.h" //system applications for light weight ip apps

#include "driver/gpio.h"

#include "oled_driver.h"
#include "bmp180_driver.h"


#define SSID "PTCL-BB"
#define PASS "33b58319"

char line[LINE_SIZE] = "";

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
        esp_netif_ip_info_t ip_info;
        esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
            sprintf(line, IPSTR, IP2STR(&ip_info.ip));
            oled_set_line(3, line);
        }
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



void vTaskTemperatureDisplay(void *pvParameters) {
    float temperature = 0.0f;
    
    oled_init();
    bmp180_init();
    sprintf(line,"Temperature:");
    oled_set_line(1, line);
    
    while (1) {
        temperature = bmp180_read_temperature();
        sprintf(line, "%.1f C", temperature);
        oled_set_line(2, line);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


void app_main() {
    nvs_flash_init();
    wifi_connection();

    xTaskCreatePinnedToCore(
        (TaskFunction_t)vTaskTemperatureDisplay,
        "TemperatureDisplayTask",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        5,
        NULL,
        0
    );
    
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
