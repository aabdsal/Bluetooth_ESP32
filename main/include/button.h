/**
    Copyright (C) 2026 Bluetooth_ESP32
    
    @file    button.h
    @author  BLUETOOTH
    @version V0.0
    @date    2026-04-01
    @brief   This file is a header for the button functions
            // Inicializa los GPIO de los botones (con pull-up).
            // Implementa una tarea FreeRTOS que periódicamente lee los botones con debounce.
            // Detecta pulsaciones de los botones "Bond" y "Erase" y envía eventos (semáforos/colas) a la máquina de estados.
   
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef BUTTON_H
#define BUTTON_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

// Modulos estadares de C
#include <stdint.h>

// Modulos propios de esp-idf

// Modulos propios para el proyecto

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/ 

#ifdef __cplusplus
}
#endif

#endif
/*** End of file **************************************************************/

