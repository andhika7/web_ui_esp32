#ifndef WIFI_SCAN_H
#define WIFI_SCAN_H

#include "esp_wifi.h"
#include "esp_err.h"    
#include "stdbool.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

// callback type untuk hasil scan
typedef void (*wifi_scan_callback_t)(wifi_ap_record_t *records, uint16_t ap_num);

// inisialisasi scan wifi
esp_err_t wifi_scan_system_init(void);

// mulai scan wifi
esp_err_t wifi_scan_start(bool passive, uint16_t duration);

// callback pendaftaran untuk hasil scan
void wifi_scan_register_callback(wifi_scan_callback_t cb);

// fungsi untuk memeriksa apakah scan sudah selesai
bool wifi_scan_is_done(void);   
// fungsi untuk mendapatkan jumlah AP yang ditemukan
uint16_t wifi_scan_get_count(void);
// fungsi untuk mendapatkan hasil scan
esp_err_t wifi_scan_get_results(const wifi_ap_record_t **list, uint16_t *count);

#ifdef __cplusplus
}
#endif      

#endif // WIFI_SCAN_H
