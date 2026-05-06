#include <stdio.h>
#include "sd_manager.h"
#include "spiffs_manager.h"

#define SD_PATH "/sdcard/log.csv"
#define SPIFFS_PATH "/spiffs/backlog.csv"
#define PIN_NUM_MISO GPIO_NUM_19
#define PIN_NUM_MOSI GPIO_NUM_23
#define PIN_NUM_CLK  GPIO_NUM_18
#define PIN_NUM_CS   GPIO_NUM_5

static const char *TAG = "[MAIN]";
bool SD_pronto = false;

// Configuração inicial módulo SPIFFS
spiffs_manager_config_t spiffs_config = {
    .base_path = "/spiffs",
    .partition_label = "spiffs",
    .max_files = 5,
};

spiffs_manager_handle_t spiffs_handle;

// Configuração inicial módulo SD
sd_manager_config_t sd_config = {
    .mosi_pin = PIN_NUM_MOSI,
    .miso_pin = PIN_NUM_MISO,
    .cs_pin = PIN_NUM_CS,
    .clk_pin = PIN_NUM_CLK,
    .format_if_mount_failed = true,
    .base_path = "/sd",
    .cluster_size = 512,
};


sd_manager_handle_t sd_handle;

sdmmc_card_t card;

void app_main(void)
{
    esp_err_t err = spiffs_manager_init(&spiffs_handle, &spiffs_config);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPIFFS manager: %s", esp_err_to_name(err));
        ESP_LOGE(TAG, "Encerrando aplicação...");
        //DESINICILIZAR SPIFFS AQUI
        vTaskDelete(NULL);
        return;
    }
    else
    {
        ESP_LOGI(TAG, "SPIFFS manager initialized successfully.");
    }

    // Configuração do barramento SPI
    spi_bus_config_t bus_cfg = {
    .mosi_io_num = PIN_NUM_MOSI,
    .miso_io_num = PIN_NUM_MISO,
    .sclk_io_num = PIN_NUM_CLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 4000,
    };

    // Inicializa o barramento SPI (usando o SPI2_HOST, também conhecido como HSPI)
    err = spi_bus_initialize(SPI2_HOST, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Falha ao inicializar o barramento SPI.\n");
        return;
    }
    else{
        ESP_LOGI(TAG, "Barramento SPI inicializado com sucesso.\n");
    }

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    err = sd_manager_init(&sd_handle, &sd_config, &host, &card);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SD manager: %s", esp_err_to_name(err));
        ESP_LOGE(TAG, "Encerrando aplicação...");
        //DESINICILIZAR SD AQUI
        vTaskDelete(NULL);
        return;
    }
    else
    {
        ESP_LOGI(TAG, "SD manager initialized successfully.");
        SD_pronto = true;
    }

    // Cria o arquivo caso não exista
    err = sd_manager_exists(&sd_handle, SD_PATH);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Arquivo %s não encontrado. Criando arquivo...", SD_PATH);
        err = sd_manager_write_file(&sd_handle, SD_PATH, "w", "Criando arquivo");
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to create file on SD card: %s", esp_err_to_name(err));
            ESP_LOGE(TAG, "Encerrando aplicação...");
            vTaskDelete(NULL);
            return;
        }
    }
    else
    {
        ESP_LOGI(TAG, "Arquivo %s encontrado.", SD_PATH);
    }

    while (1)
    {
        if (SD_pronto == true)
        {
        }
        
        err = sd_manager_write_file(&sd_handle, SD_PATH, "a", "Log de teste");
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to write to SD card: %s", esp_err_to_name(err));
            err = spiffs_manager_write_file(&spiffs_handle, SPIFFS_PATH, "a", "Log de teste");

            sd_manager_deinit(&sd_handle, &card); // Desinicializa o SD para evitar mais falhas

            if (err != ESP_OK) 
            {
                ESP_LOGE(TAG, "Failed to write to SPIFFS: %s", esp_err_to_name(err));
                break;
            }
            else {
                ESP_LOGI(TAG, "Log escrito no SPIFFS como backup.");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5000)); // Aguarda 5 segundos antes de escrever novamente
        
        err = sd_manager_read_file(&sd_handle, SD_PATH);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to read from SD card: %s", esp_err_to_name(err));
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(5000)); // Aguarda 5 segundos antes de ler novamente
    }

    //Desinicializa os módulos e libera os recursos
    spiffs_manager_deinit(&spiffs_handle);
    sd_manager_deinit(&sd_handle, &card);

    ESP_LOGI(TAG, "Encerrando aplicação...");
    vTaskDelete(NULL);
    return;
}