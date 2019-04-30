/**************************************************************************
    版权 (C), 2018- ,成都传奇信息技术有限公司

作者:硬件部        
邮箱:polaris.zhao@foxmail.com
版本:V0.0
创建日期:2019-1-14
文件描述:
   初始化IIC通信
修改:
**************************************************************************/
#ifndef __IIC_H__
#define __IIC_H__

#include "boards.h"
#include "nrf_gpio.h"
#include "hard_config.h"


//IO方向设置
#define SDA_IN()        nrf_gpio_cfg_input(PN532_IIC_SDA,NRF_GPIO_PIN_PULLUP)
#define SDA_OUT()       nrf_gpio_cfg_output(PN532_IIC_SDA) 
//IO操作函数	 
#define IIC_SCL_0()     nrf_gpio_pin_clear(PN532_IIC_SCL)   //SCL
#define IIC_SCL_1()     nrf_gpio_pin_set(PN532_IIC_SCL)
#define IIC_SDA_0()     nrf_gpio_pin_clear(PN532_IIC_SDA)   //SCL
#define IIC_SDA_1()     nrf_gpio_pin_set(PN532_IIC_SDA) //SDA	 
#define READ_SDA()      nrf_gpio_pin_read(PN532_IIC_SDA)      //输入SDA 

/******************************************************************************
Function   : IIC_Start
Description: MCU发起总线开始信号
Input      : None
Output     : None
Return     : None
Others     : 当SCL高电平时，SDA出现一个下跳沿表示I2C总线启动信号
*******************************************************************************/
void IIC_Start(void);

/******************************************************************************
Function   : IIC_Stop
Description: MCU发起总线停止信号
Input      : None
Output     : None
Return     : None
Others     : 当SCL高电平时，SDA出现一个上跳沿表示I2C总线停止信号
*******************************************************************************/
void IIC_Stop(void);

/******************************************************************************
Function   : IIC_SendByte
Description: MCU向IIC总线发送1Byte的数据
Input      : data:需要发送的数据
Output     : None
Return     : None
Others     : SCL下降沿的时候，将SDA的数据发送出去
*******************************************************************************/
void IIC_SendByte(uint8_t data);

/******************************************************************************
Function   : IIC_ReadByte
Description: MCU从IIC总线读取1Byte的数据
Input      : None
Output     : None
Return     : None
Others     : SCL下降沿的时候，将SDA的数据发送出去
*******************************************************************************/
uint8_t IIC_ReadByte(void);

/******************************************************************************
Function   : IIC_WaitACK
Description: MCU产生一个时钟，读取器件的ACK应答信号
Input      : None
Output     : None
Return     : 0:应答正常 
             其他:无应答
Others     : 
*******************************************************************************/
uint8_t IIC_WaitACK(void);

/******************************************************************************
Function   : IIC_ACK
Description: MCU产生一ACK信号
Input      : None
Output     : None
Return     : None
Others     : 
*******************************************************************************/
void IIC_ACK(void);

/******************************************************************************
Function   : IIC_NACK
Description: MCU产生一NACK信号
Input      : None
Output     : None
Return     : None
Others     : 
*******************************************************************************/
void IIC_NACK(void);

/******************************************************************************
Function   : IIC_Init
Description: 初始化IIC使用的IO口
Input      : None
Output     : None
Return     : 0:成功   
             其他:失败
Others     : 
*******************************************************************************/
int IIC_Init(void);

/******************************************************************************
Function   : IIC_unInit
Description: 释放IIC使用的IO口
Input      : None
Output     : None
Return     : 0:成功   
             其他:失败
Others     : 
*******************************************************************************/
int IIC_unInit(void);




#endif








