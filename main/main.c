/**
    Copyright (C) 2026 Bluetooth_ESP32
    
    @file    main.c
    @author  BLUETOOTH
    @version V0.0
    @date    2026-04-01
    @brief  Punto de entrada del programa (app_main).
            - Inicializa NVS, configura los GPIOs, inicializa Bluetooth (Bluedroid).
            - Registra el callback GAP (ble_gap_callback).
            - Crea las tareas de FreeRTOS: polling de botones, máquina de estados.
            - Bucle principal (vacío o con supervisión de estado global).

*/

/* Includes ------------------------------------------------------------------*/

// Modulos estadares de C
#include <stdio.h>
#include <stdint.h>

// Modulos propios de esp-idf
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "driver/gpio.h"

// Modulos propios para el proyecto
#include "bluetooth.h"
#include "button.h"
#include "led.h"
#include "bonding.h"
#include "states.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void app_main(void)
{
    return;
}
/* End of file ****************************************************************/

