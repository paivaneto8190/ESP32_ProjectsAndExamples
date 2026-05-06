#ifndef SPIFFS_MANAGER
#define SPIFFS_MANAGER

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_spiffs.h"
#include <unistd.h>
#include "esp_err.h"
#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *base_path; // File path prefix
    const char *partition_label; // Label of spiffs partition to use

    size_t max_files; // Number of files that can be open simultaneously
} spiffs_manager_config_t;

typedef struct {
    spiffs_manager_config_t config;
    SemaphoreHandle_t mutex;
} spiffs_manager_handle_t;

/*
    @brief Initiates and configures the spiffs to function
*/
esp_err_t spiffs_manager_init(spiffs_manager_handle_t *spiffs_obj, const spiffs_manager_config_t *config);

/*
    @brief Format a specified file
*/
esp_err_t spiffs_manager_format(spiffs_manager_handle_t *spiffs_obj, const char *file_path);

/*
    @brief Records data in the spiffs partition
*/
esp_err_t spiffs_manager_write_file(spiffs_manager_handle_t *spiffs_obj, const char *file_path, char record_mode, const char *data);

/*
    @brief Reads data in the spiffs partition
*/
esp_err_t spiffs_manager_read_file(spiffs_manager_handle_t *spiffs_obj, const char *file_path);

/*
    @brief Initiates and configures the spiffs to function
*/
esp_err_t spiffs_manager_delete_file(spiffs_manager_handle_t *spiffs_obj, const char *file_path);

/*
    @brief Initiates and configures the spiffs to function
*/
esp_err_t spiffs_manager_exists(spiffs_manager_handle_t *spiffs_obj, const char *file_path);

/*
    @brief Desactivate the memory and frees space
*/
esp_err_t spiffs_manager_deinit(spiffs_manager_handle_t *spiffs_obj);

#ifdef __cplusplus
}   
#endif

#endif // SPIFFS_MANAGER