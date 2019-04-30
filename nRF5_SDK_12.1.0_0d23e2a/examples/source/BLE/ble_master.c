#ifdef BLE_SCAN
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "app_error.h"
#include "sdk_errors.h"
#include "ble_conn_state.h"
#include "ble_hci.h"

#include "ble_config.h"
#include "ble_master.h"
#include "ble_salve.h"
#include "boards.h"
#include "oled_app.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"


#ifdef BLE_M_DEBUG
 #define BLE_M_Msg       NRF_LOG_INFO
#else
  #define BLE_M_Msg(...)
#endif



ble_nus_c_t        m_ble_nus_c;                                         //从机服务实例
uint16_t           m_conn_handle_nus_c = BLE_CONN_HANDLE_INVALID;       //连接状态

ble_db_discovery_t m_ble_db_discovery[CENTRAL_LINK_COUNT + PERIPHERAL_LINK_COUNT];  //作主机是，从机的服务列表

//static const char m_target_periph_name[] = "cqxd-Beacon" ;                           //需要连接的设备名

/* 扫描参数 */
static const ble_gap_scan_params_t m_scan_params =
{
    .active   = 1,
    .interval = SCAN_INTERVAL,
    .window   = SCAN_WINDOW,
    .timeout  = SCAN_TIMEOUT,
#if (NRF_SD_BLE_API_VERSION == 2)
        .selective   = 0,
        .p_whitelist = NULL,
#endif
#if (NRF_SD_BLE_API_VERSION == 3)
        .use_whitelist = 0,
#endif
};

////连接参数
//static const ble_gap_conn_params_t m_connection_param =
//{
//    (uint16_t)MIN_CONNECTION_INTERVAL,
//    (uint16_t)MAX_CONNECTION_INTERVAL,
//    0,
//    (uint16_t)SUPERVISION_TIMEOUT
//};


/**********************************************************************
功能:开启扫描
***********************************************************************/
void scan_start(void)
{
    ret_code_t err_code;

    (void) sd_ble_gap_scan_stop();

    err_code = sd_ble_gap_scan_start(&m_scan_params);
    if (err_code != NRF_ERROR_INVALID_STATE)
    {
        APP_ERROR_CHECK(err_code);
    }
}

#if 0
/**********************************************************************
功能:解析广播信息
输入参数:
    type:需要的类型
    p_advdata:广播数据
输出参数:
    p_typedata:解析后获取该类型的数据
***********************************************************************/
static uint32_t adv_report_parse(uint8_t type, data_t * p_advdata, data_t * p_typedata)
{
    uint32_t  index = 0;
    uint8_t * p_data;

    p_data = p_advdata->p_data;

    while (index < p_advdata->data_len)
    {
        uint8_t field_length = p_data[index];
        uint8_t field_type   = p_data[index + 1];

        if (field_type == type)
        {
            p_typedata->p_data   = &p_data[index + 2];
            p_typedata->data_len = field_length - 1;
            return NRF_SUCCESS;
        }
        index += field_length + 1;
    }
    return NRF_ERROR_NOT_FOUND;
}


/**********************************************************************
功能:做主机，通过NAME查找要连接的设备
输入参数:
    p_adv_report：收到的广播数据
    uuid_to_find: 需要查找的NAME
***********************************************************************/
static bool find_adv_name(const ble_gap_evt_adv_report_t *p_adv_report, const char * name_to_find)
{
    uint32_t err_code;
    data_t   adv_data;
    data_t   dev_name;

    // Initialize advertisement report for parsing
    adv_data.p_data     = (uint8_t *)p_adv_report->data;
    adv_data.data_len   = p_adv_report->dlen;

    //search for advertising names
    err_code = adv_report_parse(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME,
                                &adv_data,
                                &dev_name);
    if (err_code == NRF_SUCCESS)
    {
        if (memcmp(name_to_find, dev_name.p_data, dev_name.data_len )== 0)
        {
            return true;
        }
    }
    else
    {
        // Look for the short local name if it was not found as complete
        err_code = adv_report_parse(BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME,
                                    &adv_data,
                                    &dev_name);
        if (err_code != NRF_SUCCESS)
        {
            return false;
        }
        if (memcmp(m_target_periph_name, dev_name.p_data, dev_name.data_len )== 0)
        {
            return true;
        }
    }
    return false;
}


/**********************************************************************
功能:做主机，通过UUID查找要连接的设备
输入参数:
    p_adv_report：收到的广播数据
    uuid_to_find: 需要查找的UUID
***********************************************************************/
static bool find_adv_uuid(const ble_gap_evt_adv_report_t *p_adv_report, const uint16_t uuid_to_find)
{
    uint32_t err_code;
    data_t   adv_data;
    data_t   type_data;

    // Initialize advertisement report for parsing.
    adv_data.p_data     = (uint8_t *)p_adv_report->data;
    adv_data.data_len   = p_adv_report->dlen;

    err_code = adv_report_parse(BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_MORE_AVAILABLE,
                                &adv_data,
                                &type_data);

    if (err_code != NRF_SUCCESS)
    {
        // Look for the services in 'complete' if it was not found in 'more available'.
        err_code = adv_report_parse(BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE,
                                    &adv_data,
                                    &type_data);

        if (err_code != NRF_SUCCESS)
        {
            // If we can't parse the data, then exit.
            return false;
        }
    }

    // Verify if any UUID match the given UUID.
    for (uint32_t u_index = 0; u_index < (type_data.data_len / UUID16_SIZE); u_index++)
    {
        uint16_t    extracted_uuid;

        UUID16_EXTRACT(&extracted_uuid, &type_data.p_data[u_index * UUID16_SIZE]);

        if (extracted_uuid == uuid_to_find)
        {
            return true;
        }
    }
    return false;
}

#endif

static struct scan_flag_t{
	  uint8_t startScanFlag:1;       //0:未开启扫描      1:已经开启扫描 
	  uint8_t timeCount:2;           //时间记录，该时间用于空时扫描和不扫描的间隔	
}scanFlag; 

#define SCAN_VALUE_NUM      5      //记录扫描的缓存个数
/* 用于缓存扫描到信标的信号强度 */
static struct scan_value_t{
    int rssi;
	  UTCTime timeNode;
}scanValue[SCAN_VALUE_NUM];
static uint8_t buffCount = 0;  //用于记录当前填充的缓存空间，如果缓存空间填充满了。就向前溢出填充

#define AVERAGE_RSSI_MIN      (70)      //判断信号强度的最小信号强度平均值
#define RSSI_MIN              (90)      //判断信号强度的最小信号强度
#define INVAIN_RSSI_NUM        3        //判断无效信号强度的最大次数
#define INTERVAL_TIME          20        //判断无效的超时时间(S)
/**********************************************************************
功能:获取信标的状态，1S进入一次该函数
说明：
    对事件进行响应
***********************************************************************/
void Get_Beacon_State(void)
{
	  ret_code_t err_code;
	
	  /* 间歇性开启和关闭扫描功能。0的时候开启扫描，1-3的时候关闭扫描。在0-3之间循环 */
    if( scanFlag.timeCount == 0 && scanFlag.startScanFlag == 0 && save_param.OpenBeaconFlag == 1 )   //如果未开启扫描,且统计时间为0,且开启扫描，就开启扫描
		{
		     err_code = sd_ble_gap_scan_start(&m_scan_params);
			
         if (err_code != NRF_ERROR_INVALID_STATE)
         {
             APP_ERROR_CHECK(err_code);
         } 
         scanFlag.startScanFlag = 1;				 
				 scanFlag.timeCount++; 
		}
	  else if( scanFlag.timeCount == 1 )     //扫描时间结束，停止扫描
		{
			   if( scanFlag.startScanFlag != 0 )
				 {
		         (void) sd_ble_gap_scan_stop();  //关闭扫描
			        scanFlag.startScanFlag = 0;     //标记关闭扫描
				 }
			   scanFlag.timeCount++;
		}	
		else
		{
		     scanFlag.timeCount++;
         if( scanFlag.timeCount == 4 )
             scanFlag.timeCount = 0;					 
		}
		
		/* 如果关闭了扫描功能 */
		if( save_param.OpenBeaconFlag == 0 )
		{
		    sys_state.Beacon_scan_state = 0;
        return;			
		}
		
    /* 计算信标是否扫描到信标 */
		uint8_t beBeacon = 0;    //如果该值最后不为0,表示未扫描到信标
		int rssiSum = 0;
		uint8_t count = 0;
		//(0)判断缓存是否填满,判断不满足就返回
		if( buffCount <  SCAN_VALUE_NUM)
		{
	      beBeacon |= (1<<0);	   
  			return;
		}
		
		//(1)判断距离上一次扫描到信标是否超时,判断不满足是beBeacon |= (1<<0)
		if( INTERVAL_TIME < (GetClock_seconds() - scanValue[SCAN_VALUE_NUM-1].timeNode) )
		{
		    beBeacon |= (1<<0);	
        NRF_LOG_INFO(" scan Beacon timeout\r\n ");			
  			goto judge;    
		}
		
		//(2)判断平均信号强度是否满足,判断不满足是beBeacon |= (1<<1)
		
		for( uint8_t i = 0; i < SCAN_VALUE_NUM; i++ )
		{
		    rssiSum += scanValue[i].rssi;
		}
		if( abs( rssiSum/SCAN_VALUE_NUM ) > AVERAGE_RSSI_MIN )
		{
		    beBeacon |= (1<<1);	  
        NRF_LOG_INFO(" average rssi min\r\n ");			
  			goto judge;     
		}
		
		//(3)满足信号强度出现的次数,判断不满足是beBeacon |= (1<<2)
		
		for( uint8_t i = 0; i < SCAN_VALUE_NUM; i++ )
		{
		    if(abs(scanValue[i].rssi) > RSSI_MIN )
				    count++;
		}
		if( count >= INVAIN_RSSI_NUM )
		{
		    beBeacon |= (1<<2);	  
        NRF_LOG_INFO(" invain rssi number\r\n ");				
  			goto judge;    
		}
		
		sys_state.Beacon_scan_state = 1;
		return;   //入过都满足，就返回
		
judge:	     
     /* 清除缓存 */
		buffCount = 0;
		
    if( beBeacon != 0 && sys_state.Beacon_scan_state == 1)  // 未扫描到信标 
		{
			    /* 标记信标丢失 */
		      sys_state.Beacon_scan_state = 0;
			
					/* 如果当前页面不在手机采集，蓝牙断开，蓝牙连接，闹钟页面 */
					if(   OLED_ctrl.page_num != COLLECT_P 
							&&  OLED_ctrl.page_num != BLE_DISCONNECT_P 
						  && OLED_ctrl.page_num != BLE_CONNECT_P 
						  && OLED_ctrl.page_num != ALARM_P)
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
								
								 SetNewPageInfo(BEACON_LOST_P, OLED_ON, OLED_CLOSE_EN, OLED_OFF_TIME);   //设置当前要显示的页面信息
								 JumpPage();
					}
		}			
					  
					 
	
}

/**********************************************************************
功能:对扫描的设备进行校验，判断是否为需要的设备
说明：
    对事件进行响应
***********************************************************************/
extern save_param_t save_param;
void Cheack_Beacon(const ble_gap_evt_adv_report_t *p_adv_report)
{
    if( p_adv_report->peer_addr.addr[5] == save_param.Beacon_Mac[0] && \
			  p_adv_report->peer_addr.addr[4] == save_param.Beacon_Mac[1] && \
		    p_adv_report->peer_addr.addr[3] == save_param.Beacon_Mac[2] && \
		    p_adv_report->peer_addr.addr[2] == save_param.Beacon_Mac[3] && \
		    p_adv_report->peer_addr.addr[1] == save_param.Beacon_Mac[4] && \
		    p_adv_report->peer_addr.addr[0] == save_param.Beacon_Mac[5] )      
		{
			  BLE_M_Msg("==============================\r\n");
			  BLE_M_Msg("rssi:%d\r\n",p_adv_report->rssi );
			
			  if( buffCount < SCAN_VALUE_NUM )    //如果缓存未填满
				{
				    scanValue[buffCount++].rssi =  p_adv_report->rssi;
        		scanValue[buffCount++].timeNode = GetClock_seconds();			
				}
				else                                //缓存填满,向前溢出放入缓存
				{
				    for(int i = 0; i < (SCAN_VALUE_NUM - 1); i++)
            {
						    scanValue[i].rssi = scanValue[i+1].rssi;
                scanValue[i].timeNode = scanValue[i+1].timeNode;							
						}	
						scanValue[SCAN_VALUE_NUM - 1].rssi =  p_adv_report->rssi;
        		scanValue[SCAN_VALUE_NUM - 1].timeNode = GetClock_seconds();	
				}
		}
}

/**********************************************************************
功能:作主机对事件的回调函数
说明：
    对事件进行响应
***********************************************************************/
void on_ble_central_evt(const ble_evt_t * const p_ble_evt)
{
    const ble_gap_evt_t   * const p_gap_evt = &p_ble_evt->evt.gap_evt;
    ret_code_t                    err_code;

    switch (p_ble_evt->header.evt_id)
    {
        /* 连接事件 */
        case BLE_GAP_EVT_CONNECTED:
        {
            BLE_M_Msg("Central Connected \r\n");
            
					  //如果没有连接
            if ((m_conn_handle_nus_c == BLE_CONN_HANDLE_INVALID))
            {
                BLE_M_Msg("try to find NUS on conn_handle 0x%x\r\n", p_gap_evt->conn_handle);

                APP_ERROR_CHECK_BOOL(p_gap_evt->conn_handle < CENTRAL_LINK_COUNT + PERIPHERAL_LINK_COUNT);
                err_code = ble_db_discovery_start(&m_ble_db_discovery[p_gap_evt->conn_handle], p_gap_evt->conn_handle);
                APP_ERROR_CHECK(err_code);
            }

            if (ble_conn_state_n_centrals() == CENTRAL_LINK_COUNT)    //当前连接的数量
            {
               
            }
            else
            {
                scan_start();
            }
        } break; 

				/* 断开连接 */
        case BLE_GAP_EVT_DISCONNECTED:
        {
            uint8_t n_centrals;

            if (p_gap_evt->conn_handle == m_conn_handle_nus_c)
            {
                BLE_M_Msg("NUS central disconnected (reason: %d)\r\n",
                       p_gap_evt->params.disconnected.reason);

                m_conn_handle_nus_c = BLE_CONN_HANDLE_INVALID;
            }

            if ((m_conn_handle_nus_c == BLE_CONN_HANDLE_INVALID))
            {
                scan_start();
            }
            n_centrals = ble_conn_state_n_centrals();    //查询连接总数量

            if (n_centrals == 0)
            {
							
            }
        } break; 

				/* 扫描到广播 */
        case BLE_GAP_EVT_ADV_REPORT:
        {
				    Cheack_Beacon(&p_gap_evt->params.adv_report);        
#if 0					
            if (strlen(m_target_periph_name) != 0)   //通过名字查找要连接的设备
            {
                if (find_adv_name(&p_gap_evt->params.adv_report, m_target_periph_name))
                {
									  BLE_M_Msg("==============================\r\n");
									  BLE_M_Msg("MAC:%02x:%02x:%02x:%02x:%02x:%02x ",   \
									  p_gap_evt->params.adv_report.peer_addr.addr[5],  \
									  p_gap_evt->params.adv_report.peer_addr.addr[4],  \
									  p_gap_evt->params.adv_report.peer_addr.addr[3],  \
									  p_gap_evt->params.adv_report.peer_addr.addr[2],  \
									  p_gap_evt->params.adv_report.peer_addr.addr[1],  \
									  p_gap_evt->params.adv_report.peer_addr.addr[0]);
									  BLE_M_Msg("rssi:%d\r\n",p_gap_evt->params.adv_report.rssi );
									
//                    err_code = sd_ble_gap_connect(&p_gap_evt->params.adv_report.peer_addr,   //连接蓝牙
//                                                  &m_scan_params,
//                                                  &m_connection_param);
//                    if (err_code != NRF_SUCCESS)
//                    {
//                        BLE_M_Msg("Connection Request Failed, reason %d\r\n", err_code);
//                    }
                }
            }
            else
            {
                if ((find_adv_uuid(&p_gap_evt->params.adv_report, BLE_UUID_NUS_SERVICE)&&    //通过UUID查找要连接的设备
                     (m_conn_handle_nus_c == BLE_CONN_HANDLE_INVALID)))
                {
//                    err_code = sd_ble_gap_connect(&p_gap_evt->params.adv_report.peer_addr,   //连接蓝牙
//                                                  &m_scan_params,
//                                                  &m_connection_param);
//                    if (err_code != NRF_SUCCESS)
//                    {
//                        BLE_M_Msg("Connection Request Failed, reason %d\r\n", err_code);
//                    }
                }
            }
#endif
        } break; 

        case BLE_GAP_EVT_TIMEOUT:
        {
            // We have not specified a timeout for scanning, so only connection attemps can timeout.
            if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN)
            {
                BLE_M_Msg("Connection Request timed out.\r\n");
            }
        } break; 

        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
        {
            // Accept parameters requested by peer.
            err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle,
                                        &p_gap_evt->params.conn_param_update_request.conn_params);
            APP_ERROR_CHECK(err_code);
        } break; 

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            BLE_M_Msg("GATT Client Timeout.\r\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break; 

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            BLE_M_Msg("GATT Server Timeout.\r\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break; 

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
功能:做主机时，从机事件回调函数
输入参数:
    p_adv_report：收到的广播数据
    uuid_to_find: 需要查找的NAME
***********************************************************************/
static void nus_c_evt_handler(ble_nus_c_t * p_nus_c, const ble_nus_c_evt_t * p_nus_c_evt)
{
    switch (p_nus_c_evt->evt_type)
    {
			  /* 从机设备服务事件 */
        case BLE_NUS_C_EVT_DISCOVERY_COMPLETE:
        {
            if (m_conn_handle_nus_c == BLE_CONN_HANDLE_INVALID)
            {
                ret_code_t err_code;

                m_conn_handle_nus_c = p_nus_c_evt->conn_handle;       //当前连接句柄
                BLE_M_Msg("NUS discovered on conn_handle 0x%x\r\n",m_conn_handle_nus_c);

							  //绑定服务句柄
							  err_code = ble_nus_c_handles_assign(p_nus_c,m_conn_handle_nus_c,&p_nus_c_evt->handles);   //p_nus_c_evt->handles:在从机上发现的服务句柄
                APP_ERROR_CHECK(err_code);

                // nus service discovered. Enable notification of nus.
                err_code = ble_nus_c_rx_notif_enable(p_nus_c);
                APP_ERROR_CHECK(err_code);
            }
        } break; 

				/* 数据事件 */
        case BLE_NUS_C_EVT_NUS_RX_EVT:
        {

            BLE_M_Msg("NUS rx len = %d\r\n", p_nus_c_evt->data_len);

            BLE_Salve_SendData(p_nus_c_evt->p_data, p_nus_c_evt->data_len);    //通过蓝牙发送
            
        } break; 
        
				/* 从机断开连接事件 */
				case BLE_NUS_C_EVT_DISCONNECTED:
				{
				    BLE_M_Msg("NUS server disconnect...\r\n");
				}
				break;
				
        default:
           
            break;
    }
}


/**********************************************************************
功能:当前设备做主机（客户端）初始化
说明：
确认当前设备作主机的回调函数
***********************************************************************/
void nus_c_init(void)
{
    uint32_t         err_code;
    ble_nus_c_init_t nus_c_init_obj;

    nus_c_init_obj.evt_handler = nus_c_evt_handler;

    err_code = ble_nus_c_init(&m_ble_nus_c, &nus_c_init_obj);
    APP_ERROR_CHECK(err_code);
}



/**********************************************************************
功能:作主机蓝牙发送数据
输入参数：
pdata:要发送的数据
len:数据长度
说明
   通过蓝牙给主机发送数据。
***********************************************************************/
void BLE_Master_SendData(const uint8_t *pdata, const uint16_t len)
{
	  uint32_t  err_code;
    uint8_t frame_number = 0;
    uint8_t residue_number = 0;
	  uint8_t i = 0;
	
	  frame_number = len/m_ble_nus_max_data_len;
	  residue_number = len%m_ble_nus_max_data_len;
	
	  for(i = 0; i < frame_number; i++ )
	  {
				 err_code = ble_nus_c_string_send(&m_ble_nus_c, (uint8_t *)(pdata+i*m_ble_nus_max_data_len), m_ble_nus_max_data_len);
         if ( (err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_ERROR_BUSY) )
         {
              APP_ERROR_CHECK(err_code);
         }		
		}
		
		if(residue_number != 0)
		{
			   err_code = ble_nus_c_string_send(&m_ble_nus_c, (uint8_t *)(pdata+i*m_ble_nus_max_data_len), residue_number);
			   if ( (err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_ERROR_BUSY) )
			  {
						APP_ERROR_CHECK(err_code);
			  }	
		}	
}

#endif


