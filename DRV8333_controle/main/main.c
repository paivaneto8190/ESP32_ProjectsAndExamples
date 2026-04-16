#include <stdio.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "drv8833.h"
#include "esp_log.h"

#define A1_PIN 4
#define A2_PIN 5
#define B1_PIN 6
#define B2_PIN 7

#define CHANNEL_A1 LEDC_CHANNEL_0
#define CHANNEL_A2 LEDC_CHANNEL_1
#define CHANNEL_B1 LEDC_CHANNEL_2
#define CHANNEL_B2 LEDC_CHANNEL_3

#define LEDC_DUTY_RESOLUTION 8

#define LEDC_TIMER LEDC_TIMER_0

drv8833_config_t drv_config = {
    .motor_pinA1 = A1_PIN,
    .motor_pinA2 = A2_PIN,
    .motor_pinB1 = B1_PIN,
    .motor_pinB2 = B2_PIN,

    .channel_A1 = CHANNEL_A1,
    .channel_A2 = CHANNEL_A2,
    .channel_B1 = CHANNEL_B1,
    .channel_B2 = CHANNEL_B2,

    .timer_num = LEDC_TIMER,

    .duty_resolution = LEDC_DUTY_RESOLUTION,

};

drv8833_t drv;


void app_main(void)
{
    esp_err_t err = drv8833_init(&drv, &drv_config);

    if (err != ESP_OK)
    {
        ESP_LOGE("[DRV8833]", "NÃO INICIALIZADO!");
    }

    while (1)
    {
        err = drv8833_set_motor_A(&drv, 255);

        if (err != ESP_OK)
        {
            ESP_LOGE("[DRV8833]", "NÃO SETOU VELOCIDADE MOTOR A!");
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(5000));

        err = drv8833_set_motor_A (&drv, -255);

        if (err != ESP_OK)
        {
            ESP_LOGE("[DRV8833]", "NÃO SETOU VELOCIDADE MOTOR B!");
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }


    drv8833_deinit(&drv);
    vTaskDelete(NULL);
    return;
}
