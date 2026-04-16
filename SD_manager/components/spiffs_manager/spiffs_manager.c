#include <stdio.h>
#include "spiffs_manager.h"

#define FORMAT_FILE_SYSTEM true
#define WAIT_TIME_OPERATE 2000

static const char *TAG = "[SPIFFS]";

esp_err_t spiffs_init(spiffs_manager_handle_t *spiffs_obj, const spiffs_manager_config_t *config)
{
    if (!spiffs_obj || !config)
    {
        ESP_LOGE(TAG, "Parâmetros inválidos ou vazios");
        return ESP_ERR_INVALID_ARG;
    }

    spiffs_obj->config = *config;
    spiffs_obj->mutex = xSemaphoreCreateMutex();

    if (spiffs_obj->mutex == NULL)
    {
        ESP_LOGE(TAG, "Erro ao criar mutex");
        return ESP_FAIL;
    }

    esp_vfs_spiffs_conf_t conf = {
        .base_path = spiffs_obj->config.base_path,
        .partition_label = spiffs_obj->config.partition_label,
        .max_files = spiffs_obj->config.max_files,
        .format_if_mount_failed = FORMAT_FILE_SYSTEM
    };

    esp_err_t err = esp_vfs_spiffs_register (&conf);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Falha ao inicializar SPIFFS: %s", esp_err_to_name(err));
        vSemaphoreDelete(spiffs_obj->mutex);
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t spiffs_write(spiffs_manager_handle_t *spiffs_obj, const char *file_path, char record_mode)
{
    if (!spiffs_obj)
    {
        ESP_LOGE(TAG, "Parâmetros inválidos ou vazios");
        return ESP_ERR_INVALID_ARG;
    }

    BaseType_t mutex_taken = xSemaphoreTake(spiffs_obj->mutex, pdMS_TO_TICKS(WAIT_TIME_OPERATE));
    if (mutex_taken == pdFALSE)
    {
        ESP_LOGE(TAG, "Falha ao tomar mutex para escrita");
        return ESP_ERR_TIMEOUT;
    } 

    if (record_mode != 'a' && record_mode != 'w')
    {
        ESP_LOGE(TAG, "Modo de escrita inválido!");
        xSemaphoreGive(spiffs_obj->mutex);
        return ESP_ERR_INVALID_ARG;
    }

    FILE* f = fopen(file_path, record_mode);

    if (f == NULL) {
        ESP_LOGE(TAG, "Falha ao abrir arquivo para escrita");
        xSemaphoreGive(spiffs_obj->mutex);
        return ESP_FAIL;
    }

    fprintf(f, "Gravou\n");
    uint8_t res = fclose(f);
    if (res != 0)
    {
        ESP_LOGE(TAG, "Erro ao fechar o arquivo! Dados podem ter sido perdidos.");
        xSemaphoreGive(spiffs_obj->mutex);
        return ESP_FAIL;
    }

    xSemaphoreGive(spiffs_obj->mutex);
    return ESP_OK;
}

esp_err_t spiffs_read(spiffs_manager_handle_t *spiffs_obj, const char *file_path)
{
    if (!spiffs_obj)
    {
        ESP_LOGE(TAG, "Parâmetros inválidos ou vazios");
        return ESP_ERR_INVALID_ARG;
    }

    BaseType_t mutex_taken = xSemaphoreTake(spiffs_obj->mutex, pdMS_TO_TICKS(WAIT_TIME_OPERATE));
    if (mutex_taken == pdFALSE)
    {
        ESP_LOGE(TAG, "Falha ao tomar mutex para leitura");
        return ESP_ERR_TIMEOUT;
    } 

    FILE* f = fopen(file_path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Falha ao abrir arquivo para leitura");
        return ESP_FAIL;
    }

    char linha[64];

    while (fgets(linha, sizeof(linha), f) != NULL) {
        if (sscanf(linha, "Gravou\n") == 0) {
            ESP_LOGI(TAG, "Linha lida: %s", linha);
            //leu_algo = true;
        }
    }

    uint8_t res = fclose(f);

    if (res != 0) {
        ESP_LOGE(TAG, "Falha ao fechar arquivo");
        xSemaphoreGive(spiffs_obj->mutex);
        return ESP_FAIL; // Falha ao fechar o arquivo
    }

    xSemaphoreGive(spiffs_obj->mutex);
    return ESP_OK;
}

esp_err_t spiffs_deinit(spiffs_manager_handle_t *spiffs_obj)
{
    if (!spiffs_obj)
    {
        ESP_LOGE(TAG, "Parâmetros inválidos ou vazios");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = esp_vfs_spiffs_unregister(spiffs_obj->config.partition_label);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Erro ao desmontar SPIFFS: %s", esp_err_to_name(err));
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "SPIFFS desmontada com sucesso.");

    //Deleta o mutex e libera o espaço
    vSemaphoreDelete(spiffs_obj->mutex);
    if (spiffs_obj == NULL) 
    {
        ESP_LOGE(TAG, "Erro ao deletar mutex!");
        return ESP_FAIL;
    }

    // Libera espaço alocado para o handle
    vPortFree(spiffs_obj);

    ESP_LOGI(TAG, "SPIFFS desinicializada com sucesso!");
    return ESP_OK;
}