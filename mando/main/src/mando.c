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
#include "mando.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define ESP_INTR_FLAG_DEFAULT 0

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static const gpio_num_t BTN_SELECT = GPIO_NUM_4;
static const gpio_num_t BTN_OK     = GPIO_NUM_5;
static const gpio_num_t BTN_RIGHT  = GPIO_NUM_6;
static const gpio_num_t BTN_LEFT   = GPIO_NUM_7;
static const gpio_num_t SW_BLE_EN  = GPIO_NUM_8;

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


/* Private function prototypes -----------------------------------------------*/

static void btn_select_handler_isr(void *arg);
static void btn_ok_handler_isr(void *arg);
static void btn_right_handler_isr(void *arg);
static void btn_left_handler_isr(void *arg);
static void sw_ble_en_handler_isr(void *arg);

/* Exported functions --------------------------------------------------------*/


void mando_init(void)
{
    gpio_config_t io_conf = 
    {
        .pin_bit_mask = (1ULL << BTN_SELECT) | // Configura TODOS estos pines con esta misma configuración 
                        (1ULL << BTN_OK) |
                        (1ULL << BTN_RIGHT) |
                        (1ULL << BTN_LEFT),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE // falling edge interrupt
    };

    gpio_config(&io_conf);

    gpio_config_t io_conf_sw = 
    {
        .pin_bit_mask = (1ULL << SW_BLE_EN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE // any edge interrupt for switch
    };

    gpio_config(&io_conf_sw);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

    gpio_isr_handler_add(BTN_SELECT, btn_select_handler_isr, (void*) BTN_SELECT);
    gpio_isr_handler_add(BTN_OK, btn_ok_handler_isr, (void*) BTN_OK);
    gpio_isr_handler_add(BTN_RIGHT, btn_right_handler_isr, (void*) BTN_RIGHT);
    gpio_isr_handler_add(BTN_LEFT, btn_left_handler_isr, (void*) BTN_LEFT);
    gpio_isr_handler_add(SW_BLE_EN, sw_ble_en_handler_isr, (void*) SW_BLE_EN);

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
 * @param  arg Argumento de la ISR (no usado).
 * @retval None
 */
static void IRAM_ATTR btn_select_handler_isr(void *arg)
{
    (void)arg;
    taskENTER_CRITICAL_ISR(&btn_select_spinlock);
    btn_select_save = true;
    taskEXIT_CRITICAL_ISR(&btn_select_spinlock);
}

/******************************************************************************/
/**
 * @brief  ISR del boton OK.
 * @param  arg Argumento de la ISR (no usado).
 * @retval None
 */
static void IRAM_ATTR btn_ok_handler_isr(void *arg)
{
    (void)arg;
    taskENTER_CRITICAL_ISR(&btn_ok_spinlock);
    btn_ok_save = true;
    taskEXIT_CRITICAL_ISR(&btn_ok_spinlock);
}

/******************************************************************************/
/**
 * @brief  ISR del boton RIGHT.
 * @param  arg Argumento de la ISR (no usado).
 * @retval None
 */
static void IRAM_ATTR btn_right_handler_isr(void *arg)
{
    (void)arg;
    taskENTER_CRITICAL_ISR(&btn_right_spinlock);
    btn_right_save = true;
    taskEXIT_CRITICAL_ISR(&btn_right_spinlock);
}

/******************************************************************************/
/**
 * @brief  ISR del boton LEFT.
 * @param  arg Argumento de la ISR (no usado).
 * @retval None
 */
static void IRAM_ATTR btn_left_handler_isr(void *arg)
{
    (void)arg;
    taskENTER_CRITICAL_ISR(&btn_left_spinlock);
    btn_left_save = true;
    taskEXIT_CRITICAL_ISR(&btn_left_spinlock);
}

/******************************************************************************/
/**
 * @brief  ISR del interruptor BLE_EN.
 * @param  arg Argumento de la ISR (no usado).
 * @retval None
 */
static void IRAM_ATTR sw_ble_en_handler_isr(void *arg)
{
    (void)arg;
    taskENTER_CRITICAL_ISR(&sw_ble_en_spinlock);
    sw_ble_en_save = true;
    taskEXIT_CRITICAL_ISR(&sw_ble_en_spinlock);
}

/* End of file ****************************************************************/