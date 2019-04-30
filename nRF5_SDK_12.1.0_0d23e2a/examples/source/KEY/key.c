/*******************************************************************************************
*          版权所属, C ,2018- ,成都传奇兄弟信息技术有限公司
*
*作者:嵌入式开发部              日期:2018-9-1            版本:V0.0
*说明:
*    按键不同按压的方式进行相应的处理。
*修改:
2019-2-25:
1、关闭长按程序。
2019-2-27:
1、怎加低电量模式，不点亮屏。
2、修改息屏时间为1S    
*******************************************************************************************/
#include <stdio.h>
#include "app_button.h"
#include "app_timer.h"

#include "key.h"
#include "oled_app.h"
#include "stepcounter.h"
#include "heat_sensor.h"
#include "app_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "MAX3010x.h"
#include "nrf_delay.h"

#define KEY_LONG      0     //按键长按  0:不长按    1:长按
#ifdef KEY_DEBUG
 #define key_Msg     NRF_LOG_INFO
#else
 #define key_Msg(...)
#endif

#if KEY_LONG
APP_TIMER_DEF(m_button_timer_id);
#endif
/**********************************************************************
函数名：bsp_button_event_handler
功能:按键事件回调函数
输入参数:
    pin_no:按键IO
    button_action:按键活动
输出参数:None
返回值：None
说明：
    按键活动的响应，结合定时器，可以区分长按还是短按。
***********************************************************************/
static void bsp_button_event_handler(uint8_t pin_no, uint8_t button_action)
{  
    if (pin_no == KEY_PIN)                                          //如果按键是需要的按键
    {
        switch (button_action)                                      //按键活动状态
        {
            case APP_BUTTON_RELEASE:                                //按键释放
            {         
								/******************************* 控制屏幕 ***********************************/
					       if(OLED_ctrl.oled_state_flag == OLED_ON)    //当前屏幕是亮的
					       {
									     
                        switch((uint8_t)(OLED_ctrl.page_num))   //查询当前在哪一个页面
                        {
													  
													  /*********************************日期***************************************/
													  /*
                                如果当前在日期页面，按键按下后，就应该跳到步数页面，重新调整亮屏时间													
   													*/
												    case DATE_P:      
														{
															   SetLastPageInfo(DATE_P, OLED_OFF, OLED_CLOSE_EN, OLED_OFF_TIME);   //设置为日期页面是因为。在这些界面息屏后，都会回到日期界面
															   SetNewPageInfo(STEP_P, OLED_ON, OLED_CLOSE_EN, OLED_OFF_TIME);
															   JumpPage();
														}
														break;
														
														 /*********************************步数***************************************/
														 /*
                                如果当前在步数页面，按键按下后，就应该跳到距离页面，重新调整亮屏时间													
   													*/
	                          case STEP_P:    
                            {
															  SetLastPageInfo(DATE_P, OLED_OFF, OLED_CLOSE_EN, OLED_OFF_TIME);   //设置为日期页面是因为。在这些界面息屏后，都会回到日期界面
															  SetNewPageInfo(DISTANCE_P, OLED_ON, OLED_CLOSE_EN, OLED_OFF_TIME);
															  JumpPage();
														}
       											break;
														
                            /*********************************距离***************************************/		
                            /*
                                如果当前在距离页面，按键按下后，就应该跳到热量页面，重新调整亮屏时间													
   													*/														
	                          case DISTANCE_P: 
														{
															  SetLastPageInfo(DATE_P, OLED_OFF, OLED_CLOSE_EN, OLED_OFF_TIME);   //设置为日期页面是因为。在这些界面息屏后，都会回到日期界面
															  SetNewPageInfo(CALORIE_P, OLED_ON, OLED_CLOSE_EN, OLED_OFF_TIME);
															  JumpPage();
														}
 														break;
														
														/*********************************热量***************************************/	
														 /*
                                如果当前在热量页面，按键按下后，就应该跳到心率页面，重新调整亮屏时间													
   													*/	
	                          case CALORIE_P:
                            {
															  max30102_data.Heart = -3;
															  SetLastPageInfo(DATE_P, OLED_OFF, OLED_CLOSE_EN, OLED_OFF_TIME);   //设置为日期页面是因为。在这些界面息屏后，都会回到日期界面
															  SetNewPageInfo(HEART_P, OLED_ON, OLED_CLOSE_DISEN, OLED_OFF_TIME);
															  JumpPage();  
														}		
                            break;
														
														/*********************************心率***************************************/
														/*
                                如果当前在心率页面，按键按下后，就应该跳到血氧页面，重新调整亮屏时间。没有采集的
                                数据，就不息屏														
   													*/	
	                          case HEART_P:         
														{
															  max30102_data.SpO2 = -3;
															  SetLastPageInfo(DATE_P, OLED_OFF, OLED_CLOSE_EN, OLED_OFF_TIME);   //设置为日期页面是因为。在这些界面息屏后，都会回到日期界面
															  SetNewPageInfo(OXYGEN_P, OLED_ON, OLED_CLOSE_DISEN, OLED_OFF_TIME);
															  JumpPage();  
															  
														}
														break;
														
														/*********************************血氧***************************************/
														/*
                                如果当前在血氧页面，按键按下后，就应该跳到温度页面，重新调整亮屏时间。没有采集的
                                数据，就不息屏													
   													*/	
	                          case OXYGEN_P:       
														{
															  max30102_data.Temper = -3;
															  SetLastPageInfo(DATE_P, OLED_OFF, OLED_CLOSE_EN, OLED_OFF_TIME);   //设置为日期页面是因为。在这些界面息屏后，都会回到日期界面
															  SetNewPageInfo(TEMPER_P, OLED_ON, OLED_CLOSE_DISEN, OLED_OFF_TIME);
															  JumpPage();
														}
														break;
														
														/*********************************温度***************************************/
														/*
                                如果当前在温度页面，按键按下后，就应该跳到MAC页面，重新调整亮屏时间。没有采集的
                                数据，就不息屏													
   													*/
	                          case TEMPER_P:        
														{
															  SetLastPageInfo(DATE_P, OLED_OFF, OLED_CLOSE_EN, OLED_OFF_TIME);   //设置为日期页面是因为。在这些界面息屏后，都会回到日期界面
															  SetNewPageInfo(MAC_P, OLED_ON, OLED_CLOSE_EN, OLED_OFF_TIME);
															  JumpPage();
														}
														break;
														
														/*********************************MAC***************************************/
														/*
                                如果当前在MAC页面，按键按下后，就应该跳到日期页面，重新调整亮屏时间。没有采集的
                                数据，就不息屏													
   													*/
	                          case MAC_P:           
														{
															  SetLastPageInfo(DATE_P, OLED_OFF, OLED_CLOSE_EN, OLED_OFF_TIME);   //设置为日期页面是因为。在这些界面息屏后，都会回到日期界面
															  SetNewPageInfo(DATE_P, OLED_ON, OLED_CLOSE_EN, OLED_OFF_TIME);
															  JumpPage();
														}
														break;
														
														/*********************************闹钟***************************************/
														case ALARM_P:
														{
															  SetNewPageInfo(OLED_ctrl.lastPageNum, OLED_ctrl.lastPageOledState, OLED_ctrl.lastPageKey, OLED_ctrl.lastPageTime);
															  JumpPage(); 
														}
														break;
														
														/*********************************蓝牙断开连接***************************************/
														case BLE_DISCONNECT_P:
														{
															  SetNewPageInfo(OLED_ctrl.lastPageNum, OLED_ctrl.lastPageOledState, OLED_ctrl.lastPageKey, OLED_OFF_TIME);
															  JumpPage(); 	  
														}
														break;
#ifdef BLE_SCAN														
												   	/*********************************Beacon掉线***************************************/
														case BEACON_LOST_P:
														{
															 SetNewPageInfo(OLED_ctrl.lastPageNum, OLED_ctrl.lastPageOledState, OLED_ctrl.lastPageKey, OLED_OFF_TIME);
															 JumpPage(); 
														}
														break;
#endif
						            }
					          }
										/************************************当前屏幕是灭的**********************************/
					          else    
					          {
											   if( sys_state.power_low == 0 )    //电量不低
												 {
													   OLED_Display_On();   
												     if( COLLECT_P != OLED_ctrl.page_num )            //当前不在手机采集页面
														 {		
                                 /* 点亮屏幕显示日期 */              
                                 SetLastPageInfo(DATE_P, OLED_OFF, OLED_CLOSE_EN, OLED_OFF_TIME);   //设置为日期页面是因为。在这些界面息屏后，都会回到日期界面
															   SetNewPageInfo(DATE_P, OLED_ON, OLED_CLOSE_EN, OLED_OFF_TIME);
															   JumpPage();
														 }
														 else     //当前在手机采集页面
														 {
                                 SetNewPageInfo(COLLECT_P, OLED_ON, OLED_CLOSE_EN, OLED_OFF_TIME);
															   JumpPage();														 
														 }
												 }
					          }
             }
             break;
        }
    }
}

/**********************************************************************
函数名：bsp_button_event_handler
功能:按键事件回调函数
输入参数:
    pin_no:按键IO
    button_action:按键活动
输出参数:None
返回值：None
说明：
    按键活动的响应，结合定时器，可以区分长按还是短按。
***********************************************************************/
void buttons_init(void)
{
    uint32_t err_code = NRF_SUCCESS;

    static const app_button_cfg_t app_buttons[] =
    {
        {
				    KEY_PIN,                                //按键引脚
					  APP_BUTTON_ACTIVE_LOW,                 //按键活跃状态
					  NRF_GPIO_PIN_NOPULL,                  //IO口下拉
					  bsp_button_event_handler                //按键事件回调函数
				},
    };
				
				/* 按键初始化 */
    err_code = app_button_init((app_button_cfg_t *)app_buttons,
                                sizeof(app_buttons) / sizeof(app_buttons[0]),
                                APP_TIMER_TICKS(10, APP_TIMER_PRESCALER));
    
    if (err_code == NRF_SUCCESS)
    {
        err_code = app_button_enable();           //使能按键
    }  
}





