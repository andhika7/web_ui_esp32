#include "spiffs.h"
#include "esp_spiffs.h"
#include "esp_log.h"

static const char *tag = "SPIFFS";

void spiffs_init(void){
    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };

    // mount spiffs
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        ESP_LOGE(tag, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    ESP_LOGI(tag, "SPIFFS partition size: total: %d, used: %d", total, used);
}