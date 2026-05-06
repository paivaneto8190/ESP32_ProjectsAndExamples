#ifndef SD_MANAGER
#define SD_MANAGER

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/sdspi_host.h"
#include <esp_vfs_fat.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdmmc_cmd.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    gpio_num_t mosi_pin; // Master out - slave in PIN
    gpio_num_t miso_pin; // Master in - slave out PIN
    gpio_num_t cs_pin;   // Chip select PIN
    gpio_num_t clk_pin;  // Clock line

    bool format_if_mount_failed;
    const char *base_path;

    uint16_t cluster_size; // Tamanho dos cluster a serem configurados

} sd_manager_config_t;

typedef struct {
    sd_manager_config_t config;
    SemaphoreHandle_t mutex;
} sd_manager_handle_t;

/*
    @brief Initiates and configures the sd_manager driver and protocol to function
*/
esp_err_t sd_manager_init(sd_manager_handle_t *sd_obj, const sd_manager_config_t *config, sdmmc_host_t *host,sdmmc_card_t *card);

/*
    @brief Formats the specified file
*/
esp_err_t sd_manager_format(sd_manager_handle_t *sd_obj, const char *file_name);

/*
    @brief Write in a specified file according with the record_mode parameter
*/
esp_err_t sd_manager_write_file(sd_manager_handle_t *sd_obj, const char *file_name, const char *record_mode, const char *data);

/*
    @brief Read a specified file
*/
esp_err_t sd_manager_read_file(sd_manager_handle_t *sd_obj, const char *file_name);

/*
    @brief Delets the specified file
*/
esp_err_t sd_manager_delete_file(sd_manager_handle_t *sd_obj, const char *file_name);

/*
    @brief Check if a specific files exists
*/
esp_err_t sd_manager_exists(sd_manager_handle_t *sd_obj, const char *file_name);

/*
    @brief Deactivate the SD and frees the memory
*/
esp_err_t sd_manager_deinit(sd_manager_handle_t *sd_obj, sdmmc_card_t *card);

#ifdef __cplusplus
}   
#endif

#endif // sd_manager