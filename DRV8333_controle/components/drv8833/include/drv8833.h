#ifndef DRV8833
#define DRV833

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    gpio_num_t motor_pinA1; // GPIO pin for the A1 motor port
    gpio_num_t motor_pinA2; // GPIO pin for the A2 motor port
    gpio_num_t motor_pinB1; // GPIO pin for the B1 motor port
    gpio_num_t motor_pinB2; // GPIO pin for the B2 motor port
    //gpio_num_t sleep_pin;   // Sleep pin for the driver

    ledc_channel_t channel_A1; // LEDC channel for A1 motor port
    ledc_channel_t channel_A2; // LEDC channel for A2 motor port
    ledc_channel_t channel_B1; // LEDC channel for B1 motor port
    ledc_channel_t channel_B2; // LEDC channel for B2 motor port

    ledc_timer_t timer_num; // LEDC timer for the motor

    uint8_t duty_resolution; // LEDC duty resolution
} drv8833_config_t;

typedef struct {
    drv8833_config_t config; // Adicione o asterisco aqui
    SemaphoreHandle_t mutex;
} drv8833_t;

esp_err_t drv8833_init(drv8833_t *drv, const drv8833_config_t *config);
esp_err_t drv8833_set_motor_A(drv8833_t *drv, int32_t speedA);
esp_err_t drv8833_set_motor_B(drv8833_t *drv, int32_t speedB);
esp_err_t drv8833_stop(drv8833_t *drv);
esp_err_t drv8833_deinit(drv8833_t *drv);

#ifdef __cplusplus
}   
#endif

#endif //DRV8833