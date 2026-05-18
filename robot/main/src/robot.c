/**
 * @file robot.c
 * @author BLE-SEM
 * @version V0.0
 * @date 2026-05-14
 * @brief Robot control implementation
 */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_log.h"

#include "driver/i2c_master.h"

#include "robot.h"
#include "gap_svc.h"

/* Private define ------------------------------------------------------------*/
#define ESP_INTR_FLAG_DEFAULT 0

static const char *tag = "[ROBOT]";

static const gpio_num_t INTERRUPTOR_PIN = GPIO_NUM_4;
static const gpio_num_t LED_PIN         = GPIO_NUM_3;
// static const gpio_num_t SDA_PIN         = GPIO_NUM_8;
// static const gpio_num_t SCL_PIN         = GPIO_NUM_10;

#define PCA9685_SERVO_BASE_REG 0x06
#define PCA9685_PWM_PERIOD_US  20000U
#define SERVO_MIN_PULSE_US     500U
#define SERVO_MAX_PULSE_US     2500U
#define SERVO_MIN_ANGLE        0U
#define SERVO_MAX_ANGLE        180U
#define SERVO_STEP             1U
#define SERVO_COUNT            6U

/* Private variables ---------------------------------------------------------*/

/*
 * La ESP32 manda comandos al PCA9685.
 * El PCA9685 ya está diseñado para ser slave I2C.
 * Por eso la ESP32 debe ir en modo master y escribir registros del PCA9685.
 */
i2c_master_dev_handle_t dev_handle;

static uint16_t servo_angle[SERVO_COUNT] = {90, 90, 90, 90, 90, 90};
static uint8_t canal_servo[] = {PCA9685_SERVO_BASE_REG, 0x00, 0x00, 0x00, 0x00};

static TaskHandle_t bluetooth_control_task_handle = NULL;

/* Private function prototypes ----------------------------------------------*/
static void gpio_handler_isr(void *arg);
static void bluetooth_control_task(void *arg);

static uint16_t clamp_angle(int angle);
static uint8_t servo_to_channel(robot_servo_t servo);
static uint16_t angle_to_ticks(uint16_t angle);

estado_led_t estado_led = APAGADO;
portMUX_TYPE led_mux = portMUX_INITIALIZER_UNLOCKED;

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Limita el angulo de un servo al rango soportado
 * @param angle Angulo solicitado en grados
 * @return Angulo limitado al intervalo valido del servo
 */
static uint16_t clamp_angle(int angle)
{
    if (angle < (int)SERVO_MIN_ANGLE)
    {
        return SERVO_MIN_ANGLE;
    }

    if (angle > (int)SERVO_MAX_ANGLE)
    {
        return SERVO_MAX_ANGLE;
    }

    return (uint16_t)angle;
}

/**
 * @brief Convierte un enum de servo del robot al canal asociado del PCA9685
 * @param servo Identificador del servo
 * @return Indice del canal, o 0xFF si el servo no es valido
 */
static uint8_t servo_to_channel(robot_servo_t servo)
{
    if (servo < SERVO1 || servo > SERVO6)
    {
        return 0xFF;
    }

    return (uint8_t)servo;
}

/**
 * @brief Convierte un angulo de servo en ticks PWM del PCA9685
 * @param angle Angulo del servo en grados
 * @return Valor de ticks PWM para el angulo objetivo
 */
static uint16_t angle_to_ticks(uint16_t angle)
{
    uint32_t pulse_us = SERVO_MIN_PULSE_US;

    pulse_us += ((uint32_t)angle * (SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US)) /
                SERVO_MAX_ANGLE;

    return (uint16_t)((pulse_us * 4096U) / PCA9685_PWM_PERIOD_US);
}

/**
 * @brief ISR del interruptor.
 *
 * La ISR NO arranca ni para BLE directamente.
 * Solo despierta/notifica a la tarea bluetooth_control_task.
 */
static void IRAM_ATTR gpio_handler_isr(void *arg)
{
    uint32_t pin = (uint32_t)(uintptr_t)arg;

    if (pin == INTERRUPTOR_PIN && bluetooth_control_task_handle != NULL)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        vTaskNotifyGiveFromISR(
            bluetooth_control_task_handle,
            &xHigherPriorityTaskWoken
        );

        if (xHigherPriorityTaskWoken == pdTRUE)
        {
            portYIELD_FROM_ISR();
        }
    }
}

/**
 * @brief Tarea que controla el estado BLE segun el interruptor.
 *
 * ON  -> gap_svc_start_advertising()
 * OFF -> gap_svc_stop_advertising()
 */
static void bluetooth_control_task(void *arg)
{
    int last_applied_state = -1;

    for(;;)
    {
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(250));
        vTaskDelay(pdMS_TO_TICKS(50));

        int current_state = gpio_get_level(INTERRUPTOR_PIN);

        if (current_state != last_applied_state)
        {
            last_applied_state = current_state;

            if (current_state == 1)
            {
                ESP_LOGI(tag, "Interruptor ON -> activar BLE advertising");

                gpio_set_level(LED_PIN, 1);
                gap_svc_start_advertising();
                taskENTER_CRITICAL(&led_mux);
                estado_led = PARPADEO;
                portEXIT_CRITICAL(&led_mux);
            }
            else
            {
                ESP_LOGI(tag, "Interruptor OFF -> desactivar BLE advertising");

                gpio_set_level(LED_PIN, 0);
                gap_svc_stop_advertising();
                taskENTER_CRITICAL(&led_mux);
                estado_led = APAGADO;
                portEXIT_CRITICAL(&led_mux);
            }
        }
    }
}

static void led_pool_task(void *arg)
{
    const TickType_t xDelay = pdMS_TO_TICKS(500);

    for(;;)
    {
        taskENTER_CRITICAL(&led_mux);
        estado_led_t current_estado = estado_led;
        taskEXIT_CRITICAL(&led_mux);

        if (current_estado == PARPADEO)
        {
            gpio_set_level(LED_PIN, 1);
            vTaskDelay(xDelay);
            gpio_set_level(LED_PIN, 0);
        }
        else if (current_estado == FIJO)
        {
            gpio_set_level(LED_PIN, 1);
        }
        else // APAGADO
        {
            gpio_set_level(LED_PIN, 0);
        }
    
        vTaskDelay(xDelay);
    }
}

/* Exported functions --------------------------------------------------------*/

void robot_init(void)
{
    ESP_LOGI(tag, "Iniciando configuracion del interruptor, LED y robot");

    gpio_config_t interruptor_conf =
    {
        .pin_bit_mask = 1ULL << INTERRUPTOR_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,

        .intr_type = GPIO_INTR_ANYEDGE
    };

    gpio_config_t led_conf =
    {
        .pin_bit_mask = 1ULL << LED_PIN,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    esp_err_t ret;

    ret = gpio_config(&interruptor_conf);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(tag, "gpio_config(interruptor_conf) fallo: %s", esp_err_to_name(ret));
    }

    ret = gpio_config(&led_conf);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(tag, "gpio_config(led_conf) fallo: %s", esp_err_to_name(ret));
    }

    gpio_set_level(LED_PIN, 0);
    taskENTER_CRITICAL(&led_mux);
    estado_led = APAGADO;
    portEXIT_CRITICAL(&led_mux);

    /*
     * Instala servicio ISR.
     * Si ya estaba instalado en otro modulo, ESP_ERR_INVALID_STATE no es critico.
     */
    esp_err_t err = gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE(tag, "gpio_install_isr_service() fallo: %s", esp_err_to_name(ret));
    }
    
    ret = gpio_isr_handler_add(INTERRUPTOR_PIN, gpio_handler_isr,
            (void *)(uintptr_t)INTERRUPTOR_PIN);

    if (ret != ESP_OK)
    {
        ESP_LOGE(tag, "gpio_isr_handler_add() fallo: %s", esp_err_to_name(ret));
    }

    xTaskCreate(
        bluetooth_control_task,
        "ble_ctrl_task",
        4096,
        NULL,
        5,
        &bluetooth_control_task_handle
    );

    /*
    // Configura la comunicacion del bus Inter-Integrated Circuit 
    i2c_master_bus_config_t bus_cfg = 
    {
        .i2c_port = I2C_NUM_0,                  // Puerto I2C, -1 para autodeteccion
        .sda_io_num = SDA_PIN,                  
        .scl_io_num = SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,                 // Ajusta el filtrado de ruido en la senal
        .flags.allow_pd = false,
        .flags.enable_internal_pullup = true, 
    };

    i2c_master_bus_handle_t bus_handle;

    // Configura el dispositivo concreto con el que va a hablar la ESP32
    i2c_device_config_t dev_cfg = 
    {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x40, // Direccion del modulo PCA9685 
        .scl_speed_hz = 100000,
    };


    ret = i2c_new_master_bus(&bus_cfg, &bus_handle);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(tag, "i2c_new_master_bus() fallo: %s", esp_err_to_name(ret));
    }
    ret = i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(tag, "i2c_master_bus_add_device() fallo: %s", esp_err_to_name(ret));
    }
    const uint8_t mode_one_on[] = {0x00, 0x10}; // MODE1 con SLEEP = 1
    size_t size_data_wr = sizeof(mode_one_on);

    ret = i2c_master_transmit(dev_handle, mode_one_on, size_data_wr, -1);   
    if (ret != ESP_OK) 
    {
        ESP_LOGE(tag, "i2c_master_transmit(mode_one_on) fallo: %s", esp_err_to_name(ret));
    }
    const uint8_t prescale[] = {0xFE, 121}; // PRE_SCALE = 121 (50 Hz) 
    ret = i2c_master_transmit(dev_handle, prescale, sizeof(prescale), -1);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(tag, "i2c_master_transmit(prescale) fallo: %s", esp_err_to_name(ret));
    }
    const uint8_t mode_one_off[] = {0x00, 0x00}; // Volver a MODE1 con SLEEP = 0 
    ret = i2c_master_transmit(dev_handle, mode_one_off, sizeof(mode_one_off), -1);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(tag, "i2c_master_transmit(mode_one_off) fallo: %s", esp_err_to_name(ret));
    }
    const uint8_t restart[] = {0x00, 0x80}; // Reinicio 
    ret = i2c_master_transmit(dev_handle, restart, sizeof(restart), -1);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(tag, "i2c_master_transmit(restart) fallo: %s", esp_err_to_name(ret));
    }
    */
}

void move_servo(robot_servo_t servo, robot_move_t move)
{
    uint8_t channel = servo_to_channel(servo);

    if (channel == 0xFF)
    {
        ESP_LOGE(tag, "Servo no valido");
        return;
    }

    int new_angle = servo_angle[channel];

    if (move == HORARIO)
    {
        new_angle += SERVO_STEP;
    }
    else if (move == ANTIHORARIO)
    {
        new_angle -= SERVO_STEP;
    }
    else
    {
        ESP_LOGE(tag, "Movimiento no valido");
        return;
    }

    servo_angle[channel] = clamp_angle(new_angle);

    uint16_t ticks = angle_to_ticks(servo_angle[channel]);

    uint16_t on = 0;
    uint16_t off = ticks;

    canal_servo[0] = PCA9685_SERVO_BASE_REG + (4 * channel);
    canal_servo[1] = on & 0xFF;
    canal_servo[2] = (on >> 8) & 0xFF;
    canal_servo[3] = off & 0xFF;
    canal_servo[4] = (off >> 8) & 0xFF;

    /*
     * Aqui iria tu escritura I2C al PCA9685 si ya la tenias implementada.
     *   esp_err_t ret;
     *   ret = i2c_master_transmit(dev_handle, canal_servo, sizeof(canal_servo), -1);
     *   if (ret != ESP_OK) 
     *   {
     *       ESP_LOGE(tag, "i2c_master_transmit(canal_servo) fallo: %s", esp_err_to_name(ret));
     *   }
     */

    ESP_LOGI(
        tag,
        "Servo %u movido a %u grados, ticks=%u",
        channel + 1,
        servo_angle[channel],
        ticks
    );
}

/* End of file ***************************************************************/