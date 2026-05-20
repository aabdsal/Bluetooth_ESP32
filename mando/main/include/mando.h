/**
 * @file    mando.h
 * @author  BLE-SEM
 * @version V0.0
 * @date    2026-05-16
 * @brief   Interfaz de botones para el mando.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MANDO_H
#define MANDO_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/******************************************************************************/
/**
 * @brief  Inicializa GPIOs de botones y sus ISR.
 * @param  None
 * @retval None
 */
void mando_init(void);

/******************************************************************************/
/**
 * @brief  Lee y limpia el evento del boton SELECT.
 * @param  None
 * @retval true si se pulso desde la ultima lectura, si no false.
 */
bool mando_btn_select_read(void);

/******************************************************************************/
/**
 * @brief  Lee y limpia el evento del boton OK.
 * @param  None
 * @retval true si se pulso desde la ultima lectura, si no false.
 */
bool mando_btn_ok_read(void);

/******************************************************************************/
/**
 * @brief  Lee y limpia el evento del boton RIGHT.
 * @param  None
 * @retval true si se pulso desde la ultima lectura, si no false.
 */
bool mando_btn_right_read(void);

/******************************************************************************/
/**
 * @brief  Lee y limpia el evento del boton LEFT.
 * @param  None
 * @retval true si se pulso desde la ultima lectura, si no false.
 */
bool mando_btn_left_read(void);

/******************************************************************************/
/**
 * @brief  Lee y limpia el evento del interruptor BLE_EN.
 * @param  None
 * @retval true si ha cambiado de estado desde la ultima lectura, si no false.
 */
bool mando_sw_ble_en_event_read(void);

/*
No tiene que ver con esto pero lo dejo por aqui para hacer debug:

ESP-PROG     ESP32-S3
----------------------
TCK      ->  GPIO39
TMS      ->  GPIO42
TDI      ->  GPIO41
TDO      ->  GPIO40
GND      ->  GND

*/

#ifdef __cplusplus
}
#endif

#endif
/*** End of file **************************************************************/