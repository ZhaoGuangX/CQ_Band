#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"

#include "ble_conn_state.h"
#include "fstorage.h"
#include "boards.h"

#include "ble_config.h"
#include "ble_master.h"
#include "ble_salve.h"


/*************************************************************************
函数名:ble_evt_dispatch
功能:对不同的事件进行分配处理
输入参数:
@p_ble_evt:事件指针
输出参数:None
返回值:None
*************************************************************************/
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    uint16_t conn_handle;
    uint16_t role;

    ble_conn_state_on_ble_evt(p_ble_evt);

    conn_handle = p_ble_evt->evt.gap_evt.conn_handle;    //获取事件句柄
    role        = ble_conn_state_role(conn_handle);      //获取当前设备在连接中的角色         

    //根据不同角色将事件分配给不同回调函数
    if (role == BLE_GAP_ROLE_PERIPH)                     //如果当前设备做从机
    {
        on_ble_peripheral_evt(p_ble_evt);

        ble_advertising_on_ble_evt(p_ble_evt);
        ble_conn_params_on_ble_evt(p_ble_evt);
        ble_dfu_on_ble_evt(&m_dfus,p_ble_evt );
        ble_nus_on_ble_evt (&m_nus, p_ble_evt);
    }
    else if ((role == BLE_GAP_ROLE_CENTRAL) || (p_ble_evt->header.evt_id == BLE_GAP_EVT_ADV_REPORT))    //当前设备做主机
    {
        if (p_ble_evt->header.evt_id != BLE_GAP_EVT_DISCONNECTED)
        {
            on_ble_central_evt(p_ble_evt);               //更新连接句柄
        }

        if (conn_handle < CENTRAL_LINK_COUNT + PERIPHERAL_LINK_COUNT)
        {
            ble_db_discovery_on_ble_evt(&m_ble_db_discovery[conn_handle], p_ble_evt);
        }
        ble_nus_c_on_ble_evt(&m_ble_nus_c, p_ble_evt);

        // 如果时断开连接事件，更新连接句柄
        if (p_ble_evt->header.evt_id == BLE_GAP_EVT_DISCONNECTED)
        {
            on_ble_central_evt(p_ble_evt);
        }
    }
}

/*************************************************************************
函数名:sys_evt_dispatch
功能:系统事件处理
输入参数:
@sys_evt:事件指针
输出参数:None
返回值:None
*************************************************************************/
static void sys_evt_dispatch(uint32_t sys_evt)
{
    fs_sys_event_handler(sys_evt);
    ble_advertising_on_sys_evt(sys_evt);
}

/*************************************************************************
函数名:ble_stack_init
功能:蓝牙协议栈初始化
输入参数:None
输出参数:None
返回值:None
*************************************************************************/
void ble_stack_init(void)
{
    ret_code_t err_code;

    // 初始化晶振
    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    APP_ERROR_CHECK(err_code);

	
	  ble_enable_params.common_enable_params.vs_uuid_count = 2;
	
    //根据连接的数量来设置RAM
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);
   
    //使能BLE协议栈
#if (NRF_SD_BLE_API_VERSION == 3)
    ble_enable_params.gatt_enable_params.att_mtu = NRF_BLE_MAX_MTU_SIZE;
#endif
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    //绑定BLE事件回调函数
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    //绑定BLE系统事件回调函数
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}

/*************************************************************************
函数名:db_disc_handler
功能:做主机时，查询到从机服务的回调函数
输入参数:
@p_evt:事件指针
输出参数:None
返回值:None
*************************************************************************/
static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
    ble_nus_c_on_db_disc_evt(&m_ble_nus_c, p_evt);
}

/*************************************************************************
函数名:db_discovery_init
功能:做主机时,查询从机服务功能的初始化
输入参数:None
输出参数:None
返回值:None
*************************************************************************/
void db_discovery_init(void)
{
    ret_code_t err_code = ble_db_discovery_init(db_disc_handler);
    APP_ERROR_CHECK(err_code);
}


/*************************************************************************
函数名:adv_scan_start
功能:开启广播和扫描
输入参数:None
输出参数:None
返回值:None
*************************************************************************/
void adv_scan_start(void)
{
    ret_code_t err_code;
    uint32_t count;

    //检验没有操作flash
    err_code = fs_queued_op_count_get(&count);
    APP_ERROR_CHECK(err_code);

    if (count == 0)
    {
			  //打开广播
        err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
        APP_ERROR_CHECK(err_code);
			
			 
    }
}










