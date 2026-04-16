#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_timer.h"

#define NUM_MEDICOES 10000

SemaphoreHandle_t sem_tarefa1;
SemaphoreHandle_t sem_tarefa2;

uint64_t tempo_fim, tempo_inicio, tempo_total;
double tempo_medio_troca_us, percentual_kernel, periodo_tick_us;

// Tarefa 1
void tarefa_1(void *pvParameters) {
    //ESP_LOGI("Tarefa 1", "Iniciando tarefa 1 (Ping)");  
    // Pequeno atraso apenas para garantir que a Tarefa 2 já iniciou e está bloqueada esperando
    //vTaskDelay(10 / portTICK_PERIOD_MS);

    // Captura o tempo inicial
    tempo_inicio = esp_timer_get_time();

    // Laço para medir múltiplas vezes
    for (int i = 0; i < NUM_MEDICOES; i++) {
        xSemaphoreGive(sem_tarefa2); 
        xSemaphoreTake(sem_tarefa1, portMAX_DELAY);
    }

    // Captura o tempo final
    tempo_fim = esp_timer_get_time();

    // --- CÁLCULOS ---
    tempo_total = tempo_fim - tempo_inicio;
    
    // Ocorrem 2 trocas por iteração (T1 -> T2 e T2 -> T1)
    tempo_medio_troca_us = (double)tempo_total / (NUM_MEDICOES * 2);

    // O FreeRTOS roda baseado em Ticks. Precisamos do tempo de 1 Tick em microsegundos
    periodo_tick_us = 1000000.0 / configTICK_RATE_HZ; 
    
    // O percentual é a relação entre o tempo de troca e o tempo do Tick
    percentual_kernel = (tempo_medio_troca_us / periodo_tick_us) * 100.0;

    // Exibe os resultados
    ESP_LOGI("Resultado", "Tempo total para %d iterações: %llu us", NUM_MEDICOES, tempo_total);
    ESP_LOGI("Resultado", "Tempo medio por troca de contexto: %.2f us", tempo_medio_troca_us);
    ESP_LOGI("Resultado", "Percentual de tempo do kernel: %.4f %%", percentual_kernel);

    //ESP_LOGI("Tarefa 1", "Tarefa 1 (Ping) finalizada");
    vTaskDelete(NULL);
}

// Tarefa 2 (Pong)
void tarefa_2(void *pvParameters) {
    //ESP_LOGI("Tarefa 2", "Iniciando tarefa 2 (Pong)");

    for(int i =0; i<NUM_MEDICOES; i++) {
        xSemaphoreTake(sem_tarefa2, portMAX_DELAY);
        xSemaphoreGive(sem_tarefa1);
    }

    //ESP_LOGI("Tarefa 2", "Tarefa 2 (Pong) finalizada");
    vTaskDelete(NULL);
}

void app_main(void) {
    // Cria os semáforos binários
    sem_tarefa1 = xSemaphoreCreateBinary();
    sem_tarefa2 = xSemaphoreCreateBinary();

    xTaskCreatePinnedToCore(tarefa_2, "Tarefa 2", 2048, NULL, 1, NULL, 1);  
    xTaskCreatePinnedToCore(tarefa_1, "Tarefa 1", 2048, NULL, 1, NULL, 1);
}