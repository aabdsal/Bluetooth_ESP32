/**
    Copyright (C) 2026 Bluetooth_ESP32
    
    @file    states.h
    @author  BLUETOOTH
    @version V0.4
    @date    2026-04-01
    @brief   This file is a header for the states functions
            // Máquina de estados principal.
            // Define los estados: STATE_SCAN, STATE_BONDING, STATE_ERASE.
            // Contiene las tareas FreeRTOS:
            // - scan_task(): ejecuta escaneos periódicos con light sleep entre ciclos.
            // - bonding_task(): configura advertising, espera emparejamiento con timeout.
            // Funciones de transición:
            // - start_scan_mode()  (detiene bonding, crea scan_task)
            // - start_bonding_mode() (detiene scan, crea bonding_task)
            // - erase_all_bonds()   (borra todos los dispositivos vinculados usando esp_ble_remove_bond_device)
 
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STATES_H
#define STATES_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

// Modulos estadares de C
#include <stdint.h>

// Modulos propios de esp-idf
#include "esp_bt_defs.h"
#include "freertos/semphr.h"

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

