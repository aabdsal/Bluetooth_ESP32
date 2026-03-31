/**
    Copyright (C) 2026 Bluetooth_ESP32
    
    @file    button.c
    @author  BLUETOOTH
    @version V0.0
    @date    2026-04-01
    @brief   This file is a header for the button functions
          
*/

/* Includes ------------------------------------------------------------------*/

// Modulos estadares de C
#include <stdint.h>

// Modulos propios de esp-idf
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

// Modulos propios para el proyecto
#include "button.h"
#include "states.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/**
    @brief  Bla, bla
    @param  Bla, bla

    @retval The number os pepes
*/
uint8_t template_init(uint16_t peters)
{
    static int8_t counter;
        
    return peters+1;
}

/* End of file ****************************************************************/

