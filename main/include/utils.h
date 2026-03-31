/**
    Copyright (C) 2026 Bluetooth_ESP32
    
    @file    utils.h
    @author  BLUETOOTH
    @version V0.4
    @date    2026-04-01
    @brief   This file is a header for the utils functions
            // Funciones auxiliares:
            // - mac_to_str(): convierte dirección MAC a string para logs.
            // - rssi_to_distance(): convierte RSSI a distancia aproximada (para debug).
            // - debounce_filter(): implementación de filtro anti-rebotes para botones.
            // - maybe helpers para mostrar listas de dispositivos vinculados.
  
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

// Modulos estadares de C
#include <stdint.h>

// Modulos propios de esp-idf
#include "esp_bt_defs.h"

// Modulos propios para el proyecto

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/ 
uint8_t template_init(uint16_t peters);

#ifdef __cplusplus
}
#endif

#endif
/*** End of file **************************************************************/

