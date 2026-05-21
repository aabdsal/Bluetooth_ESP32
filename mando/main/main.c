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
#include "lcd.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static const char *tag = "[MANDO]";

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
    lcd_init();
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

            lcd_clearScreen();
            char *lcd_msg = "Servo selec: " + (char)(servo);  
            lcd_writeStr(lcd_msg);
            ESP_LOGI(tag, "Servo seleccionado: %d", servo);
            
            vTaskDelay(pdMS_TO_TICKS(300));
        }

        if (mando_btn_right_read())
        {
            snprintf(msg, sizeof(msg), "S%d,A:%s", servo, "H");
            msg_set = true;

            lcd_clearScreen();
            lcd_writeStr("Boton right seleccionado");
            ESP_LOGI(tag, "Boton right seleccionado");

            vTaskDelay(pdMS_TO_TICKS(300));
        }
        
        if (mando_btn_left_read())
        {
            snprintf(msg, sizeof(msg), "S%d,A:%s", servo, "A");
            msg_set = true;

            lcd_clearScreen();
            lcd_writeStr("Boton left seleccionado");
            ESP_LOGI(tag, "Boton left seleccionado");

            vTaskDelay(pdMS_TO_TICKS(300));
        }

        
        if (mando_btn_ok_read())
        {
            snprintf(msg, sizeof(msg), "S%d,A:%s", servo, "O");
            msg_set = true;

            lcd_clearScreen();
            lcd_writeStr("Boton ok seleccionado");
            ESP_LOGI(tag, "Boton ok seleccionado");
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