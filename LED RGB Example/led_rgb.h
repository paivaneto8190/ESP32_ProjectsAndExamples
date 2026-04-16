#ifndef LED_RGB_H
#define LED_RGB_H

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
 
    gpio_num_t red_pin;    // GPIO pin for the red LED
    gpio_num_t green_pin;  // GPIO pin for the green LED
    gpio_num_t blue_pin;   // GPIO pin for the blue LED

    ledc_channel_t red_channel;    // LEDC channel for the red LED
    ledc_channel_t green_channel;  // LEDC channel for the green LED
    ledc_channel_t blue_channel;   // LEDC channel for the blue LED

    ledc_timer_t timer_num;       // LEDC timer number
    uint32_t duty_resolution; // LEDC duty resolution (e.g., 8 bits

}led_rgb_config_t;

typedef struct{
    led_rgb_config_t config; // Configuration for the RGB LED
    xSemaphoreHandle_t mutex; // Mutex for thread-safe access to the LED
}led_rgb_t;

esp_err_t led_rgb_init(led_rgb_t *led, const led_rgb_config_t *config);
esp_err_t led_rgb_set_color(led_rgb_t *led, uint32_t red, uint32_t green, uint32_t blue);
esp_err_t led_deinit(led_rgb_t *led);


#ifdef __cplusplus
}   
#endif

#endif // LED_RGB_H

