/**
    Copyright (C) 2026 Bluetooth_ESP32
    
    @file    led.h
    @author  BLUETOOTH
    @version V0.4
    @date    2026-04-01
    @brief   This file is a header for the led functions
            // Funciones para controlar los LEDs:
            // - led_init(): configura los GPIO de salida.
            // - led_set_color(color): enciende el color correspondiente (RGB o individuales).
            // - led_blink(times, period): efecto de parpadeo para indicar estados.
            // - led_off(): apaga todos.

*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef LED_H
#define LED_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

// Modulos estadares de C
#include <stdint.h>

// Modulos propios de esp-idf

// Modulos propios para el proyecto

/* Exported types ------------------------------------------------------------*/
typedef enum {
    LED_OFF,
    LED_RED,
    LED_GREEN,
    LED_BLUE,
    LED_YELLOW,
    LED_CYAN,
    LED_MAGENTA,
    LED_WHITE,
    LED_BLINK_RAPID
} led_color_t;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/ 
uint8_t template_init(uint16_t peters);

#ifdef __cplusplus
}
#endif

#endif
/*** End of file **************************************************************/

