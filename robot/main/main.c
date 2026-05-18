/**
 * @file main.c
 * @author BLE-SEM
 * @version V0.0
 * @date 2026-05-14
 * @brief Punto de entrada principal de la aplicacion del robot
 */

/* Includes ------------------------------------------------------------------*/

#include "esp_err.h"
#include "nvs_flash.h"

#include "nimble/nimble_port.h"
#include "host/ble_hs.h"

#include "gap_svc.h"
#include "gatt_svr.h"
#include "robot.h"

/* Exported functions --------------------------------------------------------*/

void app_main(void)
{
    gap_svc_init();
    gatt_svr_init();
    gap_svc_set_device_name("ESP32S3_ROBOT");
    gap_svc_start();
    robot_init();
}

/* End of file ***************************************************************/