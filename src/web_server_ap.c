#include "esp_http_server.h"
#include "esp_log.h"
#include "stdio.h"
#include "string.h"

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
        
        ESP_LOGI(TAG, "Web server started in AP mode");
    } else {
        ESP_LOGE(TAG, "Failed to start web server in AP mode");
    }
}