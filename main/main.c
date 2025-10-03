#include "sdkconfig.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void) {
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << CONFIG_BLINK_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io);

    while (1) {
        gpio_set_level(CONFIG_BLINK_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(CONFIG_BLINK_DELAY_MS));
        gpio_set_level(CONFIG_BLINK_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(CONFIG_BLINK_DELAY_MS));
    }
}