/**
 * @file    mando.c
 * @author  BLE-SEM
 * @version V0.0
 * @date    2026-05-16
 * @brief   Implementacion de botones para el mando.
 */

/* Includes ------------------------------------------------------------------*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "mando.h"
#include "ble_client.h"
//#include "lcd.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define ESP_INTR_FLAG_DEFAULT 0

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static const char *tag = "[MANDO]";

static const gpio_num_t BTN_SELECT = GPIO_NUM_17;
static const gpio_num_t BTN_OK     = GPIO_NUM_5;
static const gpio_num_t BTN_RIGHT  = GPIO_NUM_6;
static const gpio_num_t BTN_LEFT   = GPIO_NUM_7;
static const gpio_num_t SW_BLE_EN  = GPIO_NUM_15;

static const gpio_num_t LED_PIN  = GPIO_NUM_11;

static bool volatile btn_select_save = false;
static bool volatile btn_ok_save = false;
static bool volatile btn_right_save = false;
static bool volatile btn_left_save = false;
static bool volatile sw_ble_en_save = false;

static portMUX_TYPE btn_select_spinlock = portMUX_INITIALIZER_UNLOCKED;
static portMUX_TYPE btn_ok_spinlock = portMUX_INITIALIZER_UNLOCKED;
static portMUX_TYPE btn_right_spinlock = portMUX_INITIALIZER_UNLOCKED;
static portMUX_TYPE btn_left_spinlock = portMUX_INITIALIZER_UNLOCKED;
static portMUX_TYPE sw_ble_en_spinlock = portMUX_INITIALIZER_UNLOCKED;

static uint32_t last_btn_ok_tick = 0;
static uint32_t last_btn_left_tick = 0;
static uint32_t last_btn_right_tick = 0;
static uint32_t last_btn_select_tick = 0;

static uint32_t last_sw_ble_en_tick = 0;

static const uint8_t DEBOUNCE_TICKS = 50; // 50 ms

/* Private function prototypes -----------------------------------------------*/

static void btn_select_handler_isr(void *arg);
static void btn_ok_handler_isr(void *arg);
static void btn_right_handler_isr(void *arg);
static void btn_left_handler_isr(void *arg);
static void sw_ble_en_handler_isr(void *arg);

static void mando_task(void *pvParameters);

/* Exported functions --------------------------------------------------------*/

void mando_init(void)
{
    esp_err_t ret;

    gpio_config_t io_conf = 
    {
        .pin_bit_mask = (1ULL << BTN_SELECT) | // Configura TODOS estos pines con esta misma configuración 
                        (1ULL << BTN_OK) |
                        (1ULL << BTN_RIGHT) |
                        (1ULL << BTN_LEFT) | 
                        (1ULL << SW_BLE_EN), // Enable BLE
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE // falling edge interrupt
    };

    ret = gpio_config(&io_conf);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(tag, "gpio_config (botones) fallo en mando_init: %s (%d)", esp_err_to_name(ret), ret);
        return;
    }

    gpio_config_t io_conf_led = 
    {
        .pin_bit_mask = (1ULL << LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE 
    };

    ret = gpio_config(&io_conf_led);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(tag, "gpio_config (LED) fallo en mando_init: %s (%d)", esp_err_to_name(ret), ret);
        return;
    }
    
    ret = gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(tag, "gpio_install_isr_service fallo en mando_init: %s (%d)", esp_err_to_name(ret), ret);
        return;
    }

    ret = gpio_isr_handler_add(BTN_SELECT, btn_select_handler_isr, (void*) BTN_SELECT);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(tag, "gpio_isr_handler_add (BTN_SELECT) fallo en mando_init: %s (%d)", esp_err_to_name(ret), ret);
        return;
    }

    ret = gpio_isr_handler_add(BTN_OK, btn_ok_handler_isr, (void*) BTN_OK);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(tag, "gpio_isr_handler_add (BTN_OK) fallo en mando_init: %s (%d)", esp_err_to_name(ret), ret);
        return;
    }

    ret = gpio_isr_handler_add(BTN_RIGHT, btn_right_handler_isr, (void*) BTN_RIGHT);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(tag, "gpio_isr_handler_add (BTN_RIGHT) fallo en mando_init: %s (%d)", esp_err_to_name(ret), ret);
        return;
    }
    
    ret = gpio_isr_handler_add(BTN_LEFT, btn_left_handler_isr, (void*) BTN_LEFT);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(tag, "gpio_isr_handler_add (BTN_LEFT) fallo en mando_init: %s (%d)", esp_err_to_name(ret), ret);
        return;
    }
    ret = gpio_isr_handler_add(SW_BLE_EN, sw_ble_en_handler_isr, (void*) SW_BLE_EN);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(tag, "gpio_isr_handler_add (SW_BLE_EN) fallo en mando_init: %s (%d)", esp_err_to_name(ret), ret);
        return;
    }

    xTaskCreate(
        mando_task, 
        "mando_task", 
        4095, 
        NULL, 
        5, 
        NULL
    );

    ESP_LOGI(tag, "Mando inicializado correctamente");
}

bool mando_btn_select_read(void)
{
    bool btn_state = false;

    taskENTER_CRITICAL(&btn_select_spinlock);
    btn_state = btn_select_save;
    btn_select_save = false;
    taskEXIT_CRITICAL(&btn_select_spinlock);

    return btn_state;
}

bool mando_btn_ok_read(void)
{
    bool btn_state = false;

    taskENTER_CRITICAL(&btn_ok_spinlock);
    btn_state = btn_ok_save;
    btn_ok_save = false;
    taskEXIT_CRITICAL(&btn_ok_spinlock);

    return btn_state;
}

bool mando_btn_right_read(void)
{
    bool btn_state = false;

    taskENTER_CRITICAL(&btn_right_spinlock);
    btn_state = btn_right_save;
    btn_right_save = false;
    taskEXIT_CRITICAL(&btn_right_spinlock);

    return btn_state;
}

bool mando_btn_left_read(void)
{
    bool btn_state = false;

    taskENTER_CRITICAL(&btn_left_spinlock);
    btn_state = btn_left_save;
    btn_left_save = false;
    taskEXIT_CRITICAL(&btn_left_spinlock);

    return btn_state;
}

bool mando_sw_ble_en_event_read(void)
{
    bool sw_event = false;

    taskENTER_CRITICAL(&sw_ble_en_spinlock);
    sw_event = sw_ble_en_save;
    sw_ble_en_save = false;
    taskEXIT_CRITICAL(&sw_ble_en_spinlock);

    return sw_event;
}

/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/**
 * @brief  ISR del boton SELECT.
 * @param  pvParameters Argumento de la ISR (no usado).
 * @retval None
 */
static void mando_task(void *pvParameters)
{

    uint8_t servo = 1;
    bool msg_set = false;
    bool conectado = false;
    
    for(;;)
    {   
        char msg[16];

        bool is_connected = ble_client_is_connected();

        if (!is_connected)
        {
            if (mando_btn_right_read() ||
                mando_btn_left_read() ||
                mando_btn_ok_read() ||
                mando_btn_select_read())
            {
                ESP_LOGI(tag, "No puedes usar este boton porque el robot no esta conectado");
            }
            vTaskDelay(pdMS_TO_TICKS(500));
            //gpio_set_level(LED_PIN, 0);
            continue;  
        }


        if(ble_client_is_connected())
        {

            /*if(!conectado)
            {
                lcd_clearScreen(); 
                lcd_writeStr("¡Robot");
                lcd_setCursor(0, 1);
                lcd_writeStr("conectado!");
                conectado = true;
            }*/
            if(!conectado)
            {
                ESP_LOGI(tag, "¡Robot conectado!");
                conectado = true;
            }
            gpio_set_level(LED_PIN, 1);
        }
        else if (ble_client_is_scanning()) 
        {
                /*
                lcd_clearScreen(); 
                vTaskDelay(pdMS_TO_TICKS(500));
                lcd_writeStr("Iniciando");
                lcd_setCursor(0, 1);
                lcd_writeStr("escaneo...");
                */

                gpio_set_level(LED_PIN, 1);
                vTaskDelay(pdMS_TO_TICKS(500));
                gpio_set_level(LED_PIN, 0);
                vTaskDelay(pdMS_TO_TICKS(500));
                continue;
        }
        else
        {
            conectado = false;
            gpio_set_level(LED_PIN, 0);
        }
                
        

        
        if (mando_btn_right_read())
        {
            snprintf(msg, sizeof(msg), "S%d,A:%s", servo, "H");
            msg_set = true;

            /*char lcd_msg[16];
            snprintf(lcd_msg, sizeof(lcd_msg), "Giro: S%d,A:%s", servo, "H");

            lcd_clearScreen();
            lcd_writeStr(lcd_msg);*/
            
            ESP_LOGI(tag, "Boton right seleccionado");

            vTaskDelay(pdMS_TO_TICKS(300));
        }

        if (mando_btn_left_read())
        {

            snprintf(msg, sizeof(msg), "S%d,A:%s", servo, "A");
            msg_set = true;

            /*char lcd_msg[16];
            snprintf(lcd_msg, sizeof(lcd_msg), "Giro: S%d,A:%s", servo, "A");

            lcd_clearScreen();
            lcd_writeStr(lcd_msg);*/

            ESP_LOGI(tag, "Boton left seleccionado");

            vTaskDelay(pdMS_TO_TICKS(300));
        }


        if (mando_btn_select_read())
        {
            servo++;
            if (servo > 6) 
            {
                servo = 1;
            }

            /*char lcd_msg[16];
            snprintf(lcd_msg, sizeof(lcd_msg), "Servo selec: %d", servo);
            
            lcd_clearScreen();            
            lcd_writeStr(lcd_msg);*/

            ESP_LOGI(tag, "Servo seleccionado: %d", servo);
            
            vTaskDelay(pdMS_TO_TICKS(300));
        }
        
        if (mando_btn_ok_read())
        {
            snprintf(msg, sizeof(msg), "S%d,A:%s", servo, "O");
            msg_set = true;

            /*char lcd_msg[16];
            snprintf(lcd_msg, sizeof(lcd_msg), "Servo enviado");

            lcd_clearScreen();
            lcd_writeStr(lcd_msg);

            memset(lcd_msg, 0, sizeof(lcd_msg));
            snprintf(lcd_msg, sizeof(lcd_msg), "selec: %d", servo);

            lcd_setCursor(0, 1);
            lcd_writeStr(lcd_msg);*/

            ESP_LOGI(tag, "Mensaje enviado a traves de boton ok: %s", msg);
            vTaskDelay(pdMS_TO_TICKS(300));
        }

        if(msg_set)
        {
            ble_send(msg);
            msg_set = false;
        }
        
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

/******************************************************************************/
/**
 * @brief  ISR del boton SELECT.
 * @param  arg Argumento de la ISR (no usado).
 * @retval None
 */
static void IRAM_ATTR btn_select_handler_isr(void *arg)
{
    uint32_t now = xTaskGetTickCountFromISR();
    
    if (now - last_btn_select_tick > DEBOUNCE_TICKS) 
    {
        taskENTER_CRITICAL_ISR(&btn_select_spinlock);
        btn_select_save = true;
        taskEXIT_CRITICAL_ISR(&btn_select_spinlock);
        last_btn_select_tick = now;
    }
}

/******************************************************************************/
/**
 * @brief  ISR del boton OK.
 * @param  arg Argumento de la ISR (no usado).
 * @retval None
 */
static void IRAM_ATTR btn_ok_handler_isr(void *arg)
{
    uint32_t now = xTaskGetTickCountFromISR();

    if (now - last_btn_ok_tick > DEBOUNCE_TICKS) 
    {
        taskENTER_CRITICAL_ISR(&btn_ok_spinlock);
        btn_ok_save = true;
        taskEXIT_CRITICAL_ISR(&btn_ok_spinlock);
        last_btn_ok_tick = now;
    }
}

/******************************************************************************/
/**
 * @brief  ISR del boton RIGHT.
 * @param  arg Argumento de la ISR (no usado).
 * @retval None
 */
static void IRAM_ATTR btn_right_handler_isr(void *arg)
{
    uint32_t now = xTaskGetTickCountFromISR();

    if (now - last_btn_right_tick > DEBOUNCE_TICKS) 
    {
        taskENTER_CRITICAL_ISR(&btn_right_spinlock);
        btn_right_save = true;
        taskEXIT_CRITICAL_ISR(&btn_right_spinlock);
        last_btn_right_tick = now;
    }
}

/******************************************************************************/
/**
 * @brief  ISR del boton LEFT.
 * @param  arg Argumento de la ISR (no usado).
 * @retval None
 */
static void IRAM_ATTR btn_left_handler_isr(void *arg)
{
    uint32_t now = xTaskGetTickCountFromISR();

    if (now - last_btn_left_tick > DEBOUNCE_TICKS) 
    {
        taskENTER_CRITICAL_ISR(&btn_left_spinlock);
        btn_left_save = true;
        taskEXIT_CRITICAL_ISR(&btn_left_spinlock);
        last_btn_left_tick = now;
    }
}

/******************************************************************************/
/**
 * @brief  ISR del interruptor BLE_EN.
 * @param  arg Argumento de la ISR (no usado).
 * @retval None
 */
static void IRAM_ATTR sw_ble_en_handler_isr(void *arg)
{
    uint32_t now = xTaskGetTickCountFromISR();

    if (now - last_sw_ble_en_tick > DEBOUNCE_TICKS) 
    {
        taskENTER_CRITICAL_ISR(&sw_ble_en_spinlock);
        sw_ble_en_save = true;
        taskEXIT_CRITICAL_ISR(&sw_ble_en_spinlock);
        last_sw_ble_en_tick = now;
    }
}

/* End of file ****************************************************************/