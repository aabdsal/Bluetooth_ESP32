/**
    Copyright (C) 2026 Bluetooth_ESP32
    
    @file    bonding.h
    @author  BLUETOOTH
    @version V0.1
    @date    2026-04-01
    @brief   This file is a header for the bonding functions
          
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef BONDING_H
#define BONDING_H

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

