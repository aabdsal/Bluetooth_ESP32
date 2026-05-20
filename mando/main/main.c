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
#include "esp_log.h"

#include "mando.h"
#include "ble_client.h"
#include "hd44780.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define LCD_ADDR 0x27
#define SDA_PIN  21
#define SCL_PIN  22
#define LCD_COLS 16
#define LCD_ROWS 2

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

// static const char *tag = "[MANDO]";

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
    mando_init();
    ble_client_set_device_name("ESP32-S3 Mando");
    LCD_init(LCD_ADDR, SDA_PIN, SCL_PIN, LCD_COLS, LCD_ROWS);
    ble_client_init();

    uint8_t servo = 1;
    bool msg_set = false;

    for(;;)
    {   
        char msg[16];
        
        if (mando_btn_select_read())
        {
            servo++;
            if (servo > 6) 
            {
                servo = 1;
            }

            LCD_clearScreen();
            char *lcd_msg = "Servo seleccionado: " + (char)(servo);  
            LCD_writeStr(lcd_msg);

            vTaskDelay(pdMS_TO_TICKS(300));
        }

        if (mando_btn_right_read())
        {
            snprintf(msg, sizeof(msg), "S%d,A:%s", servo, "H");
            msg_set = true;

            LCD_clearScreen();
            LCD_writeStr("Boton right seleccionado");

            vTaskDelay(pdMS_TO_TICKS(300));
        }
        
        if (mando_btn_left_read())
        {
            snprintf(msg, sizeof(msg), "S%d,A:%s", servo, "A");
            msg_set = true;

            LCD_clearScreen();
            LCD_writeStr("Boton left seleccionado");

            vTaskDelay(pdMS_TO_TICKS(300));
        }

        
        if (mando_btn_ok_read())
        {
            snprintf(msg, sizeof(msg), "S%d,A:%s", servo, "O");
            msg_set = true;

            LCD_clearScreen();
            LCD_writeStr("Boton ok seleccionado");
            vTaskDelay(pdMS_TO_TICKS(300));
        }

        if(msg_set)
        {
            ble_send(msg);
        }

        msg_set = false;
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/* Private functions ---------------------------------------------------------*/

/* End of file ****************************************************************/
