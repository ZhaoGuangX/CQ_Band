/*******************************************************************************************
*          版权所属, C ,2018- ,成都传奇兄弟信息技术有限公司
*
*作者:嵌入式开发部              日期:2018-11-12            版本:V0.0
*说明:
*    初始化GPIO的输入输出功能。
*修改:
2019-3-19:
    修改传感器电源使能引脚的控制，以降低功耗。
*******************************************************************************************/
#include "app_gpio.h"
#include "timer.h"

APP_TIMER_DEF(m_Motor_Timer_id);    //电机驱动定时器

/*************************************************************************
函数名:Motor_handler
功能:电机驱动定时器
输入参数:
@pcontext:传递参数的指针
输出参数:None
返回值:None
说明:
*************************************************************************/
static void Motor_handler(void * pcontext)
{
    MOTOR_OFF;
	   nrf_gpio_cfg_default(MOTOR_DRIVE);
}

/**********************************************************************
函数名:GPIO_Init
功能:GPIO初始化
输入参数:None
输出参数:None
返回值:None
说明:
该函数初始化GPIO输入输出功能
***********************************************************************/
void GPIO_Init(void)
{
    /* 配置GPIO为输出 */ 
//    nrf_gpio_cfg_output(SENSOR_POWER_EN);    	

//    SENSOR_OFF;
	
    /* 配置GPIO为输入 */
    nrf_gpio_cfg_input(CHARGER_STATUS,NRF_GPIO_PIN_PULLUP);      //设置输入上拉
	
	  /* 添加电机定时器 */
	  uint32_t err_code;
	
    err_code = app_timer_create(&m_Motor_Timer_id,               //定时器句柄
							                         	APP_TIMER_MODE_SINGLE_SHOT,     //单次定时器
								                         Motor_handler);                 //定时器回调函数
	   APP_ERROR_CHECK(err_code);
    
}

/**********************************************************************
函数名:Motor_Drive
功能:电机震动
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void Motor_Drive(void)
{
	  nrf_gpio_cfg_output(MOTOR_DRIVE);   
    MOTOR_ON;
    app_timer_start(m_Motor_Timer_id, APP_TIMER_TICKS(300, APP_TIMER_PRESCALER), NULL);     //关闭	
}

/**********************************************************************
函数名:GPIO_UnInit_All
功能:关闭所有的IO口
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void GPIO_UnInit_All(void)
{
    for( int i = 0; i < 32; i++ )
	      nrf_gpio_cfg_default(i);    
}
