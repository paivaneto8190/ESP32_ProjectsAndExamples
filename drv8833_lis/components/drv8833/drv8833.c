#include <stdio.h>
#include "drv8833.h"


#define LEDC_FREQ 5000

static esp_err_t config_ledc_channel(ledc_channel_t channel, gpio_num_t gpio_num, ledc_timer_t timer_num) {
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = channel,
        .timer_sel  = timer_num,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = gpio_num,
        .duty       = 0, // Inicia parado (0%)
        .hpoint     = 0,
    };
    return ledc_channel_config(&ledc_channel);
}


// INSERIR LOGS PARA DEBUGAR E INFORMAR ERROS
/**
    Inicialização do DRV8833
 */

esp_err_t drv8833_init(drv8833_t *motor, const drv8833_config_t *config){

    // Verificar se a informação que o usuário colocou está correta
     if (!motor || !config)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if(!GPIO_IS_VALID_GPIO (config->AIN1_pin) || !GPIO_IS_VALID_GPIO (config->AIN2_pin) || !GPIO_IS_VALID_GPIO (config->BIN1_pin) || !GPIO_IS_VALID_GPIO (config->BIN2_pin))
    {
        return ESP_ERR_INVALID_ARG;
    }

    motor->config = *config;
    motor->mutex = xSemaphoreCreateMutex();

    if (motor->mutex == NULL)
    {
        return ESP_FAIL;
    }

    // Configuração dos parametros do timer do LEDC
    ledc_timer_config_t ledc_timer ={
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = motor->config.duty_resolution,
        .timer_num = motor->config.timer_num,
        .freq_hz = LEDC_FREQ,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    esp_err_t err = ledc_timer_config(&ledc_timer);

    if(err != ESP_OK){
        vSemaphoreDelete(motor->mutex);
        return err;
    }

    // Configuração dos canais PWM
    err |= config_ledc_channel(motor->config.AIN1_channel, motor->config.AIN1_pin, motor->config.timer_num);
    err |= config_ledc_channel(motor->config.AIN2_channel, motor->config.AIN2_pin, motor->config.timer_num);
    err |= config_ledc_channel(motor->config.BIN1_channel, motor->config.BIN1_pin, motor->config.timer_num);
    err |= config_ledc_channel(motor->config.BIN2_channel, motor->config.BIN2_pin, motor->config.timer_num);

    if (err != ESP_OK) {
        vSemaphoreDelete(motor->mutex);
        return err;
    }

    return ESP_OK;
}


 /**
    Define a velocidade e direção do Motor A.
 */
esp_err_t drv8833_set_motor_a(drv8833_t *motor, int32_t  velocidade_A)
{
    if(!motor){
        return ESP_ERR_INVALID_ARG;
    }

    int32_t duty = abs(velocidade_A); 
    int32_t duty_in1 = (velocidade_A > 0) ? duty : 0;
    int32_t duty_in2 = (velocidade_A < 0) ? duty : 0;

    xSemaphoreTake(motor->mutex, portMAX_DELAY);

    
    esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, motor->config.AIN1_channel, duty_in1);
    if (err != ESP_OK)
    {
        xSemaphoreGive(motor->mutex);
        return err;
    }

    err = ledc_update_duty(LEDC_LOW_SPEED_MODE, motor->config.AIN1_channel);
    if (err != ESP_OK)
    {
        xSemaphoreGive(motor->mutex);
        return err;
    }

    err = ledc_set_duty(LEDC_LOW_SPEED_MODE, motor->config.AIN2_channel, duty_in2);
    
    
    if (err != ESP_OK)
    {
        xSemaphoreGive(motor->mutex);
        return err;
    }
    err = ledc_update_duty(LEDC_LOW_SPEED_MODE, motor->config.AIN2_channel);
    if (err != ESP_OK)
    {
        xSemaphoreGive(motor->mutex);
        return err;
    }

    xSemaphoreGive(motor->mutex);
    return ESP_OK;

} 

/**
    Define a velocidade e direção do Motor B.
*/

esp_err_t drv8833_set_motor_b(drv8833_t *motor, int32_t velocidade_B)
{
 if(!motor){
        return ESP_ERR_INVALID_ARG;
    }

    int32_t duty = abs(velocidade_B); 
    int32_t duty_in1 = (velocidade_B > 0) ? duty : 0;
    int32_t duty_in2 = (velocidade_B < 0) ? duty : 0;

    xSemaphoreTake(motor->mutex, portMAX_DELAY);

    
    esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, motor->config.BIN1_channel, duty_in1);
    if (err != ESP_OK)
    {
        xSemaphoreGive(motor->mutex);
        return err;
    }

    err = ledc_update_duty(LEDC_LOW_SPEED_MODE, motor->config.BIN1_channel);
    if (err != ESP_OK)
    {
        xSemaphoreGive(motor->mutex);
        return err;
    }

    err = ledc_set_duty(LEDC_LOW_SPEED_MODE, motor->config.BIN2_channel, duty_in2);
    
    
    if (err != ESP_OK)
    {
        xSemaphoreGive(motor->mutex);
        return err;
    }
    err = ledc_update_duty(LEDC_LOW_SPEED_MODE, motor->config.BIN2_channel);
    if (err != ESP_OK)
    {
        xSemaphoreGive(motor->mutex);
        return err;
    }

    xSemaphoreGive(motor->mutex);
    return ESP_OK;
}

/**
    Parada de emergencia.
 */
esp_err_t drv8833_stop(drv8833_t *motor){
    if(!motor){
        return ESP_ERR_INVALID_ARG;
    }

    xSemaphoreTake(motor->mutex, portMAX_DELAY);

    // Zera todos os canais para parar os motores (Coast mode)
    ledc_set_duty(LEDC_LOW_SPEED_MODE, motor->config.AIN1_channel, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, motor->config.AIN1_channel);
    
    ledc_set_duty(LEDC_LOW_SPEED_MODE, motor->config.AIN2_channel, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, motor->config.AIN2_channel);
    
    ledc_set_duty(LEDC_LOW_SPEED_MODE, motor->config.BIN1_channel, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, motor->config.BIN1_channel);
    
    ledc_set_duty(LEDC_LOW_SPEED_MODE, motor->config.BIN2_channel, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, motor->config.BIN2_channel);

    xSemaphoreGive(motor->mutex);

    return ESP_OK;
}