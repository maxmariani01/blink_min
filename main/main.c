#include <stdint.h>
#include "driver/gpio.h"
#define LED_GPIO 4   // LED externo en GPIO4
static void delay_loop(volatile uint32_t ticks) {
    while (ticks--) {
        __asm__ __volatile__("nop");
    }
}
void app_main(void) {
    // Configurar GPIO4 como salida
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << LED_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io);
    // Parpadear
    while (1) {
        gpio_set_level(LED_GPIO, 1);
        delay_loop(10000000);   // ajustá este número para cambiar la velocidad
        gpio_set_level(LED_GPIO, 0);
        delay_loop(10000000);
    }
}