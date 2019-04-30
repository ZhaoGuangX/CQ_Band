#ifndef __BLE_CONFIG_H__
#define __BLE_CONFIG_H__

#ifdef BLE_SCAN
#define CENTRAL_LINK_COUNT          1          //作主机最大连接数
#else
#define CENTRAL_LINK_COUNT          0          //作主机最大连接数
#endif
#define PERIPHERAL_LINK_COUNT       1          //作从机最大连接数

#if (NRF_SD_BLE_API_VERSION == 3)
#define NRF_BLE_MAX_MTU_SIZE        GATT_MTU_SIZE_DEFAULT         //使用的MTU的大小
#endif


#define APP_FEATURE_NOT_SUPPORTED   BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2          //对不支持的请求的应答内容
#define DEVICE_TYPE     0x00                 //手环的设备类型

/*************************************************************************
函数名:ble_stack_init
功能:蓝牙协议栈初始化
输入参数:None
输出参数:None
返回值:None
*************************************************************************/
void ble_stack_init(void);

/*************************************************************************
函数名:db_discovery_init
功能:做主机时,查询从机服务功能的初始化
输入参数:None
输出参数:None
返回值:None
*************************************************************************/
void db_discovery_init(void);

/*************************************************************************
函数名:adv_scan_start
功能:开启广播和扫描
输入参数:None
输出参数:None
返回值:None
*************************************************************************/
void adv_scan_start(void);

#endif
