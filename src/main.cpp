#include <Arduino.h>
// Library for NVS
#include "nvs.h"
#include "nvs_flash.h"

// Định nghĩa tên namespace và khóa
#define STORAGE_NAMESPACE "storage"
#define STORAGE_KEY "data"

void initialize_nvs();
void save_data_to_nvs(uint8_t data[], size_t size);
void read_data_from_nvs(uint8_t data[], size_t size);

uint8_t data_to_save[1200] = {0};
uint8_t data_from_nvs[1200] = {0};
uint8_t test_counter = 0;

void setup()
{
  log_i("Begin start project");
  initialize_nvs();

  // Read
  read_data_from_nvs(data_from_nvs, sizeof(data_from_nvs));
  log_i("read_data_from_nvs start project: %d \n", data_from_nvs[5]);
  delay(10000);
}

void loop()
{
  log_i("test_counter: %d", test_counter);
  log_i("read_data_from_nvs 1: %d", data_from_nvs[5]);
  log_i("read_data_from_nvs 1: %d", data_from_nvs[5]);

  // Write
  data_to_save[5] = test_counter;
  save_data_to_nvs(data_to_save, sizeof(data_to_save));
  log_i("save_data_to_nvs 2: %d", data_to_save[5]);

  // Read
  read_data_from_nvs(data_from_nvs, sizeof(data_from_nvs));
  log_i("read_data_from_nvs 2: %d", data_from_nvs[5]);

  test_counter ++; 
  delay(2000);
}

void initialize_nvs()
{
  log_i("Begin initialize_nvs");
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    // NVS partition was truncated and needs to be erased
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  log_i("Finish initialize_nvs");
}

void save_data_to_nvs(uint8_t data[], size_t size)
{
  log_i("Begin save_data_to_nvs");
  nvs_handle_t nvs_handle;
  esp_err_t err;

  // Mở NVS để ghi dữ liệu
  err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
  if (err != ESP_OK)
  {
    log_e("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    return;
  }

  // Lưu dữ liệu vào NVS
  err = nvs_set_blob(nvs_handle, STORAGE_KEY, data, size);
  if (err != ESP_OK)
  {
    log_e("Error (%s) setting data to NVS!\n", esp_err_to_name(err));
  }

  // Đóng NVS handle
  nvs_close(nvs_handle);
  log_i("Finish save_data_to_nvs");
}

void read_data_from_nvs(uint8_t data[], size_t size)
{
  log_i("Begin read_data_from_nvs");
  nvs_handle_t nvs_handle;
  esp_err_t err;

  err = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &nvs_handle);
  if (err != ESP_OK)
  {
    log_e("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    return;
  }

  size_t required_size;
  // Đọc dữ liệu từ NVS
  err = nvs_get_blob(nvs_handle, STORAGE_KEY, NULL, &required_size);
  if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
  {
    log_e("Error (%s) reading data from NVS!\n", esp_err_to_name(err));
    nvs_close(nvs_handle);
    return;
  }

  if (required_size != size)
  {
    log_e("Error: Size mismatch between read data and expected data!\n");
    nvs_close(nvs_handle);
    return;
  }

  err = nvs_get_blob(nvs_handle, STORAGE_KEY, data, &required_size);
  if (err != ESP_OK)
  {
    log_e("Error (%s) reading data from NVS!\n", esp_err_to_name(err));
  }

  nvs_close(nvs_handle);
  log_i("Finish read_data_from_nvs");
}