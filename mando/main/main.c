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

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
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

    // Verificar el estado inicial del interruptor
    bool ble_enabled = mando_sw_ble_en_event_read();
    if (ble_enabled)
    {
        ESP_LOGI(tag, "Interruptor activado en el arranque. Iniciando BLE...");
        ble_client_init();
    }
    else
    {
        ESP_LOGI(tag, "Interruptor desactivado en el arranque. Esperando...");
    }

    uint8_t servo = 1;

    for(;;)
    {   
        // Comprobar eventos del interruptor
        if (mando_sw_ble_en_event_read())
        {
            ble_enabled = mando_sw_ble_en_event_read();
            if (ble_enabled)
            {
                ESP_LOGI(tag, "Interruptor activado. Iniciando Bluetooth...");
                // Nota: Una vez inicializado en ESP-IDF, requiere un reinicio o deinit complejo para apagar la radio completamente.
                // Aqui lo mas seguro es reiniciar el ESP o simplemente inicializarlo si no lo estaba.
                // Simularemos la recarga para evitar fugas de memoria, o aseguramos iniciar solo 1 vez
                static bool first_init = true;
                if (first_init)
                {
                    ble_client_init();
                    first_init = false;
                }
                else
                {
                    ESP_LOGW(tag, "Bluetooth ya fue inicializado, usar deinit no soportado todavia.");
                }
            }
            else
            {
                ESP_LOGI(tag, "Interruptor desactivado. Se deshabilita el envio bluetooth.");
            }
            vTaskDelay(pdMS_TO_TICKS(100)); // Debounce software opcional
        }
     
        if (mando_btn_select_read())
        {
            servo++;
            if (servo > 6) servo = 1;
            ESP_LOGI(tag, "Servo seleccionado: %d", servo);
            vTaskDelay(pdMS_TO_TICKS(300));
        }

        if (mando_btn_right_read())
        {
            char msg[16];
            snprintf(msg, sizeof(msg), "S%d,A:%s", servo, "H");
            ESP_LOGI(tag, "Boton right seleccionado");
            if (ble_enabled) ble_send(msg);
            else ESP_LOGW(tag, "Bluetooth desactivado, no se envia");
            vTaskDelay(pdMS_TO_TICKS(300));
        }
        
        if (mando_btn_left_read())
        {
            char msg[16];
            snprintf(msg, sizeof(msg), "S%d,A:%s", servo, "A");
            ESP_LOGI(tag, "Boton left seleccionado");
            if (ble_enabled) ble_send(msg);
            else ESP_LOGW(tag, "Bluetooth desactivado, no se envia");
            vTaskDelay(pdMS_TO_TICKS(300));
        }

        
        if (mando_btn_ok_read())
        {
            char msg[16];
            snprintf(msg, sizeof(msg), "S%d,A:%s", servo, "O");
            ESP_LOGI(tag, "Boton ok seleccionado");
            if (ble_enabled) ble_send(msg);
            else ESP_LOGW(tag, "Bluetooth desactivado, no se envia");
            vTaskDelay(pdMS_TO_TICKS(300));
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/* Private functions ---------------------------------------------------------*/

/* End of file ****************************************************************/