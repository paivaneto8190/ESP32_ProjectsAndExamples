#include <stdio.h>
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "nvs.h"

#define INTERVAL 10000000 //Intervalo de reset da tarefa

static const char *TAG = "SISTEMA_NVS";
int64_t last_exec = 0;
int64_t now_exec = 0;


void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    nvs_handle_t handle;
    uint32_t err_count;

    nvs_open("system" , NVS_READWRITE, &handle);

    err = nvs_get_u32 (handle, "err_count", &err_count);

    if ( err == ESP_ERR_NVS_NOT_FOUND) {
        printf ("Primeira execucao! Criando chave ...\n") ;
    }

    err_count++;

    nvs_set_u32 (handle, "err_count", err_count) ;
    nvs_commit(handle);
    nvs_close(handle);

    ESP_LOGI(TAG, "O valor atual de err_count e: %lu", err_count);

    while(1)
    {
        now_exec = esp_timer_get_time();
        if ((now_exec - last_exec) >= 10000000 && err_count < 3)
        {
            nvs_open("system" , NVS_READWRITE, &handle);

            esp_err_t err = nvs_get_u32 (handle, "err_count", &err_count);

            if (err != ESP_OK) {
                break;
            }

            err_count = 0;
            nvs_set_u32 (handle, "err_count", err_count) ;
            nvs_commit(handle);
            nvs_close (handle);
        }
        else
        {
            nvs_open("system" , NVS_READWRITE, &handle);

            esp_err_t err = nvs_get_u32 (handle, "err_count", &err_count);

            if (err != ESP_OK) {
                break;
            }

            if (err_count >= 3)
            {
                err_count = 0;
                nvs_set_u32 (handle, "err_count", err_count) ;
                nvs_commit(handle);
                nvs_close (handle);

                printf("SISTEMA BLOQUEADO!\n");
                while(1)
                {
                    vTaskDelay(pdMS_TO_TICKS(100));
                };
            }
            else
            {
                nvs_commit(handle);
                nvs_close (handle);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100)); 
    }

    return;
}
