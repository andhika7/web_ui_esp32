#pragma once

#include "esp_err.h"
#include "esp_http_server.h"

// void start_web_server_ap_mode(void);
// esp_err_t handle_scan_wifi(httpd_req_t *req);

httpd_handle_t web_server_app_start(void);
void web_server_app_stop(void);