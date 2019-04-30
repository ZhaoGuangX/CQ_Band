/*******************************************************************************************
*          版权所属, C ,2018- ,成都传奇兄弟信息技术有限公司
*
*作者:嵌入式开发部              日期:2018-9-1            版本:V0.0
*说明:
*    SPI通信，用于驱动LIS3dH
*修改:
*
*******************************************************************************************/
#ifndef __SPI_H__
#define __SPI_H__


#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include <string.h>


/*************************************************************************
函数名:SPI_Init
功能:SPI事件回调函数
输入参数:
@p_event:传递事件的指针
输出参数:None
返回值:None
说明:
*************************************************************************/
void SPI_Init(void);

/*************************************************************************
函数名:SPI_UnInit
功能:关闭SPI
输入参数:None
输出参数:None
返回值:None
说明:
*************************************************************************/
void SPI_UnInit(void);

/************************************************************************
* Function Name		: SPI_WriteReg
* Description		: Generic Writing function. It must be fullfilled with either
*			: I2C or SPI writing function
* Input			: Register Address, Data to be written
* Output		: None
* Return		: None
*************************************************************************/
uint8_t SPI_WriteReg(uint8_t WriteAddr, uint8_t Data);

/************************************************************************
* Function Name		: LIS3DH_ReadReg
* Description		: Generic Reading function. It must be fullfilled with either
*			: I2C or SPI reading functions					
* Input			: Register Address
* Output		: Data REad
* Return		: None
**************************************************************************/
uint8_t SPI_ReadReg(uint8_t Reg, uint8_t* Data);


#endif

