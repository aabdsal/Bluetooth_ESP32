/**
 * @file    lcd.c
 * @author  BLE-SEM
 * @version V0.0
 * @date    2026-05-21
 * @brief   Implementación de un lcd display para mostrar los logs sin necesidad de la terminal
 */

/* Includes ------------------------------------------------------------------*/
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"
#include "lcd.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct 
{
    const uint8_t rs;
    const uint8_t en;
    const uint8_t bl;
    const uint8_t d4;
    const uint8_t d5;
    const uint8_t d6;
    const uint8_t d7;
    const uint8_t bl_active_high; 
} lcd_pcf_map_t;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

// LCD module defines
static const uint8_t LCD_LINEONE   = 0x00;        // start of line 1
static const uint8_t LCD_LINETWO   = 0x40;        // start of line 2

//static const uint8_t LCD_BACKLIGHT = 0x08;
//static const uint8_t LCD_ENABLE    = 0x04;               
static const uint8_t LCD_COMMAND   = 0x00;
static const uint8_t LCD_WRITE     = 0x01;

static const uint8_t LCD_SET_DDRAM_ADDR = 0x80;
//static const uint8_t LCD_READ_BF = 0x40;

static const uint8_t LCD_CLEAR = 0x01;        // replace all characters with ASCII 'space'
static const uint8_t LCD_HOME  = 0x02;        // return cursor to first position on first line

static char *tag = "[LCD]";

static i2c_master_bus_handle_t s_bus = NULL;
static i2c_master_dev_handle_t s_dev = NULL;

static lcd_pcf_map_t g_map =
{
    .rs = 0,  
    .en = 2,
    .d4 = 4, 
    .d5 = 5, 
    .d6 = 6, 
    .d7 = 7,
    .bl = 3, 
    .bl_active_high = 1
};

static int s_sda = -1;
static int s_scl = -1;

static uint8_t s_dev_addr = 0;
static const uint8_t lcd_addr = 0x27;

static const uint8_t SDA_pin = 42;
static const uint8_t SCL_pin = 41;

//static const uint8_t lcd_cols = 16;
static const uint8_t lcd_rows = 2;

/* Private function prototypes -----------------------------------------------*/
static void lcd_writeByte(uint8_t data, uint8_t mode);
static void lcd_write4(uint8_t nib4, uint8_t mode);
static esp_err_t i2c_init(void);

/* Exported functions --------------------------------------------------------*/

void lcd_init()
{
    ESP_ERROR_CHECK(i2c_init());
    vTaskDelay(pdMS_TO_TICKS(50));

    // Reset / 8-bit init sequence (en modo 4-bit se manda 0x03 varias veces)
    lcd_write4(0x03, LCD_COMMAND);
    vTaskDelay(pdMS_TO_TICKS(5));
    lcd_write4(0x03, LCD_COMMAND);

    ets_delay_us(150);
    
    lcd_write4(0x03, LCD_COMMAND);

    // Cambiar a 4-bit
    lcd_write4(0x02, LCD_COMMAND);

    // Function set: 0x28 (4-bit, 2 lines)
    lcd_writeByte(0x28, LCD_COMMAND);

    // Display off
    lcd_writeByte(0x08, LCD_COMMAND);

    // Clear (tarda)
    lcd_writeByte(0x01, LCD_COMMAND);
    vTaskDelay(pdMS_TO_TICKS(2));

    // Entry mode
    lcd_writeByte(0x06, LCD_COMMAND);

    // Display on
    lcd_writeByte(0x0C, LCD_COMMAND);
}

void lcd_setCursor(uint8_t col, uint8_t row)
{
    if (row > lcd_rows - 1) 
    {
        ESP_LOGE(tag, "Cannot write to row %d. Please select a row in the range (0, %d)", row, lcd_rows-1);
        row = lcd_rows - 1;
    }
    uint8_t row_offsets[] = {LCD_LINEONE, LCD_LINETWO};
    lcd_writeByte(LCD_SET_DDRAM_ADDR | (col + row_offsets[row]), LCD_COMMAND);
}

void lcd_writeChar(char c)
{
    lcd_writeByte(c, LCD_WRITE);                                        
}

void lcd_writeStr(char* str)
{
    while (*str) 
    {
        lcd_writeChar(*str++);
    }
}

void lcd_home(void)
{
    lcd_writeByte(LCD_HOME, LCD_COMMAND);
    vTaskDelay(pdMS_TO_TICKS(2));                                   
}

void lcd_clearScreen(void)
{
    lcd_writeByte(LCD_CLEAR, LCD_COMMAND);
    vTaskDelay(pdMS_TO_TICKS(10));                                   
}

/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/**
 * @brief  Inicializa el bus y el dispositivo I2C para el LCD.
 * 
 * Si el bus y el dispositivo ya están inicializados con los mismos parámetros, no realiza ninguna acción.
 * Si cambian los pines o la dirección, elimina y vuelve a crear los recursos necesarios.
 * 
 * @retval ESP_OK si la inicialización fue exitosa.
 * @retval error de ESP-IDF en caso de fallo.
 */
static esp_err_t i2c_init(void)
{
    ESP_LOGI(tag, "Creando bus I2C SDA=%d SCL=%d", SDA_pin, SCL_pin);
    
    // Si ya está creado con mismos pines y misma dirección, no hagas nada
    if (s_bus && s_dev && s_dev_addr == lcd_addr && s_sda == SDA_pin && s_scl == SCL_pin) 
    {
        return ESP_OK;
    }

    // Si cambia la dirección, elimina el device previo
    if (s_dev) 
    {
        i2c_master_bus_rm_device(s_dev);
        s_dev = NULL;
    }

    // Si cambian pines, elimina el bus y créalo de nuevo
    if (s_bus && (s_sda != SDA_pin || s_scl != SCL_pin)) 
    {
        i2c_del_master_bus(s_bus);
        s_bus = NULL;
    }

    if (!s_bus) 
    {
        i2c_master_bus_config_t bus_cfg = 
        {
            .i2c_port = I2C_NUM_0,
            .sda_io_num = SDA_pin,
            .scl_io_num = SCL_pin,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .flags.enable_internal_pullup = 1,
        };
        ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_cfg, &s_bus), tag, "i2c_new_master_bus fallo");

        s_sda = SDA_pin;
        s_scl = SCL_pin;
    }

    i2c_device_config_t dev_cfg = 
    {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = lcd_addr,      
        .scl_speed_hz = 50000,
    };

    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(s_bus, &dev_cfg, &s_dev), tag, "add_device fallo");
    s_dev_addr = lcd_addr;

    return ESP_OK;
}

/******************************************************************************/
/**
 * @brief  Empaqueta un nibble de datos y el bit RS en el formato requerido por el expansor PCF8574.
 * @param  nib4 Nibble de datos (4 bits) a enviar (D4..D7).
 * @param  rs   Valor lógico del bit RS (0: comando, 1: datos).
 * @retval Byte listo para enviar al expansor PCF8574.
 */
static uint8_t pack4(uint8_t nib4, bool rs)
{
    uint8_t out = 0;

    // Datos D4..D7 según mapa
    if (nib4 & 0x01) 
    {
        out |= (1 << g_map.d4);
    }
    
    if (nib4 & 0x02) 
    {
        out |= (1 << g_map.d5);
    }
    
    if (nib4 & 0x04) 
    {
        out |= (1 << g_map.d6);
    }
    
    if (nib4 & 0x08) 
    {
        out |= (1 << g_map.d7);
    }

    // RS según mapa
    if (rs) 
    {
        out |= (1 << g_map.rs);
    }
    // Backlight según mapa (equivale a 0x08 porque bl=3)
    out |= (1 << g_map.bl);

    return out;
}

/******************************************************************************/
/**
 * @brief  Envía una secuencia de bytes al expansor PCF8574 a través del bus I2C.
 * @param  seq Puntero al array de bytes a enviar.
 * @param  n   Número de bytes a enviar.
 * @retval None
 */
static void pcf_write_seq(const uint8_t *seq, size_t n)
{
    if (!s_dev) 
    {
        ESP_LOGE(tag, "s_dev NULL (i2c_init no creó el device)");
        return;
    }
    esp_err_t ret = i2c_master_transmit(s_dev, seq, n, 1000);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(tag, "i2c_master_transmit fallo: %s (%d)", esp_err_to_name(ret), ret);
    }
}

/******************************************************************************/
/**
 * @brief  Envía un nibble (4 bits) al LCD en modo 4 bits.
 * @param  nib4 Nibble de datos a enviar (D4..D7).
 * @param  mode Modo de operación (LCD_COMMAND o LCD_WRITE).
 * @retval None
 */
static void lcd_write4(uint8_t nib4, uint8_t mode)
{
    bool rs = (mode == LCD_WRITE);
    uint8_t gpio = pack4(nib4, rs);          // datos + RS + BL (RW implícito a 0)
    uint8_t en_mask = (1 << g_map.en);

    uint8_t seq[2] = 
    {
        (uint8_t)(gpio | en_mask),           // E = 1
        gpio                                // E = 0
    };

    pcf_write_seq(seq, 2);

    ets_delay_us(80);  // margen tras cada nibble
}

/******************************************************************************/
/**
 * @brief  Envía un byte completo al LCD, transmitiendo primero el nibble alto y luego el bajo.
 * @param  data Byte de datos/comando a enviar.
 * @param  mode Modo de operación (LCD_COMMAND o LCD_WRITE).
 * @retval None
 */
static void lcd_writeByte(uint8_t data, uint8_t mode)
{
    lcd_write4((data >> 4) & 0x0F, mode);  // nibble alto
    lcd_write4(data & 0x0F, mode);         // nibble bajo
    ets_delay_us(80);  
}

/* End of file ****************************************************************/