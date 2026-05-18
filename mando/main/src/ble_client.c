/**
 * @file    ble_client.c
 * @author  BLE-SEM
 * @version V0.0
 * @date    2026-05-16
 * @brief   Implementacion del cliente BLE.
 */

/* Includes ------------------------------------------------------------------*/

#include <string.h>
#include "ble_client.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "host/ble_gattc.h"
#include "services/gap/ble_svc_gap.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static const char *TAG = "BLE_CLIENT";
static char name_device[32] = "BLE_CLIENT";

static uint16_t conn_handle = 0;
static uint16_t char_handle = 0;

//UUIDs del robot
static const ble_uuid128_t robot_controller_uuid = 
    BLE_UUID128_INIT(0xaa, 0xaa, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCA, 0x00, 0x00, 0xea, 0x00, 0xea, 0x00, 0xea);

static const ble_uuid128_t command_chr_uuid = 
    BLE_UUID128_INIT(0xbb, 0xbb, 0xbb, 0xcc, 0xcc, 0xcc, 0xcc, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xea, 0x00, 0xea);

/* Private function prototypes -----------------------------------------------*/

static void ble_host_task(void *param);
static int gap_event(struct ble_gap_event *event, void *arg);
static void ble_app_on_sync(void);

/* Exported functions --------------------------------------------------------*/


void ble_client_set_device_name(const char *name)
{
    strncpy(name_device, name, sizeof(name_device) - 1);
    name_device[sizeof(name_device) - 1] = '\0';
}


void ble_client_init(void)
{

    esp_err_t ret = nvs_flash_init(); /* Inicializa NVS: se usa para guardar datos de calibracion PHY */
    
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    nimble_port_init();
    int rc = ble_svc_gap_device_name_set(name_device);
    
    if (rc != 0) 
    {
        ESP_LOGE(TAG, "ble_svc_gap_device_name_set fallo, rc=%d", rc);
    }
    
    ble_hs_cfg.sync_cb = ble_app_on_sync;

    nimble_port_freertos_init(ble_host_task);
}

void ble_send(char *msg)
{
    if (conn_handle == 0 || char_handle == 0) 
    {
        ESP_LOGW(TAG, "No conectado aún");
        return;
    }

    struct os_mbuf *om = ble_hs_mbuf_from_flat(msg, strlen(msg));

    int rc = ble_gattc_write_no_rsp(conn_handle, char_handle, om);
    
    if (rc != 0)
    {
        ESP_LOGE(TAG, "Error enviando: %d", rc);
    }
    
    ESP_LOGI(TAG, "Enviado: %s", msg);
    
}

/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/**
 * @brief  Envoltorio de la tarea del host NimBLE.
 * @param  param Parametro de tarea (no usado).
 * @retval None
 */
static void ble_host_task(void *param)
{
    (void)param;
    nimble_port_run();
    nimble_port_freertos_deinit();
}

/******************************************************************************/

/**
 * @brief Callback que se ejecuta cuando encuentra una característica
 */
static int chr_discovery_cb(uint16_t conn_handle, const struct ble_gatt_error *error, 
                            const struct ble_gatt_chr *chr, void *arg) 
{
    if (error->status == 0) {
        // Compara si el buzón encontrado es exactamente el de comandos
        if (ble_uuid_cmp(&chr->uuid.u, &command_chr_uuid.u) == 0) {
            char_handle = chr->val_handle; // ¡Guarda el número real asignado!
            ESP_LOGI(TAG, "¡Característica de comando encontrada! Handle asignado: %d", char_handle);
        }
    }
    return 0;
}

/**
 * @brief Callback que se ejecuta cuando encuentra un servicio
 */
static int svc_discovery_cb(uint16_t conn_handle, const struct ble_gatt_error *error, 
                            const struct ble_gatt_svc *svc, void *arg) 
{
    if (error->status == 0) {
        // Compara si el servicio encontrado es el "Robot Control Service"
        if (ble_uuid_cmp(&svc->uuid.u, &robot_controller_uuid.u) == 0) {
            ESP_LOGI(TAG, "Servicio del robot encontrado. Buscando características...");
            // Una vez hallado el servicio, pregunta por sus características
            ble_gattc_disc_all_chrs(conn_handle, svc->start_handle, svc->end_handle, chr_discovery_cb, NULL);
        }
    }
    return 0;
}
/**
 * @brief  Callback de eventos GAP.
 * @param  event Datos del evento GAP.
 * @param  arg Argumento de usuario.
 * @retval 0 siempre.
 */
static int gap_event(struct ble_gap_event *event, void *arg)
{
    (void)arg;
    switch (event->type) // TODO: Ignorar el intento de conexion a otros deispositivos si el interruptor esta en Off
    {
        case BLE_GAP_EVENT_DISC:
        {
            struct ble_hs_adv_fields fields;
            ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data);

            if (fields.name != NULL)
            {
                
            if (fields.name_len == strlen("ESP32S3_ROBOT") && memcmp(fields.name, "ESP32S3_ROBOT", fields.name_len) == 0)

                {
                    ESP_LOGI(TAG, "Robot encontrado -> conectando");

                    ble_gap_disc_cancel();

                    struct ble_gap_conn_params conn_params = 
                    {
                        .scan_itvl = 0x0010,
                        .scan_window = 0x0010,
                        .itvl_min = 0x0018,
                        .itvl_max = 0x0028,
                        .latency = 0,
                        .supervision_timeout = 0x0100,
                    };

                    ble_gap_connect(BLE_OWN_ADDR_PUBLIC,
                                    &event->disc.addr,
                                    30000,
                                    &conn_params,
                                    gap_event,
                                    NULL);
                }
            }
            return 0;
        }

        case BLE_GAP_EVENT_CONNECT:
        {
            if (event->connect.status == 0)
            {
                conn_handle = event->connect.conn_handle;
                ESP_LOGI(TAG, "Conectado al robot! Iniciando descubrimiento de servicios");
                //EL ROBOT NOS LISTA SUS SERVICIOS
                ble_gattc_disc_all_svcs(conn_handle, svc_discovery_cb, NULL);
            }
            
            else
            {
                ble_gap_disc_cancel();
                ESP_LOGI(TAG, "Fallo al conectar, reintentando...");
                ble_app_on_sync();
            }

            return 0;
        }
        case BLE_GAP_EVENT_DISCONNECT:
        {
            ESP_LOGI(TAG, "Desconectado del robot, reintentando...");
            conn_handle = 0;
            char_handle = 0;
            ble_app_on_sync();
            break;
        }

        default:
            return 0;
    }
}

/******************************************************************************/
/**
 * @brief  Callback de sincronizacion para iniciar escaneo.
 * @param  None
 * @retval None
 */
static void ble_app_on_sync(void)
{
    ESP_LOGI(TAG, "Escaneando BLE...");

    struct ble_gap_disc_params disc_params = 
    {
        .itvl = 0,
        .window = 0,
        .filter_policy = 0,
        .passive = 0
    };

    ble_gap_disc(BLE_OWN_ADDR_PUBLIC, BLE_HS_FOREVER,
                 &disc_params, gap_event, NULL); // BLE_HS_FOREVER per a que no pare mai, millor aixo que NULL o 0 que no se que pot fer
}

/* End of file ****************************************************************/
