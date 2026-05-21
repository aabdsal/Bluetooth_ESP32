/**
 * @file    lcd.h
 * @author  BLE-SEM
 * @version V0.0
 * @date    2026-05-16
 * @brief   Interfaz de botones para el mando.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef LCD_H
#define LCD_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/******************************************************************************/
/**
 * @brief  Inicializa el LCD con la dirección y pines especificados.
 * @param  None
 * @retval None
 */
void lcd_init();

/******************************************************************************/
/**
 * @brief  Posiciona el cursor en la columna y fila indicadas.
 * @param  col Columna destino.
 * @param  row Fila destino.
 * @retval None
 */
void lcd_setCursor(uint8_t col, uint8_t row);

/******************************************************************************/
/**
 * @brief  Lleva el cursor a la posición inicial (0,0).
 * @retval None
 */
void lcd_home(void);

/******************************************************************************/
/**
 * @brief  Limpia la pantalla del LCD y coloca el cursor en (0,0).
 * @retval None
 */
void lcd_clearScreen(void);

/******************************************************************************/
/**
 * @brief  Escribe un carácter en la posición actual del cursor.
 * @param  c Carácter a escribir.
 * @retval None
 */
void lcd_writeChar(char c);

/******************************************************************************/
/**
 * @brief  Escribe una cadena de texto en el LCD desde la posición actual del cursor.
 * @param  str Cadena de texto terminada en null.
 * @retval None
 */
void lcd_writeStr(char* str); 

#ifdef __cplusplus
}
#endif

#endif /* LCD_H */

/*** End of file **************************************************************/