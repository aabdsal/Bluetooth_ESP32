/**
 * @file    robot.h
 * @author  BLE-SEM
 * @version V0.0
 * @date    2026-05-14
 * @brief   Interfaz publica de control del robot
 */

/* Define to prevent recursive inclusion -------------------------------------*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifndef ROBOT_H
#define ROBOT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types ------------------------------------------------------------*/
typedef enum
{
    SERVO1,
    SERVO2,
    SERVO3,
    SERVO4,
    SERVO5,
    SERVO6,
    ERROR_SERVO,
} robot_servo_t;

typedef enum
{
    HORARIO,
    ANTIHORARIO,
    OK,
    ERROR,
} robot_move_t;

typedef enum
{
    READY,
    HOME,
    UNKNOWN,
} robot_status_t;

typedef enum
{
    FIJO,
    PARPADEO,
    APAGADO,
} estado_led_t;

extern estado_led_t estado_led;
extern portMUX_TYPE led_mux;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/**
 * @brief Inicializa los recursos de GPIO e I2C usados por el robot
 * @param None
 * @return None
 */
void robot_init(void);

/**
 * @brief Mueve un servo un paso en la direccion solicitada
 * @param servo Identificador del servo
 * @param move Direccion del movimiento
 */
void move_servo(robot_servo_t servo, robot_move_t move);

#ifdef __cplusplus
}
#endif

#endif /* ROBOT_H */

/* End of file ****************************************************************/