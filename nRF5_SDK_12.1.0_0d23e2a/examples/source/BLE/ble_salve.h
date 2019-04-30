#ifndef __BLE_SALVE_H__
#define __BLE_SALVE_H__

#include "ble_nus.h"
#include "ble_dfu.h"

#define BLE_S_DEBUG


#define APP_COMPANY_IDENTIFIER           0x1D1E                                        //特定制造商号(此号码本应该在蓝牙联盟申请;但由于其他原因，公司无法申请;
                                                                                       //所以在公司内部将该号码定义为本公司(成都传奇兄弟信息技术有限公司)特定号码。不能更改。
																																											 //一旦更改，将导致部分设备无法通信)

#define APP_ADV_INTERVAL                 MSEC_TO_UNITS(500,UNIT_0_625_MS)              //广播间隔时间500*0.625ms
#define APP_ADV_TIMEOUT_IN_SECONDS       MSEC_TO_UNITS(4800,UNIT_0_625_MS)              //广播超时时间3S  0:一直广播  


#define MIN_CONNECTION_INTERVAL          (uint16_t) MSEC_TO_UNITS(7.5, UNIT_1_25_MS)   //最小连接间隔
#define MAX_CONNECTION_INTERVAL          (uint16_t) MSEC_TO_UNITS(30, UNIT_1_25_MS)    //最大连接间隔
#define SLAVE_LATENCY                    0                                             //从机延时
#define SUPERVISION_TIMEOUT              (uint16_t) MSEC_TO_UNITS(4000, UNIT_10_MS)    //连接超时时间

#define FIRST_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)    //初始化完成到第一次调用sd_ble_gap_conn_param_update()的时间
#define NEXT_CONN_PARAMS_UPDATE_DELAY    APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER)   //调用sd_ble_gap_conn_param_update()的间隔
#define MAX_CONN_PARAMS_UPDATE_COUNT     3                                             //重连次数

#define DEVICE_NAME                      "cq-Band"                               //广播的设备名
#define MANUFACTURER_NAME                "cqxd.com"                                    //制造商名




#define m_ble_nus_max_data_len   20  //蓝牙发送数据长度

extern ble_nus_t  m_nus;             //做从机时实例
extern ble_dfu_t m_dfus;        //固件升级服务实例
/**********************************************************************
函数名:on_ble_peripheral_evt
功能:本机作外围设备时，对连接事件的处理
输入参数:
@p_ble_evt:事件指针
输出参数:None
返回值:None
说明:
***********************************************************************/
void on_ble_peripheral_evt(ble_evt_t * p_ble_evt);

/**********************************************************************
函数名:gap_params_init
功能:广播参数初始化
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void gap_params_init(void);

/**********************************************************************
函数名:conn_params_init
功能:连接参数参数初始化
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void conn_params_init(void);

/**********************************************************************
函数名:services_init
功能:串口服务初始化
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void services_init(void);


/**********************************************************************
函数名:advertising_init
功能:广播初始化
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void advertising_init(void);

/**********************************************************************
函数名:BLE_Salve_SendData
功能:蓝牙作外围设备发送数据
输入参数:
@pdata:需要发送的数据
@len:需要发送的数据长度
输出参数:None
返回值:None
说明:
***********************************************************************/
void BLE_Salve_SendData(const uint8_t *pdata, const uint16_t len);


#endif
