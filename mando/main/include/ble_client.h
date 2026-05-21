/**
 * @file    ble_client.h
 * @author  BLE-SEM
 * @version V0.0
 * @date    2026-05-16
 * @brief   Interfaz publica del cliente BLE.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef BLE_CLIENT_H
#define BLE_CLIENT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/******************************************************************************/
/**
 * @brief  Inicializa el cliente BLE y comienza el escaneo.
 * @param  None
 * @retval None
 */
void ble_client_init(void);

/******************************************************************************/
/**
 * @brief  Envia un mensaje al dispositivo conectado.
 * @param  msg Puntero al buffer del mensaje terminado en null.
 * @retval None
 */
void ble_send(char *msg);

/******************************************************************************/
/**
 * @brief  Configura el nombre de dispositivo GAP local.
 * @param  name Cadena de nombre terminada en null.
 * @retval None
 */
void ble_client_set_device_name(const char *name);

/******************************************************************************/
/**
 * @brief  Indica si el cliente BLE está actualmente escaneando dispositivos.
 * @retval true si está escaneando, false en caso contrario.
 */
bool ble_client_is_scanning();

/******************************************************************************/
/**
 * @brief  Activa o desactiva el escaneo de dispositivos BLE.
 * @param  scan true para iniciar el escaneo, false para detenerlo.
 * @retval None
 */
void ble_client_set_scanning(bool scan);

/******************************************************************************/
/**
 * @brief  Indica si el cliente BLE está actualmente conectado a un dispositivo.
 * @retval true si está conectado, false en caso contrario.
 */
bool ble_client_is_connected();

/******************************************************************************/
/**
 * @brief  Establece o elimina el estado de conexión BLE del cliente.
 * @param  connect true para marcar como conectado, false para desconectado.
 * @retval None
 */
void ble_client_set_connection(bool connect);

#ifdef __cplusplus
}
#endif

#endif
/*** End of file **************************************************************/