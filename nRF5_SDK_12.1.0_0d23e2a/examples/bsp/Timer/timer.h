/*******************************************************************************************
*          版权所属, C ,2018- ,成都传奇兄弟信息技术有限公司
*
*作者:嵌入式开发部              日期:2018-9-1            版本:V0.0
*说明:
*    定时器功能的头文件，确定个定时器的定时时间
*
*******************************************************************************************/
#ifndef __TIMER_H__
#define __TIMER_H__

#include "app_timer.h"
#include "oled_app.h"

//#define TIME_DEBUG  

#define PERIOD_TASK_TIMEOUT                 APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER)    //定时器间隔，ms
#define ADC_TASK_TIMEOUT                    APP_TIMER_TICKS(3000, APP_TIMER_PRESCALER)    //adc采集间隔
#define STEP_10MS_TASK_TIMEOUT              APP_TIMER_TICKS(10, APP_TIMER_PRESCALER)      //计步10ms
#define MAX30102_10MS_TASK_TIMEOUT          APP_TIMER_TICKS(10, APP_TIMER_PRESCALER)      //血氧传感器采集频率ms
#define STEP_50MS_TASK_TIMEOUT              APP_TIMER_TICKS(30, APP_TIMER_PRESCALER)      //计步50ms
#define GAPDATA_TASK_TIMEOUT                APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)    //更新广播数据的间隔

/**********************************************************************
函数名:timers_init
功能:对时钟进行初始化
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void timers_init(void);

/**********************************************************************
函数名:timer_task_create
功能:创建定时器任务
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void timer_task_create(void);

/**********************************************************************
函数名:timers_start
功能:开启定时器任务
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void timers_start(void);

/**********************************************************************
函数名:timers_start_adc_timer
功能:开启ADC定时器
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void timers_start_adc_timer(void);

/**********************************************************************
函数名:timers_start_Max30102_timer
功能:开启max30102传感器数据采集定时器
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void timers_start_Max30102_timer(void);

/**********************************************************************
函数名:timers_stop_Max30102_timer
功能:关闭max30102传感器数据采集定时器
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void timers_stop_Max30102_timer(void);

#endif
