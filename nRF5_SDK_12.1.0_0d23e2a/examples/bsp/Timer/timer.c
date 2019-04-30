/*******************************************************************************************
*          版权所属, C ,2018- ,成都传奇兄弟信息技术有限公司
*
*作者:嵌入式开发部              日期:2018-9-1            版本:V0.0
*说明:
*    创建定时器，并运行个定时器的回调函数。
*修改:
2019-2-26:
1、怎加刷新广播信息定时器。
2019-2-27:
1、修改充电状态逻辑程序。
2、增加电量模式，关闭广播，关闭计步定时器。
*******************************************************************************************/
#include <stdio.h>

#include "timer.h"
#include "boards.h"
#include "adc.h"
#include "utc.h"
#include "oled_app.h"
#include "ble_salve.h"
#include "ble_master.h"
#include "comm_protocol.h"
#include "lis3dh_driver.h"
#include "app_gpio.h"
#include "spi.h"
#include "lis3dh_driver.h"
#include "ble_advertising.h"
#include "MAX3010x.h"
#include "stepcounter.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#ifdef TIME_DEBUG
 #define TIME_Msg    NRF_LOG_INFO
#else
 #define TIME_Msg(...)
#endif

APP_TIMER_DEF(m_period_task_timer_id);              //1S定时器
APP_TIMER_DEF(m_adc_task_timer_id);                 //ADC转换定时器
APP_TIMER_DEF(m_step_10_task_timer_id);             //计步10ms定时器
APP_TIMER_DEF(m_step_50_task_timer_id);             //计步20ms定时器
APP_TIMER_DEF(m_GapData_task_timer_id);             //广播数据更新定时器
APP_TIMER_DEF(m_max30102_task_timer_id);            //10m获取一次MAX30102的数据

uint16_t collect_interval = 0;       //自动采集间隔  

/**********************************************************************
函数名:timers_init
功能:对时钟进行初始化
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void timers_init(void)
{
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, NULL);
}

/**********************************************************************
函数名:Low_Power_State
功能:检测是否是低电量状态，如果是，做相应的处理
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
static void Low_Power_State(void)
{
     /*************************************** 电池电量过低情况 ************************************************/
		 static uint8_t low_power_flag = 0;      //0:未标记低电量   1:已标记低电量
		 uint32_t err_code;
		 
		 if( sys_state.power_low == 1 )  //电量过低
		 {
			   if( low_power_flag == 0 )
				 {
					   TIME_Msg("Low Power\r\n");
				     low_power_flag = 1;
					   /* 关闭广播,定时器 */
					   sd_ble_gap_adv_stop();   //关广播
					   (void)app_timer_stop(m_step_10_task_timer_id);    //关闭定时器
					   (void)app_timer_stop(m_step_50_task_timer_id);
					   (void)app_timer_stop(m_GapData_task_timer_id);
				 }
			 
		 }
		 else     //电量不低
		 {
		     if( low_power_flag == 1 )
				 {
					   TIME_Msg("Not Low Power\r\n");
				     low_power_flag = 0;
					   
	           err_code = app_timer_start(m_step_10_task_timer_id, STEP_10MS_TASK_TIMEOUT, NULL);  //开启计步10ms时间基础定时器
	           APP_ERROR_CHECK(err_code);
	
	           err_code = app_timer_start(m_step_50_task_timer_id, STEP_50MS_TASK_TIMEOUT, NULL);  //开启计步50ms算法定时器
	           APP_ERROR_CHECK(err_code);
	
	           err_code = app_timer_start(m_GapData_task_timer_id, GAPDATA_TASK_TIMEOUT, NULL);  //开启修改广播数据定时器
	           APP_ERROR_CHECK(err_code);   
					 
					    //开启广播,定时器
					   err_code = ble_advertising_start(BLE_ADV_MODE_FAST);    //重新广播
             APP_ERROR_CHECK(err_code);
				 }
		 }    
}

/**********************************************************************
函数名:Alarm_Response
功能:闹钟响应，闹钟时间到了后，响应
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
static void Alarm_Response(const UTCTime_t *tm)
{
     /*************************************** 闹钟情况 ************************************************/
		 if( tm->hour == save_param.alarm[0] && tm->minute == save_param.alarm[1] && tm->second == save_param.alarm[2] )
		 {
		     if( OLED_ctrl.page_num != BLE_DISCONNECT_P &&  OLED_ctrl.page_num != BLE_CONNECT_P && OLED_ctrl.page_num != BEACON_LOST_P )    //当前不在蓝牙断开页面，也不再蓝牙连接页面，也不再信标都是页面
				 {
				       if( OLED_ctrl.oled_state_flag == OLED_OFF )    //如果当前屏幕熄灭
               {  
							      OLED_Display_On();      //点亮屏幕
								    SetLastPageInfo(DATE_P, OLED_ON, OLED_CLOSE_EN, 1);  //如果当前页面是熄灭的，就将上一屏的信息设置为日期页面。且时间为1S。确保断开连接后	                                                         //无操作，就息屏。
							 }
							 else     //如果当前屏幕亮着
							 {
							      SetLastPageInfo(OLED_ctrl.page_num, OLED_ctrl.oled_state_flag, OLED_ctrl.oled_off_key, OLED_OFF_TIME);  //记录之前的页面信息
							 }
							 
							 SetNewPageInfo(ALARM_P, OLED_ON, OLED_CLOSE_EN, OLED_OFF_TIME);   //设置当前要显示的页面信息
								 JumpPage();
							 
							 /* 开启振动 */
				       Motor_Drive();
						   
				 }

				 
		 }		     
}

/**********************************************************************
函数名:Auto_Collection
功能:自动采集检测，如果间隔时间到，就自动开启采集
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
static void Auto_Collection(void)
{
     /*************************************** 自动采集情况 ************************************************/
		 if( collect_interval != 0 )
		 {
				   if( (--collect_interval == 0) && !( OLED_ctrl.page_num == HEART_P || OLED_ctrl.page_num == OXYGEN_P || OLED_ctrl.page_num == TEMPER_P))   //倒计时时间到，且当前不在采集页面
					 {
					     collect_interval = save_param.Collect_interval;    //重新赋值计数

                timers_start_Max30102_timer();
									
						    if( OLED_ctrl.oled_state_flag == 0 )
								    OLED_Display_On(); 
                
						    max30102_data.Heart = -3;
								OLEDE_dispaly_HeartRate(max30102_data.Heart);   //显示距离
						    OLED_ctrl.oled_off_key = 0;                     //不可以倒计时息屏
						    OLED_ctrl.oled_state_flag = 1;                  //标记当前屏幕是亮着的
						    OLED_ctrl.time_off = OLED_OFF_TIME;             //重新赋值倒计时时间
						    OLED_ctrl.page_num = HEART_P;                   //标记当前页面						 
					 }
		}	  
}

/**********************************************************************
函数名:period_timeout_handler
功能:1s周期定时器,实现UTC时间的运行,和LED动态显示
输入参数:
@p_context:参数传递指针
输出参数:None
返回值:None
说明:
***********************************************************************/
static void period_timeout_handler(void * p_context)
{
   
    UTCTime_t tm;
	  
    Time_Run();                              //运行UTC
	  GetClock_UTC(&tm);                       //获取UTC时间(必须需要，不然时间要出错)
	
	  Step_Updata(&tm);                        //上传步数
	  Change_Chatger_State();                  //改变电池状态
		OLED_Change_Display(&tm);                //刷新显示页面
		Low_Power_State();                    	 //检测是否是低电量状态
	  Alarm_Response(&tm);                     //闹钟检测
    Auto_Collection();                       //自动采集检测
	
#ifdef BLE_SCAN
	      Get_Beacon_State();
#endif
}

/**********************************************************************
函数名:ADC_timeout_handler
功能:电池电压ADC采集，定时器
输入参数:
@p_context:参数传递指针
输出参数:None
返回值:None
说明:
电池电压，固定一段时间采集一次，通过定时器实现
***********************************************************************/
static void ADC_timeout_handler(void * p_context)
{
     /* 采集心率血压时不采 */
	   if( OLED_ctrl.page_num != HEART_P && OLED_ctrl.page_num != OXYGEN_P && OLED_ctrl.page_num != TEMPER_P)
		 {
         start_adc_convert();               //开启ADC转换电池
		 }
}

/**********************************************************************
函数名:step_10ms_handler
功能:计步10ms定时器
输入参数:
@p_context:参数传递指针
输出参数:None
返回值:None
说明:
计步算法中需要10ms定时器，做为基础算法的心跳时钟
***********************************************************************/
static void step_10ms_handler(void * p_context)
{
    StepDetector_tick++;
}

/**********************************************************************
函数名:step_20ms_handler
功能:计步20ms定时器
输入参数:
@p_context:参数传递指针
输出参数:None
返回值:None
说明:
计步算法中需要50ms定时器，做为采集数据基准
***********************************************************************/
static void step_50ms_handler(void * p_context)
{	  
    uint32_t err_code;
	
	  /* 初始化硬件 */
	  SPI_Init();                     //SPI初始化
    LIS3DH_Init();                  //G_Sensor初始化
	
	  /* 采集数据,并作处理 */
    get_Gsenson_Value(); 
	
	  /* 关闭硬件 */
    SPI_UnInit();
	
	  /* 重新开启定时器 */
    err_code = app_timer_start(m_step_50_task_timer_id, STEP_50MS_TASK_TIMEOUT, NULL);
    APP_ERROR_CHECK(err_code);
}

/**********************************************************************
函数名:max30102_10ms_handler
功能:采集max30102的数据
输入参数:
@p_context:参数传递指针
输出参数:None
返回值:None
说明:
检查10ms采集数据，用于计算血氧和心率
***********************************************************************/
static uint8_t Max30102_flag = 0;                        
//bit0:传感器采集标记        0：未采集   1:在采集
//bit1:是否已经初始化传感器  0:未初始化  1:已初始化
static void max30102_10ms_handler(void * p_context)
{
	  uint32_t err_code;
	
	  if( IS_SET( Max30102_flag, 1 ) == 0 )
		{
        MAX3010x_Init();  
        SET_BIT( Max30102_flag, 1 );			
		}
		else
		    Get_SensorData();    //采集数据
	  
	  err_code = app_timer_start(m_max30102_task_timer_id, MAX30102_10MS_TASK_TIMEOUT, NULL);            //开启ADC定时器
	  APP_ERROR_CHECK(err_code);
}

/**********************************************************************
函数名:GapData_handler
功能:更新广播数据的定时器
输入参数:
@p_context:参数传递指针
输出参数:None
返回值:None
说明:
***********************************************************************/
static void GapData_handler(void * p_context)
{	  
    if( sys_state.ble_connect_state == 0 )    //蓝牙未连接
		{
	      uint32_t err_code;
	
	      sd_ble_gap_adv_stop();   //关广播
		    advertising_init();      //更新广播数据
	
	      err_code = ble_advertising_start(BLE_ADV_MODE_FAST);    //重新广播
        APP_ERROR_CHECK(err_code);
		}
}

/**********************************************************************
函数名:timer_task_create
功能:创建定时器任务
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void timer_task_create(void)
{
	  uint32_t err_code;
	
	  /* 周期为1S的定时器，用于处理一些1S周期相关的事件 */
    err_code = app_timer_create(&m_period_task_timer_id,     //定时器句柄
								APP_TIMER_MODE_REPEATED,                     //周期性定时器
								period_timeout_handler);                     //定时器回调函数
	  APP_ERROR_CHECK(err_code);  

	  /* ADC周期采集定时器 */
    err_code = app_timer_create(&m_adc_task_timer_id,        //定时器句柄
								APP_TIMER_MODE_SINGLE_SHOT,                  //单次定时器
								ADC_timeout_handler);                        //定时器回调函数
	  APP_ERROR_CHECK(err_code);   
	
	  /* 计步算法10ms时间基础 */
	  err_code = app_timer_create(&m_step_10_task_timer_id,    //定时器句柄
								APP_TIMER_MODE_REPEATED,                     //周期性定时器
								step_10ms_handler);                          //定时器回调函数
	 APP_ERROR_CHECK(err_code); 
	
	 /* 计步算法50ms算法定时器 */
	 err_code = app_timer_create(&m_step_50_task_timer_id,     //定时器句柄
								APP_TIMER_MODE_SINGLE_SHOT,                  //单次定时器
								step_50ms_handler);                          //定时器回调函数
	 APP_ERROR_CHECK(err_code); 
	 
	  /* 更新广播数据的定时器 */
	 err_code = app_timer_create(&m_GapData_task_timer_id,     //定时器句柄
								APP_TIMER_MODE_REPEATED,                     //周期定时器
								GapData_handler);                            //定时器回调函数
	 APP_ERROR_CHECK(err_code); 
	 
	 /* max30102采集数据 */
	 err_code = app_timer_create(&m_max30102_task_timer_id,     //定时器句柄
								APP_TIMER_MODE_SINGLE_SHOT,                   //单次定时器
								max30102_10ms_handler);                       //定时器回调函数
	 APP_ERROR_CHECK(err_code); 
}

/**********************************************************************
函数名:timers_start
功能:开启定时器任务
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void timers_start(void)
{
	uint32_t err_code;

	err_code = app_timer_start(m_period_task_timer_id, PERIOD_TASK_TIMEOUT, NULL);      //开启1S周期定时器
	APP_ERROR_CHECK(err_code);
	
	err_code = app_timer_start(m_adc_task_timer_id, ADC_TASK_TIMEOUT, NULL);            //开启ADC定时器
	APP_ERROR_CHECK(err_code);
	
	err_code = app_timer_start(m_step_10_task_timer_id, STEP_10MS_TASK_TIMEOUT, NULL);  //开启计步10ms时间基础定时器
	APP_ERROR_CHECK(err_code);
	
	err_code = app_timer_start(m_step_50_task_timer_id, STEP_50MS_TASK_TIMEOUT, NULL);  //开启计步50ms算法定时器
	APP_ERROR_CHECK(err_code);
	
	err_code = app_timer_start(m_GapData_task_timer_id, GAPDATA_TASK_TIMEOUT, NULL);  //开启修改广播数据定时器
	APP_ERROR_CHECK(err_code);

}

/**********************************************************************
函数名:timers_start_adc_timer
功能:开启ADC定时器
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void timers_start_adc_timer(void)
{
	  uint32_t err_code;
	
    err_code = app_timer_start(m_adc_task_timer_id, ADC_TASK_TIMEOUT, NULL);            //开启ADC定时器
	  APP_ERROR_CHECK(err_code);    
}

/**********************************************************************
函数名:timers_start_Max30102_timer
功能:开启max30102传感器数据采集定时器
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void timers_start_Max30102_timer(void)
{
	   uint32_t err_code;
	
	   if( IS_SET( Max30102_flag,0) == 0 )
		 {
			  SENSOR_ON;                                          //打开传感器电源
        IIC_Init();                                           //IIC初始化
	
        err_code = app_timer_start(m_max30102_task_timer_id, MAX30102_10MS_TASK_TIMEOUT, NULL); //开启max30102数据定时器
	      APP_ERROR_CHECK(err_code); 
			 
			  SET_BIT( Max30102_flag ,0);
		 }			 
}

/**********************************************************************
函数名:timers_stop_Max30102_timer
功能:关闭max30102传感器数据采集定时器
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void timers_stop_Max30102_timer(void)
{
	  uint32_t err_code;
	  
	  if( IS_SET( Max30102_flag,0) != 0 )
		{
        err_code = app_timer_stop(m_max30102_task_timer_id); 
	      APP_ERROR_CHECK(err_code); 
        SENSOR_OFF;
       
   			Max30102_flag = 0;			
		}			
}


