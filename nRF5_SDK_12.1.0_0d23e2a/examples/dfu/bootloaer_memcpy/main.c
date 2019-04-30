/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 * @defgroup flashwrite_example_main main.c
 * @{
 * @ingroup flashwrite_example
 *
 * @brief This file contains the source code for a sample application using the Flash Write Application.
 *a
 */

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "nrf.h"
#include "app_error.h"
#include "nordic_common.h"
#include "app_uart_ex.h"
#include "crc16.h"
#include "nrf_mbr.h"
#include "nrf_bootloader_app_start.h"
#include "nrf_bootloader_info.h"

#define MAIN_DBG(...)//    UartSendStringFmt
#define CONSOLE_OUT UartSendStringFmt

#define FLASH_PAGE_SIZE         NRF_FICR->CODEPAGESIZE
#define APP_START_PAGE          31
#define APP_PAGE_SIZE           32
#define APP_SWAP_START_PAGE     63
#define APP_SWAP_PAGE_SIZE      32
#define APP_DATA_START_PAGE     95
#define APP_DATA_PAGE_SIZE      29
#define BOOTLOADER_START_PAGE   124
#define BOOTLOADER_PAGE_SIZE    4

bool b_user_login = false;
bool b_user_cmd = false;

typedef bool(*atcmd_function)(uint8_t * param, uint16_t len);
#define ATCMD_MAX_SIZE  16
typedef struct{
    char cmd[ATCMD_MAX_SIZE];
    uint8_t actual_cmd_size;
    uint8_t login;
    atcmd_function fun;
}ATCMD_def_t;

const uint16_t data_array_size = 512;
static uint8_t data_array[data_array_size] = {0};
static uint16_t data_index = 0;
static uint8_t dcb_stage = 0;

static uint8_t atcmd_rev_size = 0;
static uint8_t atcmd_end = 0;
static uint8_t atcmd_raw_rev = 0;
static uint32_t atcmd_raw_rev_time_out = 0;

static uint32_t * app_swap;

#define SWAP_MAGIC_NUMBER   0x12345678
typedef struct{
    uint32_t flag;
    uint32_t word_len;
    uint32_t crc16;
    uint32_t res;
}app_swap_info_t;

static app_swap_info_t swap_info;
static const app_swap_info_t * pFlashInfo;
uint32_t *swap_app_addr;

typedef struct{
    uint8_t DeviceID[8];
    uint32_t Server_port;         
    uint8_t  Server_IPAddr[32];
}system_setting_info_t;
#define SYSTEM_SETTINGS_ADDRESS     (0x0007F000UL)
/**@brief   This variable reserves a codepage for mbr parameters, to ensure the compiler doesn't
 *          locate any code or variables at his location.
 */
#if defined ( __CC_ARM )

typedef enum{
PID_SERVICE                 = 0x0000, //服务器
PID_IR_WARNNING             = 0x0001, //报警器          用于报警设备，红外，震动等
PID_DTU_ONLY                = 0x0002, //DTU             数传终端
PID_TEMP_HUMIDITY           = 0x0003, //温湿度传感器    用于温度和湿度采集
PID_WALL_SWITCH             = 0x0004, //开关            表示按键开关
PID_WALL_SOCKET             = 0x0005, //插座            表示继电器或插座类的电源
PID_LIGHT_CONTROLLER        = 0x0006, //灯光控制        专用灯光控制，可调光
PID_PM25_SENSOR             = 0x0007, //PM2.5           粉尘传感器
PID_GAS_SENSOR              = 0x0008, //气体传感器      采集气体，eg：NH3、H2S…
PID_GPS_LOCATION            = 0x0009, //GPS定位         电动车/汽车 GPS定位器
PID_LORA_GATEWAY            = 0x000A, //LoRa网关        LoRa网关
PID_MASSAGER_CONTROLLER     = 0x000B, //按摩椅控制终端  按摩椅远程控制
PID_FERTILIZER_CONTROL      = 0x000C, //施肥控制终端    施肥灌溉远程控制
PID_SOIL_SENSOR             = 0x000D, //土壤传感器      土壤水分和温度采集
PID_WATER_CONTROL           = 0x000E, //浇水控制终端    浇水远程控制
PID_DTU_VSM                 = 0x000F, //数传终端      振弦传感器
PID_LORA_STATION            = 0x0010, //lora基站    传lora数据到服务器
PID_DTU_RS485               = 0x0011, //数传终端      485传感器数据采集
}APP_ProductID_e;

typedef enum{
COMM_SERVICE    = 0x00,//服务器
COMM_BLE        = 0x01,//BLE 低功耗蓝牙
COMM_WIFI24     = 0x02,//Wi-Fi-2.4   2.4G-Wi-Fi通讯
COMM_WIFI50     = 0x12,//Wi-Fi-5.0   5G-Wi-Fi通讯
COMM_GPRS       = 0x03,//2G-GPRS GPRS通讯
COMM_CDMA1X     = 0x13,//2G-CDMA1X   电信的2G网络
COMM_LORA433    = 0x04,//LoRa-433    433MHz-LoRa扩频通讯技术
COMM_LORA868    = 0x14,//LoRa-868    868MHz-LoRa扩频通讯技术
COMM_LORA915    = 0x24,//LoRa-915    915MHz-LoRa扩频通讯技术
COMM_NBIOT      = 0x05,//NB-IOT  
COMM_NETWORK    = 0X06,//有线网络通信
COMM_4G         = 0X07,//4G通信
}APP_CommType_e;


#define VM_CHANNEL     1 //振弦通道数
#define VM_T_CHANNEL   1 //温度通道数
#define RS485_CHANNEL  2 //485通道数

#define YEAR      17   
#define MONTH     12

#define COMMUNICATION_TYPE  COMM_LORA433

#define TYPE      (VM_CHANNEL<<4) | (VM_T_CHANNEL) //振弦版本，高4位表示振弦路数，低4位表示温度路数
//#define TYPE    RS485_CHANNEL            //485版本，表示通道数

#define DEVICE_TYPE         PID_DTU_VSM
//#define DEVICE_TYPE         PID_DTU_RS485


const uint16_t Device_Number = 0x1001;

    system_setting_info_t m_mbr_params_page       __attribute__((at(SYSTEM_SETTINGS_ADDRESS)))= {
                                                            .DeviceID = {(uint8_t)(DEVICE_TYPE),(uint8_t)(DEVICE_TYPE >> 8),COMMUNICATION_TYPE,TYPE,
                                                                          MONTH,YEAR,(uint8_t)(Device_Number),(uint8_t)(Device_Number >> 8)},
                                                            .Server_port = 4415,
                                                            .Server_IPAddr = {"120.26.106.135\0"},
                                                        };

#elif defined ( __GNUC__ )

    system_setting_info_t m_mbr_params_page       __attribute__ ((section(".sysSettingAddr")));

#elif defined ( __ICCARM__ )

    __no_init system_setting_info_t m_mbr_params_page    @ SYSTEM_SETTINGS_ADDRESS;

#else

    #error Not a valid compiler/linker for m_mbr_params_page placement.

#endif

void app_system_reset(void)
{
    NVIC_SystemReset();
}

// Retrieve the address of a page.
static uint32_t * address_of_page(uint16_t page_num)
{
    uint32_t * page_address;
    page_address = (uint32_t *)(page_num * FLASH_PAGE_SIZE);
    return page_address;
}

/** @brief Function for erasing a page in flash.
 *
 * @param page_address Address of the first word in the page to be erased.
 */
static void flash_page_erase(uint32_t * page_address)
{
    // Turn on flash erase enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Een << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // Erase page:
    NRF_NVMC->ERASEPAGE = (uint32_t)page_address;

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // Turn off flash erase enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }
}

static void flash_pages_erase(uint16_t start_page, uint16_t pages_cout)
{   
    while(pages_cout--)
    {
        flash_page_erase(address_of_page(start_page++));
    }
}

static void flash_erase_app(void)
{
    MAIN_DBG("erase APP_START_PAGE:%d,%d\r\n", APP_START_PAGE, APP_PAGE_SIZE);
    MAIN_DBG("Addr:0x%08X-0x%08X\r\n", address_of_page(APP_START_PAGE), address_of_page(APP_START_PAGE+APP_PAGE_SIZE));
    flash_pages_erase(APP_START_PAGE, APP_PAGE_SIZE);
}

static void flash_erase_swap(void)
{
    MAIN_DBG("erase APP_SWAP_START_PAGE:%d,%d\r\n", APP_SWAP_START_PAGE, APP_SWAP_PAGE_SIZE);
    MAIN_DBG("Addr:0x%08X-0x%08X\r\n", address_of_page(APP_SWAP_START_PAGE), address_of_page(APP_SWAP_START_PAGE+APP_SWAP_PAGE_SIZE));
    flash_pages_erase(APP_SWAP_START_PAGE, APP_SWAP_PAGE_SIZE);
}

static void flash_erase_data(void)
{
    MAIN_DBG("erase APP_DATA_START_PAGE:%d,%d\r\n", APP_DATA_START_PAGE, APP_DATA_PAGE_SIZE);
    MAIN_DBG("Addr:0x%08X-0x%08X\r\n", address_of_page(APP_DATA_START_PAGE), address_of_page(APP_DATA_START_PAGE+APP_DATA_PAGE_SIZE));
    flash_pages_erase(APP_DATA_START_PAGE, APP_DATA_PAGE_SIZE);
}

/** @brief Function for filling a page in flash with a value.
 *
 * @param[in] address Address of the first word in the page to be filled.
 * @param[in] value Value to be written to flash.
 */
static void flash_write(uint32_t * address, uint32_t * value, uint16_t word_len)
{
    // Turn on flash write enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }
    while(word_len--)
    {
        *address++ = *value++;
    }

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // Turn off flash write enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }
}

bool flash_memcpy(uint32_t * const dest, uint32_t * const src, uint32_t word_len)
{
    uint32_t crc_src;
    uint32_t crc_dest;
    MAIN_DBG("flash_memcpy+++:\r\nwl:%d\r\n", word_len);
    MAIN_DBG("wl:%d*4=%d\r\n", word_len, word_len*4);
    MAIN_DBG("0x%08X->0x%08X\r\n", src, dest);
    crc_src = crc16_compute((uint8_t *)src, (word_len * 4), NULL);
    MAIN_DBG("src CRC:0x%08X\r\n", crc_src);
    flash_write(dest, src, word_len);
    
    crc_dest = crc16_compute((uint8_t *)dest, (word_len * 4), NULL);
    MAIN_DBG("dest CRC:0x%08X\r\n", crc_dest);
    MAIN_DBG("flash_memcpy---\r\n", word_len);
    if (crc_src == crc_dest)
        return true;
    
    return false;
}

void flash_app_copy(void)
{   
    flash_memcpy(address_of_page(APP_START_PAGE), swap_app_addr, pFlashInfo->word_len);
}

bool at_erase(uint8_t * param, uint16_t len)
{
    CONSOLE_OUT("Erase:+++\r\n");
    if (len>2)
    {
        switch(param[1])
        {
            case 'A':
            case 'a':
                flash_erase_app();
                break;
            case 'S':
            case 's':
                flash_erase_swap();
                break;
            case 'D':
            case 'd':
                flash_erase_data();
                break;
        }
    }
    else
    {
        CONSOLE_OUT("All Page:%d,%d\r\n", APP_START_PAGE, (APP_PAGE_SIZE+APP_SWAP_PAGE_SIZE));
        CONSOLE_OUT("Addr:0x%08X-0x%08X\r\n", address_of_page(APP_START_PAGE), address_of_page(APP_START_PAGE+(APP_PAGE_SIZE+APP_SWAP_PAGE_SIZE)));
        flash_pages_erase(APP_START_PAGE, (APP_PAGE_SIZE+APP_SWAP_PAGE_SIZE));
    }
    CONSOLE_OUT("Erase:---\r\n");
    
    return true;
}

bool at_write(uint8_t * param, uint16_t len)
{
    uint32_t * app_swap = address_of_page(APP_DATA_START_PAGE);
    
    CONSOLE_OUT("at_write:0x%08X\r\n", app_swap);
    flash_write(app_swap, (uint32_t*)param, len/4);
    
    return true;
}

bool at_update(uint8_t * param, uint16_t len)
{
    dcb_stage = 4;
    return true;
}

bool at_login(uint8_t * param, uint16_t len)
{
    uint8_t login_stage = 0;
    
    b_user_login = 0;
    for(uint8_t i = 0; i < len; i++)
    {
        switch(login_stage)
        {
            case 0:
                if(param[i] == '=')
                    login_stage++;
                else
                    login_stage = 0;
                break;
            case 1:
                if(param[i] == '"')
                    login_stage++;
                else
                    login_stage = 0;
                break;
            case 2:
                if(param[i] == 'e')
                    login_stage++;
                else
                    login_stage = 0;
                break;
            case 3:
                if(param[i] == 'e')
                    login_stage++;
                else
                    login_stage = 0;
                break;
            case 4:
                if(param[i] == 'X')
                    login_stage++;
                else
                    login_stage = 0;
                break;
            case 5:
                if(param[i] == 'c')
                    login_stage++;
                else
                    login_stage = 0;
                break;
            case 6:
                if(param[i] == 'l')
                    login_stage++;
                else
                    login_stage = 0;
                break;
            case 7:
                if(param[i] == 'o')
                    login_stage++;
                else
                    login_stage = 0;
                break;
            case 8:
                if(param[i] == 'u')
                    login_stage++;
                else
                    login_stage = 0;
                break;
            case 9:
                if(param[i] == 'd')
                    login_stage++;
                else
                    login_stage = 0;
                break;
            case 10:
                if(param[i] == '"')
                {
                    b_user_cmd = true;
                    b_user_login = 1;
                    CONSOLE_OUT("User login\r\n", param);
                    login_stage++;
                    
                    return true;
                }
                else
                    login_stage = 0;
                break;
        }
    }
    return false;
}

bool at_reset(uint8_t * param, uint16_t len)
{
    app_system_reset();
    
    return true;
}

static bool bStartApp = false;
bool start_app(uint8_t * param, uint16_t len)
{
    bStartApp = true;
    
    return true;
}

bool start_boot(uint8_t * param, uint16_t len)
{
    //nrf_bootloader_app_start((uint32_t)address_of_page(BOOTLOADER_START_PAGE));
    app_system_reset();
    
    return true;
}

bool at_crc(uint8_t * param, uint16_t len)
{
    uint32_t atc;
    atc = crc16_compute((uint8_t *)MAIN_APPLICATION_START_ADDR, 31480, NULL);
    CONSOLE_OUT("CRC=0x%08X\r\n", atc);
    
    return true;
}

const ATCMD_def_t g_cmd_def[] = {
    {"ERASE", 5, 1, at_erase},
    {"WRITE", 5, 1, at_write},
    {"UPDATE", 6, 1, at_update},
    {"RST", 3, 0, at_reset},
    {"LOGIN", 5, 0, at_login},
    {"START_APP", 9, 0, start_app},
    {"START_BOOT", 10, 0, start_boot},
    {"CRC", 3, 0, at_crc},
    {0, 0, NULL},
};

void app_swap_area_wrtie(void)
{
    swap_info.crc16 = crc16_compute(data_array, data_index, (swap_info.word_len>0)?((uint16_t *)&swap_info.crc16):NULL);
    flash_write(app_swap, (uint32_t*)data_array, data_index/4);
    app_swap += data_index/4;
    swap_info.word_len += data_index/4;
    data_index = 0;
}

void uart_dcb(uint8_t *pb, uint16_t len)
{
    switch(dcb_stage)
    {
        case 0:
            if (pb[0] == 'a' || pb[0] == 'A')
            {
                dcb_stage++;
            }
            break;
        case 1:
            if (pb[0] == 't' || pb[0] == 'T')
            {
                dcb_stage++;
            }
            break;
        case 2:
            if (pb[0] == '+')
            {
                dcb_stage++;
                data_index = 3;
                memset(data_array, 0, data_array_size);
                data_array[0] = 'A';
                data_array[1] = 'T';
                data_array[2] = '+';
                atcmd_rev_size = 0;
                atcmd_end = 0;
            }
            break;
        case 3:
            if (pb[0] == '\r' || pb[0] == '\n')//
            {
                uint16_t ifun = 0;
                uint8_t bFind_cmd = 0;
                dcb_stage = 0;
                //data_array[data_index++] = '\r';
                data_array[data_index++] = '\n';
                // Loopback
                UartSendBuffer(data_array, data_index);
                //MAIN_DBG("cmd size:%d\r\n", atcmd_rev_size);
                
                while(g_cmd_def[ifun].cmd[0] != 0)
                {
                    if (atcmd_rev_size == g_cmd_def[ifun].actual_cmd_size)
                    {
                        if (0 == memcmp(g_cmd_def[ifun].cmd, &data_array[3], atcmd_rev_size))
                        {
                            if ((g_cmd_def[ifun].login && b_user_login) || (g_cmd_def[ifun].login == 0))
                            {
                                if (g_cmd_def[ifun].fun)
                                {
                                    if (g_cmd_def[ifun].fun(&data_array[3+atcmd_rev_size], data_index - 3 - atcmd_rev_size))
                                    {
                                        CONSOLE_OUT("\r\nOK\r\n");
                                    }
                                }
                                else
                                    CONSOLE_OUT("CMD Unrealized\r\n");
                            }
                            else
                            {
                                CONSOLE_OUT("CMD : Please login first\r\n");
                            }
                            
                            bFind_cmd = 1;
                            
                            break;
                        }
                    }
                    ifun++;
                }
                
                if (bFind_cmd == 0)
                {
                    CONSOLE_OUT("CMD Not Support\r\n");
                }
            }
            else
            {
                if (atcmd_end == 0)
                {
                    switch(pb[0])
                    {
                        case ',':
                        case '=':
                        case '?':
                            atcmd_end = 1;
                        break;
                        default:
                            atcmd_rev_size++;
                        break;
                    }
                    
                    if ((pb[0] <= 'z') && (pb[0] >= 'a'))
                    {
                        data_array[data_index++] = pb[0]+'A'-'a';
                    }
                    else
                    {
                        data_array[data_index++] = pb[0];
                    }
                }
                else
                {
                    data_array[data_index++] = pb[0];
                }

                
            }
            break;
            
        case 4:
            app_swap = address_of_page(APP_SWAP_START_PAGE);
            app_swap += (sizeof(app_swap_info_t)/4);
            atcmd_raw_rev = 1;
            atcmd_raw_rev_time_out = 0;
            data_index = 0;
            swap_info.crc16 = 0;
            swap_info.word_len = 0;
            memset(data_array, 0, data_array_size);
            dcb_stage = 5;
            CONSOLE_OUT("update 0x%08X: wait send\r\n", app_swap);
            break;
        case 5:
            if (atcmd_raw_rev)
            {
                atcmd_raw_rev_time_out = 0;
                data_array[data_index++] = pb[0];
                if (data_index >= 256)
                {
                    atcmd_raw_rev = 2;
                    app_swap_area_wrtie();
                }
                return ;
            }
            break;
                            
    }
}

/**
 * @brief Function for application main entry.
 */


int main(void)
{
    uint32_t crc32_flash_ = 0;
    uint32_t wait_time_out = 100000;
    
    //UartInit(UART_State_Gprs, UART_BAUDRATE_BAUDRATE_Baud115200, uart_dcb);

    MAIN_DBG("Bootloader Start Release\r\n");
    MAIN_DBG("App:0x%08X\r\n", MAIN_APPLICATION_START_ADDR);
    MAIN_DBG("Boot:0x%08X\r\n", BOOTLOADER_START_ADDR);

    pFlashInfo = (app_swap_info_t *)address_of_page(APP_SWAP_START_PAGE);
    swap_app_addr = address_of_page(APP_SWAP_START_PAGE) + sizeof(app_swap_info_t)/4;
    
    while (true)
    {
        if (atcmd_raw_rev)
        {
            atcmd_raw_rev_time_out++;
            if (((atcmd_raw_rev_time_out % 100000) == 0) && (atcmd_raw_rev == 1))
            {
                CONSOLE_OUT(".");
            }

            if ((atcmd_raw_rev_time_out > 2500000) && (atcmd_raw_rev == 2))
            {
                dcb_stage = 0;
                app_swap_area_wrtie();
                
                crc32_flash_ = crc16_compute((uint8_t *)swap_app_addr, (swap_info.word_len*4), NULL);
                CONSOLE_OUT("Rev end,len=%d\r\n", (swap_info.word_len*4));
                CONSOLE_OUT("UC:0x%08X,FC:0x%08X\r\n", swap_info.crc16, crc32_flash_);
                atcmd_raw_rev = 0;
                if (crc32_flash_ == swap_info.crc16)
                {
                    swap_info.flag = SWAP_MAGIC_NUMBER;
                    flash_write(address_of_page(APP_SWAP_START_PAGE), (uint32_t*)&swap_info, sizeof(app_swap_info_t)/4);
                    
                    CONSOLE_OUT("\r\nFlash Info:\r\nFlag:0x%08X\r\n", pFlashInfo->flag);
                    CONSOLE_OUT("WordLen:%d\r\n", pFlashInfo->word_len);
                    CONSOLE_OUT("CRC32:0x%08X\r\n\r\n", pFlashInfo->crc16);
                    
                    CONSOLE_OUT("\r\nOK\r\n", pFlashInfo->crc16);
                }
                flash_app_copy();
                
                flash_erase_swap();
            }
        }
        
        if (!b_user_cmd)
        {
            if (wait_time_out-- == 0)
            {
                bStartApp = true;
            }
        }
        
        if (bStartApp)
        {
            if (pFlashInfo->flag == SWAP_MAGIC_NUMBER)
            {
                uint32_t wait_t = 100000;

                crc32_flash_ = crc16_compute((uint8_t *)swap_app_addr, (pFlashInfo->word_len*4), NULL);
                
                if (crc32_flash_ == pFlashInfo->crc16)
                {
                    MAIN_DBG("\r\nFlash Info:\r\nFlag:0x%08X\r\n", pFlashInfo->flag);
                    MAIN_DBG("WordLen:%d\r\n", pFlashInfo->word_len);
                    MAIN_DBG("CRC32:0x%08X\r\n\r\n", pFlashInfo->crc16);
                    
                    MAIN_DBG("\r\nStep 1+++\r\n\r\n");
                    flash_erase_app();
                    wait_t = 100000;
                    while(wait_t--);
                    MAIN_DBG("\r\nStep 2+++\r\n\r\n");
                    flash_app_copy();
                    wait_t = 100000;
                    while(wait_t--);
                }
                MAIN_DBG("\r\nStep 3+++\r\n\r\n");
                flash_erase_swap();
                wait_t = 100000;
                while(wait_t--);
            }
            
            nrf_bootloader_app_start(MAIN_APPLICATION_START_ADDR);
            break;
        }
    }
    
    MAIN_DBG("Bootloader End,Should never be reached.\r\n");
}


/** @} */
