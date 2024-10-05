#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include <stdio.h>

static const char *TAG = "PUBMOTE-SETTINGS";

// Define the NVS namespace
#define STORAGE_NAMESPACE "settings"

// Function to initialize NVS
esp_err_t init_nvs() {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
  return ESP_OK;
}

// Function to write an integer to NVS
esp_err_t nvs_write_int(const char *key, int32_t value) {
  nvs_handle_t my_handle;
  esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    return err;
  }

  err = nvs_set_i32(my_handle, key, value);

  if (err != ESP_OK) {
    ESP_LOGI(TAG, "Write done");
  }
  else {
    ESP_LOGE(TAG, "Failed to write!");
  }

  err = nvs_commit(my_handle);
  if (err != ESP_OK) {
    ESP_LOGI(TAG, "Commit done");
  }
  else {
    ESP_LOGE(TAG, "Failed to commit!");
  }

  nvs_close(my_handle);
  return err;
}

// Function to read an integer from NVS
esp_err_t nvs_read_int(const char *key, int32_t *value) {
  nvs_handle_t my_handle;
  esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &my_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    return err;
  }

  err = nvs_get_i32(my_handle, key, value);
  switch (err) {
  case ESP_OK:
    ESP_LOGI(TAG, "Read done, value = %ld\n", *value);
    break;
  case ESP_ERR_NVS_NOT_FOUND:
    ESP_LOGE(TAG, "The value is not initialized yet!");
    break;
  default:
    ESP_LOGE(TAG, "Error (%s) reading!\n", esp_err_to_name(err));
  }

  nvs_close(my_handle);
  return err;
}

// void app_main(void) {
//   // Initialize NVS
//   esp_err_t err = nvs_init();
//   if (err != ESP_OK) {
//     printf("Error initializing NVS!\n");
//     return;
//   }

//   // Write an integer
//   err = nvs_write_int("my_key", 123);
//   if (err != ESP_OK) {
//     printf("Error writing to NVS!\n");
//     return;
//   }

//   // Read the integer
//   int32_t read_value;
//   err = nvs_read_int("my_key", &read_value);
//   if (err != ESP_OK) {
//     printf("Error reading from NVS!\n");
//     return;
//   }

//   // printf("Read value: %d\n", read_value);
// }