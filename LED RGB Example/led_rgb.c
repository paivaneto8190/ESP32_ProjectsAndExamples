#include <stdio.h>
#include "led_rgb.h"

#define LEDC_FREQ 5000

// INSERIR LOGS PARA DEBUGAR E INFORMAR ERROS E TALS

esp_err_t led_rgb_init(led_rgb_t *led, const led_rgb_config_t *config)
{
    if (!led || !config)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (!GPIO_IS_VALID_GPIO(config->red_pin) || !GPIO_IS_VALID_GPIO(config->blue_pin) || !GPIO_IS_VALID_GPIO(config->green_pin))
    {
        return ESP_ERR_INVALID_ARG;
    }

    // verificar canais tbm XP

    led->config = *config;

    led->mutex = xSemaphoreCreateMutex();

    if (led->mutex == NULL)
    {
        return ESP_FAIL;
    }

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = led->config.duty_resolution,
        .timer_num = led->config.timer_num,
        .freq_hz = LEDC_FREQ,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    esp_err_t err = ledc_timer_config(&ledc_timer);

    if (err != ESP_OK)
    {
        vSemaphoreDelete(led->mutex);
        return err;
    }

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = led->config.timer_num,
        .duty = 0, // Set duty to 0%
        .hpoint = 0,
    };

    ledc_channel.channel = led->config.red_channel;
    ledc_channel.gpio_num = led->config.red_pin;

    err = ledc_channel_config(&ledc_channel);
    if (err != ESP_OK)
    {
        vSemaphoreDelete(led->mutex);
        return err;
    }
    ledc_channel.channel = led->config.blue_channel;
    ledc_channel.gpio_num = led->config.blue_pin;

    err = ledc_channel_config(&ledc_channel);
    if (err != ESP_OK)
    {
        vSemaphoreDelete(led->mutex);
        return err;
    }
    ledc_channel.channel = led->config.green_channel;
    ledc_channel.gpio_num = led->config.green_pin;

    err = ledc_channel_config(&ledc_channel);
    if (err != ESP_OK)
    {
        vSemaphoreDelete(led->mutex);
        return err;
    }

    return ESP_OK;
}

esp_err_t led_rgb_set_color(led_rgb_t *led, uint32_t red, uint32_t green, uint32_t blue)
{

    if (!led)
    {
        return ESP_ERR_INVALID_ARG;
    }
    // verificar os valores RGB

    // verificar se pegou de fato o mutex ou se deu erro (definir timeout)
    xSemaphoreTake(led->mutex, portMAX_DELAY);

    esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, led->config.red_channel, red);
    if (err != ESP_OK)
    {
        xSemaphoreGive(led->mutex);
        return err;
    }
    err = ledc_update_duty(LEDC_LOW_SPEED_MODE, led->config.red_channel);
    if (err != ESP_OK)
    {
        xSemaphoreGive(led->mutex);
        return err;
    }

    err = ledc_set_duty(LEDC_LOW_SPEED_MODE, led->config.green_channel, green);
    if (err != ESP_OK)
    {
        xSemaphoreGive(led->mutex);
        return err;
    }
    err = ledc_update_duty(LEDC_LOW_SPEED_MODE, led->config.green_channel);
    if (err != ESP_OK)
    {
        xSemaphoreGive(led->mutex);
        return err;
    }

    err = ledc_set_duty(LEDC_LOW_SPEED_MODE, led->config.blue_channel, blue);
    if (err != ESP_OK)
    {
        xSemaphoreGive(led->mutex);
        return err;
    }
    err = ledc_update_duty(LEDC_LOW_SPEED_MODE, led->config.blue_channel);
    if (err != ESP_OK)
    {
        xSemaphoreGive(led->mutex);
        return err;
    }

    xSemaphoreGive(led->mutex);
    return ESP_OK;
}

esp_err_t led_deinit(led_rgb_t *led)
{
    if (!led)
    {
        return ESP_ERR_INVALID_ARG;
    }
    // verificar se foi deletado;
    vSemaphoreDelete(led->mutex);

    esp_err_t err = ledc_stop(LEDC_LOW_SPEED_MODE, led->config.green_channel, 0);
    if (err != ESP_OK)
    {
        return err;
    }

    err = ledc_stop(LEDC_LOW_SPEED_MODE, led->config.red_channel, 0);
    if (err != ESP_OK)
    {
        return err;
    }

    err = ledc_stop(LEDC_LOW_SPEED_MODE, led->config.blue_channel, 0);
    if (err != ESP_OK)
    {
        return err;
    }
    //desalocar config tbm =O

    return ESP_OK;
}