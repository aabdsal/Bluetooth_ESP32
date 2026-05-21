/**
 * @file    main.c
 * @author  BLE-SEM
 * @version V0.0
 * @date    2026-05-16
 * @brief   Punto de entrada de la aplicacion.
 */

/* Includes ------------------------------------------------------------------*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "mando.h"
#include "ble_client.h"
#include "lcd.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/******************************************************************************/
/**
 * @brief  Bucle principal de la aplicacion.
 * @param  None
 * @retval None
 */
void app_main(void)
{
    lcd_init();
    lcd_clearScreen();
    lcd_writeStr("Mando Init");
    
    mando_init();
    ble_client_set_device_name("ESP32-S3 Mando");
    ble_client_init();
}

/* Private functions ---------------------------------------------------------*/

/* End of file ****************************************************************/