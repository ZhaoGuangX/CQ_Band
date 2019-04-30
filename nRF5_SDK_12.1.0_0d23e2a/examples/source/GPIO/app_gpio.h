/*******************************************************************************************
*          版权所属, C ,2018- ,成都传奇兄弟信息技术有限公司
*
*作者:嵌入式开发部              日期:2018-11-12            版本:V0.0
*说明:
*    初始化GPIO的输入输出功能。
*修改:
*******************************************************************************************/

#ifndef APP_GPIO_H__
#define APP_GPIO_H__

#include "boards.h"
#include "nrf_gpio.h"
#include "IIC.h"

/* 电机驱动 */
#define MOTOR_ON          {nrf_gpio_pin_set(MOTOR_DRIVE);}
#define MOTOR_OFF         {nrf_gpio_pin_clear(MOTOR_DRIVE);}

/* 传感器使能 */
#define SENSOR_ON         {nrf_gpio_cfg_output(SENSOR_POWER_EN); nrf_gpio_pin_set(SENSOR_POWER_EN);}
#define SENSOR_OFF        {nrf_gpio_pin_clear(SENSOR_POWER_EN); nrf_gpio_cfg_default(SENSOR_POWER_EN); IIC_unInit();}

/* 充电状态 */
#define CHARGER_SATE      nrf_gpio_pin_read(CHARGER_STATUS)
/**********************************************************************
函数名:GPIO_Init
功能:GPIO初始化
输入参数:None
输出参数:None
返回值:None
说明:
该函数初始化GPIO输入输出功能
***********************************************************************/
void GPIO_Init(void);

/**********************************************************************
函数名:Motor_Drive
功能:电机震动
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void Motor_Drive(void);

/**********************************************************************
函数名:GPIO_UnInit_All
功能:关闭所有的IO口
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void GPIO_UnInit_All(void);





#endif
