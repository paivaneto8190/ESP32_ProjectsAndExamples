#include <stdio.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "led_rgb.h"
#include "esp_log.h"

#define GREEN_PIN_GPIO GPIO_NUM_12
#define BLUE_PIN_GPIO GPIO_NUM_11
#define RED_PIN_GPIO GPIO_NUM_10

#define RED_LEDC_CHANNEL LEDC_CHANNEL_0
#define GREEN_LEDC_CHANNEL LEDC_CHANNEL_1
#define BLUE_LEDC_CHANNEL LEDC_CHANNEL_2

#define LEDC_DUTY_RESOLUTION 8 // bits

#define LEDC_TIMER LEDC_TIMER_0

led_rgb_config_t led_config = {
    .red_pin = RED_PIN_GPIO,
    .blue_pin = BLUE_PIN_GPIO,
    .green_pin = GREEN_PIN_GPIO,

    .red_channel = RED_LEDC_CHANNEL,
    .green_channel = GREEN_LEDC_CHANNEL,
    .blue_channel = BLUE_LEDC_CHANNEL,

    .timer_num = LEDC_TIMER,
    .duty_resolution = LEDC_DUTY_RESOLUTION,

};

led_rgb_t led_rgb_handle;

void app_main(void)
{
    esp_err_t err = led_rgb_init(led_rgb_handle, led_config);

    if (err != ESP_OK)
    {
        ESP_LOGE("[LED_RGB]", "NÃO INICIALIZADO!");
    }

    while (1)
    {
        err = led_rgb_set_color(led_rgb_handle, 255, 255, 255);
        if (err != ESP_OK)
        {
            ESP_LOGE("[LED_RGB]", "NÃO SETOU em 1!");
            break;
        }

        vTaskDelay(pdMS_TO_TICK(500));

        err = led_rgb_set_color(led_rgb_handle, 0, 0, 0);
        if (err != ESP_OK)
        {
            ESP_LOGE("[LED_RGB]", "NÃO SETOU em 0!");
            break;
        }
        vTaskDelay(pdMS_TO_TICK(500));
    }

    led_deinit(led_rgb_handle);
    vTaskDelete(NULL);
    return;
}
