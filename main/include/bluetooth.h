/**
    Copyright (C) 2026 Bluetooth_ESP32
    
    @file    bluetooth.h
    @author  BLUETOOTH
    @version V0.0
    @date    2026-04-01
    @brief   This file is a header for the bluetooth functions
    // Callback único de eventos GAP (esp_gap_cb).
    // Maneja:
    // - ESP_GAP_BLE_SCAN_RESULT_EVT: recibe dispositivos escaneados; compara con la lista de bonds y evalúa RSSI.
    // - ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT: libera semáforo para la tarea de escaneo.
    // - ESP_GAP_BLE_AUTH_CMPL_EVT: detecta finalización del bonding (éxito/fracaso).
    // - Otros eventos necesarios (seguridad, advertising, etc.).

*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

// Modulos estadares de C
#include <stdint.h>
#include <string.h>

// Modulos propios de esp-idf
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_gap_ble_api.h"

// Modulos propios para el proyecto
#include "states.h"
#include "led.h"
/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/ 

#ifdef __cplusplus
}
#endif

#endif
/*** End of file **************************************************************/

