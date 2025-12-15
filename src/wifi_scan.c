// =====================================
// file ini adalah implementasi sistem pemindaian WiFi pada ESP32.
// Sistem ini menginisialisasi driver WiFi dalam mode station (STA),
// melakukan pemindaian jaringan WiFi di sekitar, dan mengelola hasil pemindaian
// melalui callback yang dapat didaftarkan oleh pengguna.
// ada init, start, callback, event handler
// =====================================

#include "wifi_scan.h"

#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_err.h"
#include "esp_log.h"
#include "stdlib.h"
#include "string.h"

static const char *TAG = "WIFI_SCAN_APP";

// global buffer untuk menyimpan hasil scan
#define MAX_AP_RESULTS 32

static wifi_ap_record_t g_scan_results[MAX_AP_RESULTS];
static uint16_t g_scan_count = 0;
static volatile bool g_scan_done = false;

// global callback untuk hasil scan
static wifi_scan_callback_t g_callback = NULL;

// =========================================
// event handler untuk wifi scan selesai
// =========================================
static void wifi_scan_event_handler(void* arg, 
                                    esp_event_base_t event_base,
                                    int32_t event_id, 
                                    void* event_data){

    // pastikan event yang diterima adalah scan done/selesai
    if (event_base != WIFI_EVENT || event_id != WIFI_EVENT_SCAN_DONE){
        return;
    }
    
    uint16_t ap_num = 0;

    // ambil jumlah AP yang ditemukan
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_num)); 
    // ESP_LOGI(TAG, "Jumlah AP ditemukan: %d", ap_num);
    if (ap_num == 0){
        ESP_LOGI(TAG, "scan selesai, tidak ada AP ditemukan");
        g_scan_count = 0;
        g_scan_done = true;
        // kirim hasil scan ke callback
        if (g_callback)
            g_callback(NULL, 0);
        return;
    }
    
    if (ap_num > MAX_AP_RESULTS) ap_num = MAX_AP_RESULTS;

    // alokasi memori untuk daftar AP
    wifi_ap_record_t *ap_records = calloc(ap_num, sizeof(wifi_ap_record_t));
    if (ap_records == NULL){
        ESP_LOGE(TAG, "gagal alokasi memori untuk daftar AP");
        return;
    }
    
    // ambil daftar AP
    esp_wifi_scan_get_ap_records(&ap_num, ap_records);
    ESP_LOGI(TAG, "scan selesai, Jumlah AP ditemukan: %d",ap_num);
    
    memcpy(g_scan_results, ap_records, ap_num * sizeof(wifi_ap_record_t));
    g_scan_count = ap_num;
    g_scan_done = true;

    free(ap_records);

    if(g_callback){
        g_callback(g_scan_results, g_scan_count);   
    }

    // // kirim hasil scan ke callback
    // if (g_callback)
    //     g_callback(ap_records, ap_num);
    // // bebaskan memori
    // free(ap_records);
}


// =========================================
// inisialisasi wifi scan system
// =========================================
esp_err_t wifi_scan_system_init(void){
    // esp_err_t untuk menyimpan hasil fungsi
    esp_err_t ret;

    // register event handler untuk scan selesai
    // esp_event_handler_instance_t instance_scan;
    ret = esp_event_handler_instance_register(WIFI_EVENT,
                                              WIFI_EVENT_SCAN_DONE,
                                              &wifi_scan_event_handler,
                                              NULL,
                                              NULL); 
                                            //   &instance_scan);
    // if (ret != ESP_OK) return ret;
    // ESP_LOGI(TAG, "wifi scan system initialized");
    // vTaskDelay(pdMS_TO_TICKS(2000));
    // return ESP_OK;
    if (ret!=ESP_OK){
        ESP_LOGE(TAG, "gagal register event handler untuk scan selesai");
        return ret;
    }
    ESP_LOGI(TAG, "wifi scan system initialized");
    return ESP_OK;    
}


// =========================================
// mulai scan wifi
// =========================================
esp_err_t wifi_scan_start(bool passive, uint16_t duration){
    wifi_scan_config_t cfg = {0};

    cfg.ssid = NULL; // scan semua SSID
    cfg.bssid = NULL; // scan semua BSSID
    cfg.channel = 0; // scan semua channel
    cfg.show_hidden = true; // scan SSID tersembunyi
    // cfg.scan_time.passive = 60;

    if (passive){
        cfg.scan_type = WIFI_SCAN_TYPE_PASSIVE;
        cfg.scan_time.passive = duration;
    } else {
        cfg.scan_type = WIFI_SCAN_TYPE_ACTIVE;
        cfg.scan_time.active.min = duration;
        cfg.scan_time.active.max = duration + 10;
    }

    g_scan_done = false;

    esp_err_t err = esp_wifi_scan_start(&cfg, false); // false: non-blocking
    if (err != ESP_OK){
        ESP_LOGE(TAG, "gagal memulai scan wifi: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "memulai scan wifi (%s, durasi: %d ms)...", 
                 passive ? "pasif" : "aktif", duration);
    }
    // ESP_LOGI(TAG, "memulai scan wifi (%s, durasi: %d ms)...", 
    //          passive ? "pasif" : "aktif", duration);
    // return esp_wifi_scan_start(&cfg, false); // false: non-blocking
    return err;
}

// register callback untuk hasil scan
void wifi_scan_register_callback(wifi_scan_callback_t callback){
    g_callback = callback;
}

// =========================================
// API untuk web server mendapatkan hasil scan
// =========================================
bool wifi_scan_is_done(void){
    return g_scan_done;
}

uint16_t wifi_scan_get_count(void){
    return g_scan_count;
}

esp_err_t wifi_scan_get_results(const wifi_ap_record_t **list, uint16_t *count){
    if (!g_scan_done){
        ESP_LOGW(TAG, "scan belum selesai, tidak dapat mengambil hasil scan");
        return ESP_ERR_INVALID_STATE; // scan belum selesai
    }
    if (list) *list = g_scan_results;
    if (count) *count = g_scan_count;
    return ESP_OK;
}   