/**
 * @file gap_svc.c
 * @author BLE-SEM
 * @version V0.0
 * @date 2026-05-14
 * @brief Implementacion del servicio GAP para anuncio y emparejamiento BLE
 */

/* Includes ------------------------------------------------------------------*/
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

#include "host/ble_hs.h"
#include "host/util/util.h"

#include "console/console.h"

#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "gap_svc.h"
#include "gatt_svr.h"
#include "robot.h"

/* Private define ------------------------------------------------------------*/
#define GAP_SVC_IO_CAP BLE_SM_IO_CAP_NO_IO

/* Private variables ---------------------------------------------------------*/
static const char *tag = "GAP-BLE";

static int gap_svc_gap_event(struct ble_gap_event *event, void *arg);

static uint8_t own_addr_type;

static bool s_ble_synced  = false;

static uint16_t s_conn_handle = BLE_HS_CONN_HANDLE_NONE;

void ble_store_config_init(void);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Imprime una descripcion legible de una conexion BLE
 * @param desc Descriptor de conexion proporcionado por NimBLE
 */
static void gap_svc_print_conn_desc(struct ble_gap_conn_desc *desc)
{
    MODLOG_DFLT(
        INFO,
        "handle=%d our_ota_addr_type=%d peer_ota_addr_type=%d "
        "conn_itvl=%d conn_latency=%d supervision_timeout=%d "
        "encrypted=%d authenticated=%d bonded=%d\n",
        desc->conn_handle,
        desc->our_ota_addr.type,
        desc->peer_ota_addr.type,
        desc->conn_itvl,
        desc->conn_latency,
        desc->supervision_timeout,
        desc->sec_state.encrypted,
        desc->sec_state.authenticated,
        desc->sec_state.bonded
    );
}

/**
 * @brief Inicia el anuncio BLE con los parametros por defecto de conexion
 * @param None
 * @return None
 */
static void gap_svc_advertise(void)
{
    if (!s_ble_synced)
    {
        ESP_LOGW(tag, "No se puede anunciar todavia: host BLE no sincronizado");
        return;
    }

    if (ble_gap_adv_active())
    {
        ESP_LOGI(tag, "Advertising ya estaba activo");
        return;
    }

    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;

    const char *name;
    int rc;

    memset(&fields, 0, sizeof(fields));

    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    name = ble_svc_gap_device_name();

    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

    fields.uuids16 = (ble_uuid16_t[])
    {
        BLE_UUID16_INIT(GATT_SVR_SVC_ALERT_UUID)
    };

    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);

    if (rc != 0)
    {
        MODLOG_DFLT(ERROR, "error setting advertisement data; rc=%d\n", rc);
        return;
    }

    memset(&adv_params, 0, sizeof(adv_params));

    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    rc = ble_gap_adv_start(
        own_addr_type,
        NULL,
        BLE_HS_FOREVER,
        &adv_params,
        gap_svc_gap_event,
        NULL
    );

    if (rc != 0)
    {
        MODLOG_DFLT(ERROR, "error enabling advertisement; rc=%d\n", rc);
        return;
    }

    ESP_LOGI(tag, "BLE advertising ACTIVADO");
}

/**
 * @brief Gestiona los eventos GAP reportados por NimBLE
 * @param None
 * @return 0 si no ha habido problema
 */
static int gap_svc_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc desc;
    int rc;

    switch (event->type)
    {
        case BLE_GAP_EVENT_CONNECT:
            MODLOG_DFLT(
                INFO,
                "connection %s; status=%d\n",
                event->connect.status == 0 ? "established" : "failed",
                event->connect.status
            );

            if (event->connect.status == 0)
            {
                s_conn_handle = event->connect.conn_handle;
                taskENTER_CRITICAL(&led_mux);
                estado_led = FIJO;
                taskEXIT_CRITICAL(&led_mux);

                rc = ble_gap_conn_find(event->connect.conn_handle, &desc);

                if (rc == 0)
                {
                    gap_svc_print_conn_desc(&desc);
                }
            }
            else
            {
                s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
                gap_svc_advertise();
            }
            return 0;

        case BLE_GAP_EVENT_DISCONNECT:
            MODLOG_DFLT(
                INFO,
                "disconnect; reason=%d\n",
                event->disconnect.reason
            );
            taskENTER_CRITICAL(&led_mux);
            estado_led = PARPADEO;
            taskEXIT_CRITICAL(&led_mux);

            s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
            gap_svc_advertise();
            return 0;

        case BLE_GAP_EVENT_ADV_COMPLETE:
            MODLOG_DFLT(
                INFO,
                "advertise complete; reason=%d\n",
                event->adv_complete.reason
            );

            gap_svc_advertise();
            return 0;

        case BLE_GAP_EVENT_SUBSCRIBE:
            MODLOG_DFLT(
                INFO,
                "subscribe event; conn_handle=%d attr_handle=%d "
                "reason=%d prevn=%d curn=%d previ=%d curi=%d\n",
                event->subscribe.conn_handle,
                event->subscribe.attr_handle,
                event->subscribe.reason,
                event->subscribe.prev_notify,
                event->subscribe.cur_notify,
                event->subscribe.prev_indicate,
                event->subscribe.cur_indicate
            );
            return 0;

        case BLE_GAP_EVENT_MTU:
            MODLOG_DFLT(
                INFO,
                "mtu update event; conn_handle=%d cid=%d mtu=%d\n",
                event->mtu.conn_handle,
                event->mtu.channel_id,
                event->mtu.value
            );
            return 0;

        default:
            return 0;
    }
}

/* Exported functions --------------------------------------------------------*/

void gap_svc_on_reset(int reason)
{
    s_ble_synced = false;
    MODLOG_DFLT(ERROR, "Resetting state; reason=%d\n", reason);
}

void gap_svc_on_sync(void)
{
    int rc;

    rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);

    rc = ble_hs_id_infer_auto(0, &own_addr_type);

    if (rc != 0)
    {
        MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
        return;
    }

    uint8_t addr_val[6] = {0};

    rc = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);

    if (rc == 0)
    {
        MODLOG_DFLT(
            INFO,
            "Device Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
            addr_val[5],
            addr_val[4],
            addr_val[3],
            addr_val[2],
            addr_val[1],
            addr_val[0]
        );
    }

    s_ble_synced = true;

    ESP_LOGI(tag, "BLE host sincronizado");

    gap_svc_advertise();
}

void gap_svc_host_task(void *pvParameters)
{
    ESP_LOGI(tag, "BLE Host Task Started");

    nimble_port_run();

    nimble_port_freertos_deinit();
}

void gap_svc_start(void)
{
    nimble_port_freertos_init(gap_svc_host_task);
}

void gap_svc_set_device_name(const char *name)
{
    int rc = ble_svc_gap_device_name_set(name);

    if (rc != 0)
    {
        ESP_LOGE(tag, "Error configurando nombre BLE; rc=%d", rc);
    }
}

void gap_svc_init(void)
{
    esp_err_t ret = nvs_flash_init(); /* Inicializa NVS: se usa para guardar datos de calibracion PHY */
    
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    if (ret != ESP_OK) 
    {
        ESP_LOGE(tag, "Failed to init nvs %d ", ret);
        return;
    }

    ret = nimble_port_init();
    if (ret != ESP_OK) 
    {
        ESP_LOGE(tag, "Failed to init nimble %d ", ret);
        return;
    }

    /* Inicializa la configuracion del host de NimBLE. */
    ble_hs_cfg.reset_cb = gap_svc_on_reset;
    ble_hs_cfg.sync_cb = gap_svc_on_sync;
    ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
    ble_hs_cfg.sm_io_cap = GAP_SVC_IO_CAP;
    ble_hs_cfg.sm_sc = 0;

    ble_store_config_init();
}

/* End of file ***************************************************************/