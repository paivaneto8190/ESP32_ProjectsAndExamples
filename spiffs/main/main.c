#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_spiffs.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_random.h"
#include "esp_log.h"

static const char *TAG = "[SPIFFS]";
static const char *TAG_NVS = "[NVS]";

esp_err_t record_spiffs (uint32_t valTemp, uint32_t contador)
{
    ESP_LOGI(TAG, "Abrindo arquivo para escrita...");
    FILE* f = fopen("/spiffs/log.txt", "a");

    if (f == NULL) {
        ESP_LOGE(TAG, "Falha ao abrir arquivo para escrita");

        return ESP_FAIL;
    }
    fprintf(f, "%lu - Temp. sensor: %lu\n", contador, valTemp);
    uint8_t res = fclose(f);

    if (res != 0)
    {
        ESP_LOGE(TAG, "Erro ao fechar o arquivo! Dados podem ter sido perdidos.");

        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Arquivo fechado com sucesso.");
    return ESP_OK;
}

esp_err_t read_spiffs(void)
{
    ESP_LOGI(TAG, "Abrindo arquivo para leitura...");
    FILE* f = fopen("/spiffs/log.txt", "r");

    if (f == NULL) {
        ESP_LOGE(TAG, "Falha ao abrir arquivo para leitura");
        return ESP_FAIL;
    }

    char linha[64];
    uint32_t contador;
    uint32_t value_read;
    bool leu_algo = false;

    while (fgets(linha, sizeof(linha), f) != NULL) {
        if (sscanf(linha, "%lu - Temp. sensor: %lu\n", &contador, &value_read) == 2) {
            ESP_LOGI(TAG, "%lu - Temp. sensor: %lu", contador, value_read);
            leu_algo = true;
        }
    }
    uint8_t res = fclose(f);

    if (res != 0) {
        ESP_LOGI(TAG, "Leitura falhou");
        return ESP_FAIL; // Falha ao fechar o arquivo
    }

    if (leu_algo != true)
    {
        ESP_LOGI(TAG, "Leitura passou");
        return ESP_ERR_NOT_FINISHED;
    }

    return ESP_OK;
}

void app_main(void)
{
    uint32_t err_count = 0;
    uint32_t err_count_aux = 0;

    // Inicializa NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG_NVS, "Falha ao inicializar NVS: %s", esp_err_to_name(err));
        return;
    }

    nvs_handle_t handle;
    err = nvs_open("system" , NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG_NVS, "Falha ao abrir NVS: %s", esp_err_to_name(err));
        return;
    }

    err = nvs_get_u32 (handle, "err_count", &err_count);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_NVS, "Falha ao obter valor do contador de inicializacao: %s", esp_err_to_name(err));
        return;
    }
    
    nvs_commit(handle);
    nvs_close (handle);

    // Configura início da SPIFFS
    esp_vfs_spiffs_conf_t conf = {
    .base_path = "/spiffs" ,
    . partition_label = "spiffs",
    . max_files = 5,
    . format_if_mount_failed = true
    };
    err = esp_vfs_spiffs_register (&conf);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Falha ao inicializar SPIFFS: %s", esp_err_to_name(err));
        return;
    }
    
    while(1)
    {

        err = nvs_open("system" , NVS_READWRITE, &handle);

        if (err != ESP_OK)
        {
            ESP_LOGE(TAG_NVS, "Falha ao abrir NVS: %s", esp_err_to_name(err));
            return;
        }

        err = nvs_get_u32 (handle, "err_count", &err_count);

        err_count_aux = err_count + 1;

        nvs_set_u32 (handle, "err_count", err_count_aux) ;
        nvs_commit(handle);
        nvs_close(handle);

        ESP_LOGI(TAG_NVS, "Valor de err_count e: %lu", err_count);

        uint32_t valTemp = esp_random() % 100;
        esp_err_t err = record_spiffs(valTemp, err_count);

        if (err != ESP_OK)
        {
            break;
        }

        err = read_spiffs();
        if (err != ESP_OK)
        {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(10000));
    }

    vTaskDelay(pdMS_TO_TICKS(100));
    return;
}