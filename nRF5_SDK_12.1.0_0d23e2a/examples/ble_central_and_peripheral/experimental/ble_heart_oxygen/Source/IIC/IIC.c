/**************************************************************************
    版权 (C), 2018- ,成都传奇信息技术有限公叿

作耿硬件郿       
邮箱:polaris.zhao@foxmail.com
版本:V0.0
创建日期:2019-1-14
文件描述:
   初始化IIC通信，使用模拟IIC
修改:
**************************************************************************/
#include "IIC.h"
#include "nrf_delay.h"


/******************************************************************************
Function   : IIC_Start
Description: MCU发起总线开始信叿
Input      : None
Output     : None
Return     : None
Others     : 当SCL高电平时，SDA出现一个下跳沿表示I2C总线启动信号
*******************************************************************************/
void IIC_Start(void)
{
    SDA_OUT();      //sda线输凿
    IIC_SDA_1();;	  	  
    IIC_SCL_1();
    nrf_delay_us(1);
    IIC_SDA_0();     //START:when CLK is high,DATA change form high to low 
    nrf_delay_us(1);
    IIC_SCL_0();     //钳住I2C总线，准备发送或接收数据 
}

/******************************************************************************
Function   : IIC_Stop
Description: MCU发起总线停止信号
Input      : None
Output     : None
Return     : None
Others     : 当SCL高电平时，SDA出现一个上跳沿表示I2C总线停止信号
*******************************************************************************/
void IIC_Stop(void)
{
    SDA_OUT();    //sda线输凿
    IIC_SCL_0();
    IIC_SDA_0();    //STOP:when CLK is high DATA change form low to high
    nrf_delay_us(1);
    IIC_SCL_1(); 
    IIC_SDA_1();    //发送I2C总线结束信号
    nrf_delay_us(1);		
}

/******************************************************************************
Function   : IIC_SendByte
Description: MCU向IIC总线发逿Byte的数捿
Input      : data:需要发送的数据
Output     : None
Return     : None
Others     : SCL下降沿的时候，将SDA的数据发送出县
*******************************************************************************/
void IIC_SendByte(uint8_t data)
{
    uint8_t t;   
    
	  SDA_OUT(); 	    
    IIC_SCL_0();          //拉低时钟开始数据传辿
    
	   for(t=0;t<8;t++)
    {              
        if(0 != ((data & 0x80) >> 7))
				    IIC_SDA_1();
				else
					  IIC_SDA_0();
				
        data <<= 1; 	  
        nrf_delay_us(1);   //对TEA5767这三个延时都是必须的
        IIC_SCL_1();
        nrf_delay_us(1); 
        IIC_SCL_0();	
        nrf_delay_us(1);
    }	 
}


/******************************************************************************
Function   : IIC_ReadByte
Description: MCU从IIC总线读取1Byte的数捿
Input      : None
Output     : None
Return     : None
Others     : SCL下降沿的时候，将SDA的数据发送出县
*******************************************************************************/
uint8_t IIC_ReadByte(void)
{
    uint8_t i = 0;
	   uint8_t value = 0;
	
	   SDA_IN();
	   
	   /* 读取第一个bit为bit7 */
	   for(i = 0;  i < 8; i++ )
	   {
        IIC_SCL_0(); 
        nrf_delay_us(1);
        IIC_SCL_1();
        value <<= 1;
        
				if(READ_SDA())
            value++;   
		    nrf_delay_us(1); 
		 }
			return value;
}

/******************************************************************************
Function   : IIC_WaitACK
Description: MCU产生一个时钟，读取器件的ACK应答信号
Input      : None
Output     : None
Return     : 0:应答正常 
             其他:无应筿
Others     : 
*******************************************************************************/
uint8_t IIC_WaitACK(void)
{
    uint32_t ucErrTime=0;
    
	  SDA_OUT();      //SDA设置为输兿 
    IIC_SDA_1();
	  nrf_delay_us(1);	   
	  IIC_SCL_1();
	  nrf_delay_us(1);	 
	
	  SDA_IN();       //SDA设置为输兿 
	  while(READ_SDA())
	  {
		    ucErrTime++;
        if(ucErrTime > 16*250)
		      {
			         IIC_Stop();
			         return 1;
	      	}
	   }
	   IIC_SCL_0();//时钟输出0 	   
	   return 0;  
}

/******************************************************************************
Function   : IIC_ACK
Description: MCU产生一ACK信号
Input      : None
Output     : None
Return     : None
Others     : 
*******************************************************************************/
void IIC_ACK(void)
{
    IIC_SCL_0();
    SDA_OUT();
    IIC_SDA_0();
    nrf_delay_us(1);
    IIC_SCL_1();
    nrf_delay_us(1);
    IIC_SCL_0();
}

/******************************************************************************
Function   : IIC_NACK
Description: MCU产生一NACK信号
Input      : None
Output     : None
Return     : None
Others     : 
*******************************************************************************/
void IIC_NACK(void)
{
    IIC_SCL_0();
    SDA_OUT();
    IIC_SDA_1();;
    nrf_delay_us(1);
    IIC_SCL_1();
    nrf_delay_us(1);
    IIC_SCL_0();
}

/******************************************************************************
Function   : IIC_Init
Description: 初始化IIC使用的IO叿
Input      : None
Output     : None
Return     : 0:成功   
             其他:失败
Others     : 
*******************************************************************************/
int IIC_Init(void)
{
	   nrf_gpio_cfg_output(PN532_IIC_SCL);
	   nrf_gpio_cfg_output(PN532_IIC_SDA);
	   IIC_Stop();
     return 0;
}

/******************************************************************************
Function   : IIC_unInit
Description: 释放IIC使用的IO口
Input      : None
Output     : None
Return     : 0:成功   
             其他:失败
Others     : 
*******************************************************************************/
int IIC_unInit(void)
{
	   nrf_gpio_cfg_default(PN532_IIC_SCL);
	   nrf_gpio_cfg_default(PN532_IIC_SDA);
	   IIC_Stop();
     return 0;
}








