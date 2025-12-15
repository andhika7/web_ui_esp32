#include "stdio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi_scan.h"
#include "ap_mode.h"
#include "web_server_ap.h"
#include "nvs_flash.h"

#include "spiffs.h"


static const char *TAG = "MAIN_APP";

void hasil_scan(wifi_ap_record_t *records, uint16_t ap_num){
    for (int i = 0; i < ap_num; i++){
        // ESP_LOGI(TAG, "SSID: %s, RSSI: %d, CH: %d", records[i].ssid, records[i].rssi, records[i].primary);
        ESP_LOGI(TAG, "[%d] SSID: %s | RSSI: %d dBm | CH: %d",
                i, 
                (char *)records[i].ssid, 
                records[i].rssi, 
                records[i].primary);
    }
}


void app_main(){

    // =============== inisialisasi nvs, netif, dan event loop mode STA =====
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // =============== inisialisasi nvs, netif, dan event loop mode STA =====

    // ================== mulai soft AP mode ==================
    start_soft_ap_mode();

    // // debug wifi mode
    // wifi_mode_t mode;
    // ESP_ERROR_CHECK(esp_wifi_get_mode(&mode));
    // ESP_LOGI(TAG, "Current WiFi mode: %s",
    //          (mode == WIFI_MODE_AP) ? "AP" :
    //          (mode == WIFI_MODE_STA) ? "STA" :
    //          (mode == WIFI_MODE_APSTA) ? "APSTA" : "NULL"); 

    // ================== mulai web server di mode AP ==================
    // start_web_server_ap_mode();    

    // =============== init dan mulai wifi scan system ========
    ESP_ERROR_CHECK(wifi_scan_system_init());
    // // register callback untuk hasil scan
    wifi_scan_register_callback(hasil_scan);
    // // vTaskDelay(pdMS_TO_TICKS(2000));

    // // mulai scan wifi
    // ESP_ERROR_CHECK(esp_wifi_scan_start(false, 100)); // scan aktif
    // // =============== init dan mulai wifi scan system ========

    // // // ======== start web server
    // // web_server_app_start();

    spiffs_init();
    web_server_app_start();
    
}