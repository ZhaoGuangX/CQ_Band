#ifndef __BLE_MASTER_H__
#define __BLE_MASTER_H__

#ifdef BLE_SCAN
#include "ble_config.h"
#include "ble_db_discovery.h"
#include "ble_nus_c.h"

#define BLE_M_DEBUG

#define SCAN_INTERVAL               0x0050                            //扫描间隔(两次扫描的间隔时间)*0.625ms
#define SCAN_WINDOW                 0x0050                            //扫描窗口(扫描的时间)
#define SCAN_TIMEOUT                0                                 //扫描超时时间  0:一直扫描

#define UUID16_SIZE                 2             //UUID的大小
#define UUID16_EXTRACT(DST, SRC)   do{(*(DST))   = (SRC)[1];(*(DST)) <<= 8;(*(DST))  |= (SRC)[0];} while (0)   //解析UUID

typedef struct
{
    uint8_t     * p_data;    //数据指针
    uint16_t      data_len;  //数据大小
} data_t;


extern ble_nus_c_t   m_ble_nus_c;                      //从机服务实例
extern uint16_t      m_conn_handle_nus_c;              //连接状态
extern ble_db_discovery_t m_ble_db_discovery[CENTRAL_LINK_COUNT + PERIPHERAL_LINK_COUNT];

void nus_c_init(void);
void scan_start(void);
void on_ble_central_evt(const ble_evt_t * const p_ble_evt);
void BLE_Master_SendData(const uint8_t *pdata, const uint16_t len);

/**********************************************************************
功能:获取信标的状态，1S进入一次该函数
说明：
    对事件进行响应
***********************************************************************/
void Get_Beacon_State(void);

#endif

#endif
