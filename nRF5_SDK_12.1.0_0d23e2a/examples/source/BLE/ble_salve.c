/*******************************************************************************************
*          版权所属, C ,2018- ,成都传奇兄弟信息技术有限公司
*
*作者:嵌入式开发部              日期:2018-9-1            版本:V0.0
*说明:
*    该设备做从机使用
*修改:
2019-3-1:
    修改广播数据为11字节。
2019-3-14:
    增加DFU服务。
*******************************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "ble_db_discovery.h"
#include "ble_conn_state.h"
#include "fstorage.h"
#include "boards.h"
#include "misc.h"
#include "app_timer.h"
#include "ble_hci.h"
#include "nrf_delay.h"

#include "ble_salve.h"
#include "app_uart_ex.h"
#include "ble_config.h"
#include "ble_master.h"
#include "ble_salve.h"
#include "comm_protocol.h"
#include "oled_app.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "stepcounter.h"
#include "MAX3010x.h"
#include "ble_dfu.h"
#include "nrf_bootloader_app_start.h"
#include "nrf_dfu_mbr.h"
#include "nrf_bootloader_info.h"
#include "nrf_mbr.h"

#ifdef BLE_S_DEBUG
 #define BLE_S_Msg       NRF_LOG_INFO
#else
  #define BLE_S_Msg(...)
#endif


ble_nus_t  m_nus;        //做从机时实例
ble_dfu_t m_dfus;        //固件升级服务实例
static ble_uuid_t  m_adv_uuids[] = {{BLE_UUID_NUS_SERVICE, BLE_UUID_TYPE_VENDOR_BEGIN}};  //UUID格式
/**********************************************************************
函数名:nus_data_handler
功能:蓝牙串口服务，接收数据
输入参数:
@p_nus:连接句柄
@p_data:接收的数据
@length:接收数据的长度
输出参数:None
返回值:None
说明:
***********************************************************************/
static void nus_data_handler(ble_nus_t * p_nus, uint8_t * p_data, uint16_t length)
{
    NRF_LOG_HEXDUMP_INFO(p_data,length);
    BLE_Data_analysis(p_data,length);
}

/**********************************************************************
函数名:on_ble_peripheral_evt
功能:本机作外围设备时，对连接事件的处理
输入参数:
@p_ble_evt:事件指针
输出参数:None
返回值:None
说明:
***********************************************************************/
void on_ble_peripheral_evt(ble_evt_t * p_ble_evt)
{
    ret_code_t err_code;
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
				{
            BLE_S_Msg("Peripheral connected\r\n");
            sys_state.ble_connect_state = 1;      //标记蓝牙连接 
				    
					  /* 如果当前页面不在信标丢失，闹钟页面 */
					  if( OLED_ctrl.page_num != BEACON_LOST_P && OLED_ctrl.page_num != ALARM_P)
						{
							   /* 如果是蓝牙断开页面，则不保存，直接返回到断开之前的页面。 */
							   if( OLED_ctrl.page_num != BLE_DISCONNECT_P )
								     SetLastPageInfo(OLED_ctrl.page_num, OLED_ctrl.oled_state_flag, OLED_ctrl.oled_off_key, OLED_ctrl.time_off);  //记录之前的页面信息
								
								 if( OLED_ctrl.oled_state_flag == OLED_OFF )
			              OLED_Display_On();
								
								 SetNewPageInfo(BLE_CONNECT_P, OLED_ON, OLED_CLOSE_EN, 2);   //设置当前要显示的页面信息
								 JumpPage();	
					  }
				}
        break;

        case BLE_GAP_EVT_DISCONNECTED:
				{
            BLE_S_Msg("Peripheral disconnected\r\n");
            sys_state.ble_connect_state = 0;       //标记蓝牙断开
					  
					  /* 如果当前页面不在信标丢失，闹钟页面 */
					  if( OLED_ctrl.page_num != BEACON_LOST_P && OLED_ctrl.page_num != ALARM_P)
						{
								 if( OLED_ctrl.oled_state_flag == OLED_OFF )  //如果当前屏幕熄灭的
								 {
								      OLED_Display_On();
									    SetLastPageInfo(DATE_P, OLED_ON, OLED_CLOSE_EN, 1);  //如果当前页面是熄灭的，就将上一屏的信息设置为日期页面。且时间为1S。确保断开连接后
									                                                         //无操作，就息屏。
								 }
								 else     //如果当前屏幕是亮的
								 {
								      SetLastPageInfo(OLED_ctrl.page_num, OLED_ctrl.oled_state_flag, OLED_ctrl.oled_off_key, OLED_OFF_TIME);  //记录之前的页面信息
								 }
			             
								
								 SetNewPageInfo(BLE_DISCONNECT_P, OLED_ON, OLED_CLOSE_EN, OLED_OFF_TIME);   //设置当前要显示的页面信息
								 JumpPage();
					   }
				}
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            BLE_S_Msg("GATT Client Timeout.\r\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break; 

        case BLE_GATTS_EVT_TIMEOUT:
            BLE_S_Msg("GATT Server Timeout.\r\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break; 

        case BLE_EVT_USER_MEM_REQUEST:
            err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gap_evt.conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        {
            ble_gatts_evt_rw_authorize_request_t  req;
            ble_gatts_rw_authorize_reply_params_t auth_reply;

            req = p_ble_evt->evt.gatts_evt.params.authorize_request;

            if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
            {
                if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
                {
                    if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                    }
                    else
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                    }
                    auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                    err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                               &auth_reply);
                    APP_ERROR_CHECK(err_code);
                }
            }
        } break; 

#if (NRF_SD_BLE_API_VERSION == 3)
        case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
            err_code = sd_ble_gatts_exchange_mtu_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                       NRF_BLE_MAX_MTU_SIZE);
            APP_ERROR_CHECK(err_code);
            break; 
#endif			
        default:
            break;
    }
}

/**********************************************************************
函数名:gap_params_init
功能:广播参数初始化
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONNECTION_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONNECTION_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = SUPERVISION_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

/**********************************************************************
函数名:conn_params_error_handler
功能:连接参数错误回调函数
输入参数:
@nrf_error:错误类型
输出参数:None
返回值:None
说明:
***********************************************************************/
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**********************************************************************
函数名:conn_params_init
功能:连接参数参数初始化
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_CONN_HANDLE_INVALID; 
    cp_init.disconnect_on_fail             = true;
    cp_init.evt_handler                    = NULL;  
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**********************************************************************
函数名:service_nus_init
功能:串口服务初始化
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
static void service_nus_init(void)
{
    uint32_t       err_code;
    ble_nus_init_t nus_init;

    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;
    
	   m_nus.uuid_type = BLE_UUID_TYPE_BLE;
    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
	
}


/**********************************************************************
函数名:nus_data_handler
功能:蓝牙串口服务，接收数据
输入参数:
@p_nus:连接句柄
@p_data:接收的数据
@length:接收数据的长度
输出参数:None
返回值:None
说明:
***********************************************************************/
#define BOOTLOADER_DFU_STATRT    0xB1
static void ble_dfu_evt_handler(ble_dfu_t * p_dfu, ble_dfu_evt_t * p_evt)
{
    switch (p_evt->type)
    {
        case BLE_DFU_EVT_INDICATION_DISABLED:
            NRF_LOG_INFO("Indication for BLE_DFU is disabled\r\n");
            break;

        case BLE_DFU_EVT_INDICATION_ENABLED:
				{
					  NRF_LOG_INFO("Indication for BLE_DFU is enabled\r\n");
				}  
            break;

        case BLE_DFU_EVT_ENTERING_BOOTLOADER:
            NRF_LOG_INFO("Device is entering bootloader mode!\r\n");
            break;
        default:
            NRF_LOG_INFO("Unknown event from ble_dfu\r\n");
            break;
    }
}

/**********************************************************************
函数名:services_init
功能:串口服务初始化
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void services_init(void)
{
	  uint32_t err_code;
	  ble_dfu_init_t dfus_init;
	
	  memset( &dfus_init, 0, sizeof(dfus_init) );
	
	  dfus_init.evt_handler								              = ble_dfu_evt_handler;
    dfus_init.ctrl_point_security_req_write_perm		  = SEC_SIGNED;
    dfus_init.ctrl_point_security_req_cccd_write_perm	= SEC_SIGNED;
			
    err_code = ble_dfu_init(&m_dfus, &dfus_init);
    APP_ERROR_CHECK(err_code);

	
    service_nus_init();
	
}

/**********************************************************************
函数名:on_adv_evt
功能:广播事件处理函数
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
//            BLE_S_Msg("Fast Advertising\r\n");
            break;

        case BLE_ADV_EVT_IDLE:
        {
            ret_code_t err_code;
            err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
            APP_ERROR_CHECK(err_code);
        } break;

        default:
            break;
    }
}


/**********************************************************************
函数名:advertising_init
功能:广播初始化
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
extern MAX3010x_Data_t Save_heart_sensor_data;  //保留上次获取的心率血压数据，用于广播
void advertising_init(void)
{
    uint32_t               err_code;
    ble_advdata_t          advdata;
    ble_advdata_t          scanrsp;
    ble_adv_modes_config_t options;
    static uint8_t manuf_data_scanrsp[6];
	  uint8_t gap_data[11] = {0};                      //自定义广播数据缓存
    ble_advdata_manuf_data_t manuf_specific_data;    //自己定义广播数据
    ble_advdata_manuf_data_t manuf_specific_data_scanrsp;
	
		/* 填充步数 */
		union{
         uint32_t a;
         uint8_t b[4];
    }c;
                     
    c.a = movement.stepcount;
		gap_data[3] = c.b[3];
    gap_data[4] = c.b[2];
    gap_data[5] = c.b[1];
    gap_data[6] = c.b[0];
		
		/* 填充心率血氧 */
		gap_data[0] = PRODUCT_NUMBER;
		gap_data[1] = Save_heart_sensor_data.SpO2;
		gap_data[2] = Save_heart_sensor_data.Heart;
		
    manuf_specific_data.company_identifier = APP_COMPANY_IDENTIFIER;     //公司标识
    manuf_specific_data.data.p_data = (uint8_t *) gap_data;              //广播的信息
    manuf_specific_data.data.size   = sizeof(gap_data);                  //广播的长度

    //添加广播信息
    memset(&advdata, 0, sizeof(advdata));
    advdata.name_type          = BLE_ADVDATA_FULL_NAME;                  //不广播设备名
    advdata.include_appearance = false;
    advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;    //一般广播模式
    advdata.p_manuf_specific_data = &manuf_specific_data;                //广播数据

    //传递扫描响应结构
    get_mac_addr(manuf_data_scanrsp);
    manuf_specific_data_scanrsp.company_identifier = APP_COMPANY_IDENTIFIER;
    manuf_specific_data_scanrsp.data.p_data = (uint8_t *) manuf_data_scanrsp;
    manuf_specific_data_scanrsp.data.size   = sizeof(manuf_data_scanrsp);
    
    memset(&scanrsp, 0, sizeof(scanrsp));
    scanrsp.name_type          = BLE_ADVDATA_FULL_NAME;
    scanrsp.include_appearance = false;
    scanrsp.p_manuf_specific_data = &manuf_specific_data_scanrsp;

    memset(&scanrsp, 0, sizeof(scanrsp));
    scanrsp.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    scanrsp.uuids_complete.p_uuids  = m_adv_uuids;
		
    memset(&options, 0, sizeof(options));
    options.ble_adv_fast_enabled  = true;
    options.ble_adv_fast_interval = APP_ADV_INTERVAL;
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

    err_code = ble_advertising_init(&advdata, &scanrsp, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
		
		sd_ble_gap_tx_power_set(0);    //设置发送功率为0dBm
}

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
void BLE_Salve_SendData(const uint8_t *pdata, const uint16_t len)
{
    if( sys_state.ble_connect_state == 1 )    //如果蓝牙有连接
    {
	       uint32_t  err_code;
					
				    err_code = ble_nus_string_send(&m_nus, (uint8_t *)pdata, len);
        if ( (err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_ERROR_BUSY) )
        {
                 APP_ERROR_CHECK(err_code);
        } 			 
		  }
}

