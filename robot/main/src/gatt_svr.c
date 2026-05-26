/**
 * @file    gatt_svr.c
 * @author  BLE-SEM
 * @version V0.0
 * @date    2026-05-14
 * @brief   Implementacion del servidor GATT
 */

/* Includes ------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "services/ans/ble_svc_ans.h"

#include "gatt_svr.h"
#include "robot.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Client Characteristic Configuration Descriptor = CCCD */
/* Maximum number of characteristics with the notify flag */
#define MAX_NOTIFY 5

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static const char *gatt = "[GATT]";
static const char *mando = "[MANDO]";
static const char *robot = "[ROBOT]";

static robot_move_t move_val;
static robot_servo_t servo_val;
static robot_status_t status_val;

static const ble_uuid128_t robot_controller_uuid =
    BLE_UUID128_INIT(0xaa, 0xaa, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x00,
                     0xCA, 0x00, 0x00, 0xea, 0x00, 0xea, 0x00, 0xea);

static uint16_t command_chr_val_handle;
static const ble_uuid128_t command_chr_uuid =
    BLE_UUID128_INIT(0xbb, 0xbb, 0xbb, 0xcc, 0xcc, 0xcc, 0xcc, 0xaa,
                     0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xea, 0x00, 0xea);

static uint16_t status_chr_val_handle;
static const ble_uuid128_t status_chr_uuid =
    BLE_UUID128_INIT(0x13, 0x13, 0x13, 0x47, 0x47, 0x47, 0x47, 0xaa,
                     0x0, 0x0, 0x0, 0x0, 0x0, 0xea, 0x35, 0xea);

/* A custom descriptor */
static uint8_t gatt_svr_dsc_val;

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief Valida y procesa un comando de control del robot recibido por BLE
 * @param msg Cadena de comando bruta
 * @param len Longitud del comando
 * @return 0 si tiene exito, -1 si falla el analisis o la validacion
 */
static int handle_robot_message(const char *msg, uint16_t len, robot_servo_t *servo, robot_move_t *move)
{

    int servo_num;
    char mov_char;
    if (sscanf(msg, "S%d,A:%c", &servo_num, &mov_char) == 2) 
    {
        if (servo_num >= 1 && servo_num <= 6) 
        {
            *servo = (robot_servo_t)(servo_num - 1); // Porque en nuestro enum SERVO1=0, SERVO2=1, etc.
            ESP_LOGI(robot, "Servo seleccionado: %d", *servo);
        } else 
        {
            ESP_LOGE(gatt, "Número de servo inválido: %d", servo_num);
            return -1;
        }

        switch (mov_char)
        {
            case 'H':
                *move = HORARIO;
                ESP_LOGI(robot, "Movimiento HORARIO");
                break;
            case 'A':
                *move = ANTIHORARIO;
                ESP_LOGI(robot, "Movimiento ANTIHORARIO");
                break;
            case 'O':
                *move = OK;
                ESP_LOGI(robot, "Movimiento OK (confirmación)");
                break;
            default:
                ESP_LOGE(gatt, "Movimiento no reconocido: %c", mov_char);
                return -1;
        }
    } else 
    {
        ESP_LOGE(gatt, "Formato de mensaje incorrecto: %s", msg);
        return -1;
    }

    return 0; // Mensaje manejado exitosamente
}

static int gatt_svc_access(uint16_t conn_handle, uint16_t attr_handle,
                            struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def gatt_svr_svcs[] = 
{
    {
        /*** Robot Control Service ***/
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &robot_controller_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]) 
        { 
            {
                /* Command characteristic: writable from BLE client */
                .uuid = &command_chr_uuid.u,
                .access_cb = gatt_svc_access,
                .flags = BLE_GATT_CHR_F_WRITE, 
                .val_handle = &command_chr_val_handle,
            }, 
            {
                /* Status characteristic: readable from BLE client */
                .uuid = &status_chr_uuid.u,
                .access_cb = gatt_svc_access,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY, 
                .val_handle = &status_chr_val_handle,
            },
            {
                0,
            }
        },
    },
    {
        0, /* No more services. */
    },
};

/**
 * Access callback whenever a characteristic/descriptor is read or written to.
 * Here reads and writes need to be handled.
 * ctxt->op tells weather the operation is read or write and
 * weather it is on a characteristic or descriptor,
 * ctxt->dsc->uuid tells which characteristic/descriptor is accessed.
 * attr_handle give the value handle of the attribute being accessed.
 * Accordingly do:
 *     Append the value to ctxt->om if the operation is READ
 *     Write ctxt->om to the value if the operation is WRITE
 **/
static int gatt_svc_access(uint16_t conn_handle, uint16_t attr_handle,
                struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc;
    uint8_t mensaje[20];
    uint16_t len;
    robot_servo_t servo;
    robot_move_t move;

    switch (ctxt->op) 
    {
        case BLE_GATT_ACCESS_OP_WRITE_CHR:

            rc = ble_hs_mbuf_to_flat(ctxt->om, mensaje, sizeof(mensaje), &len);            
            
            if (rc != 0)            
            {                
                ESP_LOGE(gatt, "Error decodificando el mensaje, rc=%d", rc);                
                return BLE_ATT_ERR_UNLIKELY;            
            }

            char traducido[21];
            for(int idx = 0; idx < len; idx++)
            {
                traducido[idx] = (char)mensaje[idx];
                
            }

            traducido[len] = '\0';
            rc = handle_robot_message(traducido, len, &servo, &move);
            if (rc != 0)            
            {                
                ESP_LOGE(gatt, "Error decodificando el mensaje, rc=%d", rc);                
                return BLE_ATT_ERR_UNLIKELY;            
            }
            if (move != OK && servo_val != ERROR_SERVO) 
            {
                move_val = move;
                vTaskDelay(pdMS_TO_TICKS(10));
                
                move_servo(servo_val, move_val);
                char c = ' ';
                if (move_val == HORARIO)
                {
                    c = 'H';
                }
                else if (move_val == ANTIHORARIO)
                {
                    c = 'A';
                }
                ESP_LOGI(robot, "Movimiento ejecutado: servo %d, movimiento %c", servo_val + 1, c);
            }
            else if (move == OK)
            {
                servo_val = servo;
                ESP_LOGI(robot, "Servo seleccionado: %d", servo_val + 1);
            }
            else 
            {
                ESP_LOGE(gatt, "Servo no seleccionado aun");
                return BLE_ATT_ERR_UNLIKELY;
            }

            ESP_LOGI(robot, "\n");

            return 0;

        case BLE_GATT_ACCESS_OP_READ_CHR:
            
            if (attr_handle != status_chr_val_handle)
            {
                return BLE_ATT_ERR_UNLIKELY;
            }
            
            os_mbuf_append(ctxt->om, &status_val, sizeof(uint8_t));
            ESP_LOGI(mando, "Operacion de lectura exitososa, se parlar?");
            
            return 0;
        
        default:
            return BLE_ATT_ERR_UNLIKELY;      
    }
}

void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    char buf[BLE_UUID_STR_LEN];

    switch (ctxt->op) 
    {
        case BLE_GATT_REGISTER_OP_SVC:
            MODLOG_DFLT(DEBUG, "registered service %s with handle=%d\n",
                        ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                        ctxt->svc.handle);
            break;

        case BLE_GATT_REGISTER_OP_CHR:
            MODLOG_DFLT(DEBUG, "registering characteristic %s with "
                        "def_handle=%d val_handle=%d\n",
                        ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                        ctxt->chr.def_handle,
                        ctxt->chr.val_handle);
            break;

        case BLE_GATT_REGISTER_OP_DSC:
            MODLOG_DFLT(DEBUG, "registering descriptor %s with handle=%d\n",
                        ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                        ctxt->dsc.handle);
            break;

        default:
            assert(0);
            break;
    }
}

void gatt_svr_init(void)
{
    int rc;

    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_svc_ans_init();

    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) 
    {
        return;
    }

    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) 
    {
        return;
    }

    /* Setting a value for the read-only descriptor */
    gatt_svr_dsc_val = 0x99;
    status_val = READY;

}