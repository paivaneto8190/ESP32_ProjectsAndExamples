#include <stdio.h>
#include "sd_manager.h"
//#include <esp_vfs_fat.h>

#define MOSI_PIN
#define MISO_PIN
#define CS_PIN
#define CLK_PIN
#define FORMAT_IF_MOUNT_FAILED true
#define BASE_PATH  "/sdcard"
#define CLUSTER_SIZE 16
#define WAIT_TIME_OPERATE 2000 //2 segundos para esperar pelo semáforo

static const char *TAG = "[SD]";

esp_err_t sd_manager_init(sd_manager_handle_t *sd_obj, const sd_manager_config_t *config, sdmmc_card_t *card)
{
    if (!sd_obj || !config)
    {
        ESP_LOGE(TAG, "Parâmetros inválidos ou vazios!");
        return ESP_ERR_INVALID_ARG;
    }

    if (!GPIO_IS_VALID_GPIO(config->mosi_pin) || !GPIO_IS_VALID_GPIO(config->miso_pin) || !GPIO_IS_VALID_GPIO(config->cs_pin) || !GPIO_IS_VALID_GPIO(config->clk_pin))
    {
        ESP_LOGE(TAG, "Pinos inválidos ou já conectados!");
        return ESP_ERR_INVALID_ARG;
    }

    sd_obj->config = *config;
    sd_obj->mutex = xSemaphoreCreateMutex();
    if (sd_obj->mutex == NULL) // Verifica se o semáforo foi criado corretamente
    {
        ESP_LOGE(TAG, "Erro ao criar mutex!");
        return ESP_FAIL;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = sd_obj->config.cs_pin;

    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = FORMAT_IF_MOUNT_FAILED,
        .max_files = 5,
        .allocation_unit_size = CLUSTER_SIZE * 1024
    };

    esp_err_t err = esp_vfs_fat_sdspi_mount(BASE_PATH , &sd_obj->config.host, &slot_config , &mount_config, &card);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Montagem falhou: %s", esp_err_to_name(err));
        vSemaphoreDelete(sd_obj->mutex);
        return err;
    }

    ESP_LOGI(TAG, "Cartão SD inicializado com sucesso!");
    return ESP_OK;
}

esp_err_t sd_manager_format(sd_manager_handle_t *sd_obj, const char *file_name)
{
        if (!sd_obj)
    {
        ESP_LOGE(TAG, "Parâmetro inválido ou vazio!");
        return ESP_ERR_INVALID_ARG;
    }

    BaseType_t mutex_taken = xSemaphoreTake(sd_obj->mutex, pdMS_TO_TICKS(WAIT_TIME_OPERATE));
    if (mutex_taken == pdFALSE)
    {
        ESP_LOGE(TAG, "Falha ao tomar mutex para escrita");
        return ESP_ERR_TIMEOUT;
    } 

    FILE *f = fopen(file_name, "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Falha ao abrir arquivo para formatação");
        xSemaphoreGive(sd_obj->mutex);
        return ESP_FAIL;
    }

    //fprintf(f, "%s\n", data);
    fclose(f);

    ESP_LOGI(TAG, "Arquivo formatado com sucesso!");

    xSemaphoreGive(sd_obj->mutex);
    return ESP_OK;
}

esp_err_t sd_manager_write_file(sd_manager_handle_t *sd_obj, const char *file_name, const char *record_mode, const char *data)
{
    if (!sd_obj)
    {
        ESP_LOGE(TAG, "Parâmetro inválido ou vazio!");
        return ESP_ERR_INVALID_ARG;
    }

    BaseType_t mutex_taken = xSemaphoreTake(sd_obj->mutex, pdMS_TO_TICKS(WAIT_TIME_OPERATE));
    if (mutex_taken == pdFALSE)
    {
        ESP_LOGE(TAG, "Falha ao tomar mutex para escrita");
        return ESP_ERR_TIMEOUT;
    } 

    if (strcmp(record_mode, "a") != 0 && strcmp(record_mode, "w") != 0)
    {
        ESP_LOGE(TAG, "Modo de escrita inválido!");
        xSemaphoreGive(sd_obj->mutex);
        return ESP_ERR_INVALID_ARG;
    }

    FILE *f = fopen(file_name, record_mode);
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Falha ao abrir arquivo para escrita");
        xSemaphoreGive(sd_obj->mutex);
        return ESP_FAIL;
    }

    fprintf(f, "%s\n", data);
    fclose(f);

    ESP_LOGI(TAG, "Escrita bem sucedida");

    xSemaphoreGive(sd_obj->mutex);
    return ESP_OK;
}

esp_err_t sd_manager_read_file(sd_manager_handle_t *sd_obj, const char *file_name)
{
    if (!sd_obj)
    {
        ESP_LOGE(TAG, "Parâmetro inválido ou vazio!");
        return ESP_ERR_INVALID_ARG;
    }

    BaseType_t mutex_taken = xSemaphoreTake(sd_obj->mutex, pdMS_TO_TICKS(WAIT_TIME_OPERATE));
    if (mutex_taken == pdFALSE)
    {
        ESP_LOGE(TAG, "Falha ao tomar mutex para escrita");
        return ESP_ERR_TIMEOUT;
    } 

    FILE *f = fopen(file_name, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Falha ao abrir o arquivo para leitura");
        xSemaphoreGive(sd_obj->mutex);
        return ESP_FAIL;
    }

    char linha[64]; // Buffer para ler linha por linha
    while (fgets(linha, sizeof(linha), f) != NULL) {
        // Remove o caractere de nova linha (\n) se desejar um log mais limpo
        size_t len = strlen(linha);
        if (len > 0 && linha[len - 1] == '\n') {
            linha[len - 1] = '\0';
        }
        
        ESP_LOGI(TAG, "CONTEÚDO: %s", linha);
    }

    fclose(f);
    ESP_LOGI(TAG, "Fim da leitura do arquivo.");

    xSemaphoreGive(sd_obj->mutex);
    return ESP_OK;
}

esp_err_t sd_manager_delete_file(sd_manager_handle_t *sd_obj, const char *file_name)
{
    if (!sd_obj)
    {
        ESP_LOGE(TAG, "Parâmetro inválido ou vazio!");
        return ESP_ERR_INVALID_ARG;
    }

    BaseType_t mutex_taken = xSemaphoreTake(sd_obj->mutex, pdMS_TO_TICKS(WAIT_TIME_OPERATE));
    if (mutex_taken == pdFALSE)
    {
        ESP_LOGE(TAG, "Falha ao tomar mutex para escrita");
        return ESP_ERR_TIMEOUT;
    } 

    // Verifica se o arquivo existe antes de tentar deletar
    struct stat st;
    if (stat(file_name, &st) != 0) {
        ESP_LOGE(TAG, "Arquivo %s não encontrado.", file_name);
        xSemaphoreGive(sd_obj->mutex);
        return ESP_ERR_NOT_FOUND;
    }

    if (unlink(file_name) == 0) {
        ESP_LOGI(TAG, "Arquivo deletado com sucesso!");
        xSemaphoreGive(sd_obj->mutex);
        return ESP_OK;
    } else {
        ESP_LOGE(TAG, "Falha ao deletar o arquivo.");
        xSemaphoreGive(sd_obj->mutex);
        return ESP_FAIL;
    }
}

esp_err_t sd_manager_deinit(sd_manager_handle_t *sd_obj, const char *file_name, sdmmc_card_t *card)
{
    if (!sd_obj)
    {
        return ESP_ERR_INVALID_ARG;
    }

    vSemaphoreDelete(sd_obj->mutex);

    esp_err_t err = esp_vfs_fat_sdcard_unmount(BASE_PATH, card);
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao desmontar o VFS: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "Cartão SD desmontado e driver liberado.");
    return ESP_OK;
}