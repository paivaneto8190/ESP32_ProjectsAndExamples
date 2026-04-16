#include <stdio.h>
#include "drv8833.h"

#define MOTOR_FREQ 5000
#define MAX_SPEED 255

/*
* @brief Configuração inicial para os dois motores
*/

esp_err_t drv8833_init(drv8833_t *drv, const drv8833_config_t *config)
{
    if (!drv || !config) // Verifica se os objetos são válidos
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (!GPIO_IS_VALID_GPIO(config->motor_pinA1) || !GPIO_IS_VALID_GPIO(config->motor_pinA2) || !GPIO_IS_VALID_GPIO(config->motor_pinB1) || !GPIO_IS_VALID_GPIO(config->motor_pinB2))
    {
        return ESP_ERR_INVALID_ARG;
    }
    
    drv->config = *config;

    drv->mutex = xSemaphoreCreateMutex();

    if (drv->mutex == NULL) // Verifica se o semáforo foi criado corretamente
    {
        return ESP_FAIL;
    }
    
   // Configurar o PWM
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = drv->config.duty_resolution,
        .timer_num = drv->config.timer_num,
        .freq_hz = MOTOR_FREQ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    
    // Verificar configuração do timer
    esp_err_t err = ledc_timer_config(&ledc_timer);

    if (err != ESP_OK)
    {
        vSemaphoreDelete(drv->mutex);
        return err;
    }

    // Configurar o canal 1 do PWM
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = drv->config.timer_num,
        .duty = 0,
        .hpoint = 0,
    };

    ledc_channel.channel = drv->config.channel_A1;
    ledc_channel.gpio_num = drv->config.motor_pinA1;

    err = ledc_channel_config(&ledc_channel);
    if (err != ESP_OK)
    {
        vSemaphoreDelete(drv->mutex);
        return err;
    }

   // Configurar o canal 2 do PWM
    ledc_channel.channel = drv->config.channel_A2;
    ledc_channel.gpio_num = drv->config.motor_pinA2;

    err = ledc_channel_config(&ledc_channel);
    if (err != ESP_OK)
    {
        vSemaphoreDelete(drv->mutex);
        return err;
    }

    // Configurar o canal 3 do PWM
    ledc_channel.channel = drv->config.channel_B1;
    ledc_channel.gpio_num = drv->config.motor_pinB1;

    err = ledc_channel_config(&ledc_channel);
    if (err != ESP_OK)
    {
        vSemaphoreDelete(drv->mutex);
        return err;
    }

    // Configurar o canal 4 do PWM
    ledc_channel.channel = drv->config.channel_B2;
    ledc_channel.gpio_num = drv->config.motor_pinB2;

    err = ledc_channel_config(&ledc_channel);
    if (err != ESP_OK)
    {
        vSemaphoreDelete(drv->mutex);
        return err;
    }

    return ESP_OK;
}

/*
* @brief Define velocidade e direção para o motor A
*/

esp_err_t drv8833_set_motor_A (drv8833_t *drv, int32_t speedA)
{
    if (!drv)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Verifica se os valores de velocidade são válidos
    if (speedA > ((1<<drv->config.duty_resolution) - 1))
    {
        return ESP_ERR_INVALID_ARG;
    }

    // verificar se pegou de fato o mutex ou se deu erro (definir timeout)
    BaseType_t mutex_taken = xSemaphoreTake(drv->mutex, portMAX_DELAY);

    if (mutex_taken == pdFALSE)
    {
        return ESP_ERR_TIMEOUT;
    } 

    if (speedA > 0) //Se o motor A for para frente
    {
        esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_A1, speedA);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }

        err = ledc_update_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_A1);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }

        err = ledc_set_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_A2, 0);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }

        err = ledc_update_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_A2);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }
    }

    else if (speedA < 0) //Se o motor A for para trás
    {
        esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_A1, 0);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }

        err = ledc_update_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_A1);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }

        err = ledc_set_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_A2, (-1) * speedA);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }

        err = ledc_update_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_A2);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }
    }
    else
    {
        esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_A1, 0);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }

        err = ledc_update_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_A1);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }

        err = ledc_set_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_A2, 0);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }

        err = ledc_update_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_A2);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }
    }

    xSemaphoreGive(drv->mutex);
    return ESP_OK;
}

/*
* @brief Define velocidade e direção para o motor B
*/

esp_err_t drv8833_set_motor_B (drv8833_t *drv, int32_t speedB)
{
    if (!drv)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Verifica se os valores de velocidade são válidos
    if (speedB > ((1<<drv->config.duty_resolution)-1))
    {
        return ESP_ERR_INVALID_ARG;
    }

    // verificar se pegou de fato o mutex ou se deu erro (definir timeout)
    BaseType_t mutex_taken = xSemaphoreTake(drv->mutex, portMAX_DELAY);

    if (mutex_taken == pdFALSE)
    {
        return ESP_ERR_TIMEOUT;
    } 

    if (speedB > 0) //Se o motor A for para frente
    {
        esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_B1, speedB);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }

        err = ledc_update_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_B1);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }

        err = ledc_set_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_B2, 0);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }

        err = ledc_update_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_B2);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }
    }

    else if (speedB < 0) //Se o motor A for para trás
    {
        esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_B1, 0);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }

        err = ledc_update_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_B1);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }

        err = ledc_set_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_B2, (-1) * speedB);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }

        err = ledc_update_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_B2);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }
    }
    else
    {
        esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_B1, 0);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }

        err = ledc_update_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_B1);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }

        err = ledc_set_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_B2, 0);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }

        err = ledc_update_duty(LEDC_LOW_SPEED_MODE, drv->config.channel_B2);
        if (err != ESP_OK)
        {
            xSemaphoreGive(drv->mutex);
            return err;
        }
    }

    xSemaphoreGive(drv->mutex);
    return ESP_OK;
}

/*
* @brief Para ambos os motores
*/

esp_err_t dr8833_stop(drv8833_t *drv)
{
    if (!drv)
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = drv8833_set_motor_A(drv, 0);
    if (err != ESP_OK)
    {
        xSemaphoreGive(drv->mutex);
        return err;
    }

    err = drv8833_set_motor_B(drv, 0);
    if (err != ESP_OK)
    {
        xSemaphoreGive(drv->mutex);
        return err;
    }

    xSemaphoreGive(drv->mutex);
    return ESP_OK;
}

/*
* @brief Desinicializa e desaloca as memórias ao parar de usar o CI
*/

esp_err_t drv8833_deinit(drv8833_t *drv)
{
    if (!drv)
    {
        return ESP_ERR_INVALID_ARG;
    }

    vSemaphoreDelete(drv->mutex);

    if (drv->mutex == NULL) 
    {
        return ESP_FAIL;
    }

    esp_err_t err = ledc_stop(LEDC_LOW_SPEED_MODE, drv->config.channel_A1, 0);
    if (err != ESP_OK)
    {
        return err;
    }

    err = ledc_stop(LEDC_LOW_SPEED_MODE, drv->config.channel_A2, 0);
    if (err != ESP_OK)
    {
        return err;
    }

    err = ledc_stop(LEDC_LOW_SPEED_MODE, drv->config.channel_B1, 0);
    if (err != ESP_OK)
    {
        return err;
    }

    err = ledc_stop(LEDC_LOW_SPEED_MODE, drv->config.channel_B2, 0);
    if (err != ESP_OK)
    {
        return err;
    }

    vPortFree(drv);
    return ESP_OK;
}