#include "esp_http_server.h"
#include "esp_log.h"
#include "stdio.h"
#include "string.h"
#include "cJSON.h"
#include "wifi_scan.h"

static const char *TAG = "WEB_SERVER_AP";
static httpd_handle_t server = NULL;

// handler for root URI "/" ---- untuk menampilkan halaman index.html dari SPIFFS
static esp_err_t index_handler(httpd_req_t *req){
    FILE *file = fopen("/spiffs/index.html", "r");
    if (!file){
        ESP_LOGE(TAG, "Failed to open index.html");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    // set content type
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");

    char buf[256];
    size_t len;
    // read file and send to client
    while ((len = fread(buf, 1, sizeof(buf), file)) > 0){
        httpd_resp_send_chunk(req, buf, len);
    }

    // send empty chunk to signal end of response
    httpd_resp_send_chunk(req, NULL, 0);
    fclose(file);
    return ESP_OK;
}

// handler /scan (trigger scan)
static esp_err_t scan_handler(httpd_req_t *req){
    ESP_LOGI(TAG, "Received /scan request");
    // mulai scan wifi
    esp_err_t err = wifi_scan_start(false, 150); // scan aktif, durasi 100 ms
    cJSON *root = cJSON_CreateObject();
    if (err == ESP_OK){
        cJSON_AddStringToObject(root, "status", "scan started");
    } else {
        cJSON_AddStringToObject(root, "status", "scan failed to start");
    }

    char *json = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, json);

    cJSON_Delete(root);
    free(json);
    return ESP_OK;
}


// handler /scan_results (get scan results) -- JSON
static esp_err_t scan_results_handler(httpd_req_t *req){
    ESP_LOGI(TAG, "Received /scan_results request");

    const wifi_ap_record_t *ap_list = NULL;
    uint16_t ap_count = 0;

    if(!wifi_scan_is_done()){
        cJSON *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "status", "scan in progress");
        ESP_LOGW(TAG, "scan belum selesai, tidak dapat mengirim hasil scan");

        char *json = cJSON_PrintUnformatted(root);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, json);

        cJSON_Delete(root);
        free(json);
        return ESP_OK;
    }
    
    wifi_scan_get_results(&ap_list, &ap_count);
    cJSON *root = cJSON_CreateObject();
    cJSON *arr = cJSON_CreateArray();

    cJSON_AddStringToObject(root, "status", "scan completed");
    cJSON_AddNumberToObject(root, "ap_count", ap_count);

    for (int i = 0; i < ap_count; i++){
        cJSON *ap = cJSON_CreateObject();

        cJSON_AddStringToObject(ap, "ssid", (char *)ap_list[i].ssid);
        cJSON_AddNumberToObject(ap, "rssi", ap_list[i].rssi);
        cJSON_AddNumberToObject(ap, "channel", ap_list[i].primary);
        cJSON_AddNumberToObject(ap, "authmode", ap_list[i].authmode);
        cJSON_AddItemToArray(arr, ap);
    }
    cJSON_AddItemToObject(root, "aps", arr);


    char *json = cJSON_Print(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, json);

    cJSON_Delete(root);
    free(json);
    return ESP_OK;
}



void web_server_app_start(void){
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // start http server -- - inisialisasi web server
    if (httpd_start(&server, &config) == ESP_OK){
        // register URI handlers
        httpd_uri_t index_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = index_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &index_uri);
        
        httpd_uri_t scan_uri = {
            .uri = "/scan",
            .method = HTTP_GET,
            .handler = scan_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &scan_uri);

        httpd_uri_t scan_results_uri = {
            .uri = "/scan_results",
            .method = HTTP_GET,
            .handler = scan_results_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &scan_results_uri);

        ESP_LOGI(TAG, "Web server started in AP mode");
    } else {
        ESP_LOGE(TAG, "Failed to start web server in AP mode");
    }
}