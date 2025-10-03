// --- Cabeceras necesarias ---
#include "sdkconfig.h"           // Trae valores de menuconfig (p.ej., CONFIG_BLINK_GPIO)
#include "driver/ledc.h"         // Driver del periférico LEDC (PWM)
#include "freertos/FreeRTOS.h"   // Base de FreeRTOS
#include "freertos/task.h"       // vTaskDelay
#include "esp_err.h"             // ESP_ERROR_CHECK para validar llamadas

// Resolución y rango: 10 bits => duty de 0..1023
#define LEDC_RES_BITS  LEDC_TIMER_10_BIT
#define LEDC_MAX_DUTY  ((1 << 10) - 1)   // 1023

void app_main(void) {
    // --- 1) Configuración del TIMER del LEDC ---
    ledc_timer_config_t tcfg = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,  // Modo "lento": apto para la mayoría de GPIOs
        .duty_resolution = LEDC_RES_BITS,        // Resolución del duty (10 bits)
        .timer_num       = LEDC_TIMER_0,         // Usaremos el timer 0
        .freq_hz         = 5000,                 // Frecuencia PWM: 5 kHz (sin flicker en LED)
        .clk_cfg         = LEDC_AUTO_CLK         // Selección automática de reloj
    };
    ESP_ERROR_CHECK(ledc_timer_config(&tcfg));   // Aplica config al hardware y valida que no falle

    // --- 2) Configuración del CANAL del LEDC ---
    ledc_channel_config_t ch = {
        .gpio_num   = CONFIG_BLINK_GPIO,         // Pin del LED (lo definís en menuconfig)
        .speed_mode = LEDC_LOW_SPEED_MODE,       // Debe coincidir con el timer
        .channel    = LEDC_CHANNEL_0,            // Canal 0 (salida PWM)
        .timer_sel  = LEDC_TIMER_0,              // Asocia este canal al timer 0
        .duty       = 0,                         // Duty inicial = 0 (apagado en activo-alto)
        .hpoint     = 0                          // Punto de inicio del ciclo (0 por defecto)
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch));   // Aplica config del canal y valida

    // --- 3) Habilitar el motor de FADE (rampas por hardware) ---
    ESP_ERROR_CHECK(ledc_fade_func_install(0));  // Inicializa internamente el soporte de fade
                                                 // (0 = sin tamaño extra de ISR)

    // Tiempos de subida y bajada de la respiración (en milisegundos)
    const int up_ms = 1200;   // Subir brillo 0 -> 1023 en 1.2 s
    const int dn_ms = 300;   // Bajar brillo 1023 -> 0 en 0.3 s

    // --- 4) Bucle principal: pedir rampas y esperar a que terminen ---
    while (1) {
        // Pide al hardware: subir hasta duty máximo en 'up_ms' milisegundos
        ESP_ERROR_CHECK(ledc_set_fade_with_time(
            LEDC_LOW_SPEED_MODE,       // Modo
            LEDC_CHANNEL_0,            // Canal que maneja el GPIO
            LEDC_MAX_DUTY,             // Duty destino (1023)
            up_ms                      // Duración de la rampa
        ));
        // Arranca la rampa y espera a que termine antes de seguir
        ESP_ERROR_CHECK(ledc_fade_start(
            LEDC_LOW_SPEED_MODE,
            LEDC_CHANNEL_0,
            LEDC_FADE_WAIT_DONE        // Bloquea hasta completar la rampa
        ));

        // Pide al hardware: bajar hasta duty 0 en 'dn_ms' milisegundos
        ESP_ERROR_CHECK(ledc_set_fade_with_time(
            LEDC_LOW_SPEED_MODE,
            LEDC_CHANNEL_0,
            0,                          // Duty destino (apagado)
            dn_ms
        ));
        // Arranca la rampa y espera al final
        ESP_ERROR_CHECK(ledc_fade_start(
            LEDC_LOW_SPEED_MODE,
            LEDC_CHANNEL_0,
            LEDC_FADE_WAIT_DONE
        ));
    }
}