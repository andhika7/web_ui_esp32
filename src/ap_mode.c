// =====================================
// File ini adalah kode untuk menjalankan soft AP mode pada ESP32 (ESP32 bertindak sebagai Access Point).
// Dalam mode ini, ESP32 membuat jaringan WiFi sendiri yang dapat dihubungkan oleh perangkat lain.
// Kode ini menginisialisasi mode AP, mengatur konfigurasi AP seperti SSID dan password, serta memulai layanan WiFi AP.
// Setelah dijalankan, perangkat lain dapat menemukan dan terhubung ke jaringan WiFi yang dibuat oleh ESP32.
// Selain sebagai AP, ESP32 juga dikonfigurasi untuk berfungsi sebagai STA (station) sehingga dapat terhubung ke jaringan WiFi lain jika diperlukan.
// =====================================
#include "ap_mode.h"

#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "string.h"

static const char *TAG = "AP_STA_MODE";

void start_soft_ap_mode(void){
    
    // inisialisasi default wifi ap
    esp_netif_create_default_wifi_ap();

    // inisialisasi default wifi sta
    esp_netif_create_default_wifi_sta();

    // inisialisasi wifi 
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    // mode APSTA
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

    // konfigurasi soft AP
    wifi_config_t ap_config = {
        .ap = {
            .ssid = "ESP32_Configuration",
            .ssid_len = 0,
            .password = "config1234",
            .channel = 1,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .max_connection = 4,
        },
    };

    // jika password kosong, atur mode autentikasi ke OPEN
    if (strlen((char *)ap_config.ap.password) == 0) {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }    

    // set konfigurasi dan mulai wifi ap
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));

    //dhcp server mulai jalan setelah wifi start
    ESP_ERROR_CHECK(esp_wifi_start()); 

    ESP_LOGI(TAG, "Soft AP+STA mode started with SSID:%s, Password:%s",
             ap_config.ap.ssid, ap_config.ap.password);
}