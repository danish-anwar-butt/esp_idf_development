#include <stdlib.h>

#include <stdio.h> //for basic printf commands
#include <string.h> //for handling strings
#include "freertos/FreeRTOS.h" //for delay,mutexs,semphrs rtos operations
#include "freertos/task.h"// 

#include "esp_system.h" //esp_init funtions esp_err_t 
#include "esp_log.h" //for showing logs
#include "esp_event.h" //for wifi event


#include "esp_wifi.h" //esp_wifi_init functions and wifi operations
#include "nvs_flash.h" //non volatile storage
#include "lwip/err.h" //light weight ip packets error handling
#include "lwip/sys.h" //system applications for light weight ip apps
#include "esp_http_client.h"

#include "driver/gpio.h"
#include "oled_driver.h"
#include "bmp180_driver.h"

#include "esp_random.h"

#define SSID "PTCL-BB"
#define PASS "33b58319"

char line[LINE_SIZE] = "";

bool wifi_connected = false;

char html_page[200]; 

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
        wifi_connected = true;
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

esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt)
{
    char *buffer = (char *)evt->user_data;  // Cast back to your buffer type
    static bool body_started = false;
    
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_HEADER:
            // Completely ignore all headers
            break;
            
        case HTTP_EVENT_ON_DATA:
            // Skip until we find the double CRLF that marks end of headers
            if (!body_started) {
                char *body_start = strstr((char*)evt->data, "\r\n\r\n");
                if (body_start) {
                    body_started = true;
                    // Point to the first byte after headers
                    body_start += 4; // Skip the \r\n\r\n
                    size_t body_len = evt->data_len - (body_start - (char*)evt->data);
                    if (body_len > 0) {
                        strncat(buffer, body_start, 
                               sizeof(html_page) - strlen(buffer) - 1);
                    }
                }
            } else {
                // Normal body content
                strncat(buffer, (char*)evt->data, 
                       sizeof(html_page) - strlen(buffer) - 1);
            }
            break;
            
        case HTTP_EVENT_ON_FINISH:
            body_started = false; // Reset for next request
            break;
            
        default:
            break;
    }
    return ESP_OK;
}

static void rest_get()
{
    char url[100];

    sprintf(url, "http://numbersapi.com/%d/trivia/", (int)(esp_random() % 100));
    esp_http_client_config_t config_get = {
        .url = url,
        .method = HTTP_METHOD_GET,
        .cert_pem = NULL,
        .event_handler = client_event_get_handler,
        .user_data = html_page
    };
        
    esp_http_client_handle_t client = esp_http_client_init(&config_get);
    esp_http_client_perform(client);
    esp_http_client_cleanup(client);

    printf("End: %s\n", html_page);
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
    while (wifi_connected == false)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    rest_get();

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
