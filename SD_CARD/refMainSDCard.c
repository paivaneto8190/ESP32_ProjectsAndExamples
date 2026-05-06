#include <stdio.h>
#include <string.h>
#include "sd_manager.h"
#include "spiffs_manager.h"

#define SD_PATH "/sd/log.csv"
#define SPIFFS_PATH "/spiffs/backlog.csv"

static const char *TAG = "[MAIN]";

bool sd_disponivel = false;

spiffs_manager_handle_t spiffs_handle;
sd_manager_handle_t sd_handle;
sdmmc_card_t card;

/* =========================
   TESTA / REINICIALIZA SD
   ========================= */
bool tentar_reiniciar_sd()
{
    esp_err_t err;

    ESP_LOGI(TAG, "Tentando reinicializar SD...");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    err = sd_manager_init(&sd_handle, NULL, &host, &card);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "SD reinicializado com sucesso!");
        return true;
    }

    ESP_LOGW(TAG, "SD ainda indisponível.");
    return false;
}

/* =========================
   TRANSFERE BACKLOG
   ========================= */
void transferir_backlog()
{
    ESP_LOGI(TAG, "Transferindo backlog do SPIFFS para SD...");

    FILE *f_spiffs = fopen(SPIFFS_PATH, "r");
    if (!f_spiffs)
    {
        ESP_LOGI(TAG, "Nenhum backlog encontrado.");
        return;
    }

    FILE *f_sd = fopen(SD_PATH, "a");
    if (!f_sd)
    {
        ESP_LOGE(TAG, "Erro ao abrir SD para escrita.");
        fclose(f_spiffs);
        return;
    }

    char buffer[128];

    while (fgets(buffer, sizeof(buffer), f_spiffs))
    {
        fputs(buffer, f_sd);
    }

    fclose(f_spiffs);
    fclose(f_sd);

    // Apaga backlog
    remove(SPIFFS_PATH);

    ESP_LOGI(TAG, "Backlog transferido e apagado com sucesso.");
}

/* =========================
   APP MAIN
   ========================= */
void app_main(void)
{
    esp_err_t err;

    // Inicializa SPIFFS
    spiffs_manager_config_t spiffs_config = {
        .base_path = "/spiffs",
        .partition_label = "spiffs",
        .max_files = 5,
    };

    err = spiffs_manager_init(&spiffs_handle, &spiffs_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Erro ao iniciar SPIFFS");
        return;
    }

    // Inicializa SD (primeira tentativa)
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    err = sd_manager_init(&sd_handle, NULL, &host, &card);

    if (err == ESP_OK)
    {
        sd_disponivel = true;
        ESP_LOGI(TAG, "SD disponível no boot");
    }
    else
    {
        sd_disponivel = false;
        ESP_LOGW(TAG, "SD não disponível no boot");
    }

    while (1)
    {
        /* =========================
           CASO SD ESTEJA OK
           ========================= */
        if (sd_disponivel)
        {
            err = sd_manager_write_file(&sd_handle, SD_PATH, "a", "Log de teste\n");

            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Falha no SD → entrando em FAILOVER");

                sd_disponivel = false;
                sd_manager_deinit(&sd_handle, &card);
            }
        }
        else
        {
            /* =========================
               FAILOVER → SPIFFS
               ========================= */
            err = spiffs_manager_write_file(&spiffs_handle, SPIFFS_PATH, "a", "Log de teste\n");

            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Erro ao escrever no SPIFFS!");
            }
            else
            {
                ESP_LOGI(TAG, "Gravando no SPIFFS (backup)");
            }

            /* =========================
               TENTA RECUPERAR SD
               ========================= */
            if (tentar_reiniciar_sd())
            {
                sd_disponivel = true;

                /* =========================
                   SINCRONIZA BACKLOG
                   ========================= */
                transferir_backlog();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}