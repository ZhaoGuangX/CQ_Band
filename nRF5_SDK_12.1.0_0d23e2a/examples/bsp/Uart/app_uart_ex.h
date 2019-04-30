/*******************************************************************************************
*          版权所属, C ,2018- ,成都传奇兄弟信息技术有限公司
*
*作者:嵌入式开发部              日期:2018-9-1            版本:V0.0
*说明:
*    串口相关头文件
*修改:
*
*******************************************************************************************/
#ifndef APP_UART_EX_H_
#define APP_UART_EX_H_


#include <stdbool.h>
#include <stdint.h>
#include "app_uart.h"

#define UART_TX_BUF_SIZE            256                                           /**< Size of the UART TX buffer, in bytes. Must be a power of two. */
#define UART_RX_BUF_SIZE            256                                           /**< Size of the UART RX buffer, in bytes. Must be a power of two. */


/**********************************************************************
函数名:UartInit
功能:串口初始化
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void UartInit(void);

/**********************************************************************
函数名:Uart_SendData
功能:串口直接发送数据
输入参数:
@pData:需要发送的数据的指针
@Len:需要发送的数据的长度
输出参数:None
返回值:None
说明:
***********************************************************************/
void Uart_SendData(const uint8_t *pData, const uint16_t Len);

#endif
