/**
 * @file    hd44780.h
 * @author  BLE-SEM
 * @version V0.0
 * @date    2026-05-16
 * @brief   Interfaz de botones para el mando.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef HD44780
#define HD44780

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
 * @param  addr Dirección del LCD en el bus I2C.
 * @param  dataPin Pin de datos.
 * @param  clockPin Pin de reloj.
 * @param  cols Número de columnas del LCD.
 * @param  rows Número de filas del LCD.
 * @retval None
 */
void LCD_init(uint8_t addr, uint8_t dataPin, uint8_t clockPin, uint8_t cols, uint8_t rows);

/******************************************************************************/
/**
 * @brief  Posiciona el cursor en la columna y fila indicadas.
 * @param  col Columna destino.
 * @param  row Fila destino.
 * @retval None
 */
void LCD_setCursor(uint8_t col, uint8_t row);

/******************************************************************************/
/**
 * @brief  Lleva el cursor a la posición inicial (0,0).
 * @retval None
 */
void LCD_home(void);

/******************************************************************************/
/**
 * @brief  Limpia la pantalla del LCD y coloca el cursor en (0,0).
 * @retval None
 */
void LCD_clearScreen(void);

/******************************************************************************/
/**
 * @brief  Escribe un carácter en la posición actual del cursor.
 * @param  c Carácter a escribir.
 * @retval None
 */
void LCD_writeChar(char c);

/******************************************************************************/
/**
 * @brief  Escribe una cadena de texto en el LCD desde la posición actual del cursor.
 * @param  str Cadena de texto terminada en null.
 * @retval None
 */
void LCD_writeStr(char* str); 

#ifdef __cplusplus
}
#endif

#endif
/*** End of file **************************************************************/