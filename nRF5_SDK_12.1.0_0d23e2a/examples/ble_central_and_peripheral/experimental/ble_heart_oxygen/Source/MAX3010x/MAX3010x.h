/**************************************************************************
    版权 (C), 2018- ,成都传奇信息技术有限公司

作者:硬件部        
邮箱:polaris.zhao@foxmail.com
版本:V0.0
创建日期:2019-1-14
文件描述:
   心率传感器MAX3010x的驱动程序
修改:
**************************************************************************/
#ifndef __MAX_3010X_H__
#define __MAX_3010X_H__

#include "boards.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#define MAX3010x_WR_BASE_ADDR     0xAE          /* 读写地址  */
#define MAX3010x_WR	              0		           /* 写控制bit */
#define MAX3010x_RD	              1		           /* 读控制bit */

typedef struct{
    int SpO2;           //血氧浓度
	  int Heart;          //心率值
	  int Temper;         //温度
}MAX3010x_Data_t;

extern MAX3010x_Data_t max30102_data;

/* MAX3010x的寄存器地址 */
//status
#define INTERRUPT_STATUS1               0x00    /*< 中断状态寄存器1 */
#define INTERRUPT_STATUS2               0x01    /*< 中断状态寄存器2 */
#define INTERRUPT_ENABLE1               0x02    /*< 中断使能寄存器1 */
#define INTERRUPT_ENABLE2               0x03    /*< 中断使能寄存器2 */

//FIFO
#define FIFO_WRITE_POINTER              0x04    /*< FIFO写指针 */
#define FIFO_OVERFLOW_COUNTER           0x05    /*< FIFO溢出计数 */
#define FIFO_READ_POINTER               0x06    /*< FIFO读指针 */
#define FIFO_DATA_REG                   0x07    /*< FIFO数据寄存器 */

//config
#define FIFO_CONFIG                     0x08    /*< FIFO控制寄存器 */
#define MODE_CONFIG                     0x09    /*< 模式控制 */
#define SPO2_CONFIG                     0x0A    /*< 血氧控制 */
#define LED1_PULSE_AMPLITUDE            0x0C    /*< LED1脉冲幅度*/
#define LED2_PULSE_AMPLITUDE            0x0D    /*< LED2脉冲幅度*/
#define PROXIMITY_LED_PULSE_AMPLITUDE   0x10    /*< 接近模式LED的脉冲幅度 */
#define MULTI_LED_CONTROL_REG1          0x11    /*< 多个LED控制寄存器1 */           
#define MULTI_LED_CONTROL_REG2          0x12    /*< 多个LED控制寄存器2 */ 

//die temperature   芯片内部温度
#define DIE_TEMP_INTEGER                0x1F    /*< 内部温度整数部分 */
#define DIE_TEMP_FRACTION               0x20    /*< 内部温度小数部分 */
#define DIE_TEMP_CONFIG                 0x21    /*< 内部温度控制寄存器 */

//proximity function
#define PROXIMITY_INTERRUPT_THRESHOLD   0x30    /*< 接近终端临界值 */

//part ID
#define REVISION_ID                     0xFE    /*< 修整ID */
#define PART_ID                         0xFF    /*< 部分ID */

/******************************************************************************
Function   : MAX3010x_FIFO_Read
Description: 从MAX3010x的FIFO中，读取数据
Input      : None
Output     : iR:红外信号值
             Red:红色信号值
Return     : None
Others     : 
*******************************************************************************/
void MAX3010x_FIFO_Read(int *iR, int *Red);
	
/******************************************************************************
Function   : MAX3010x_Init
Description: 初始化MAX3010x传感器
Input      : None
Output     : None
Return     : 0:成功
             其他:失败
Others     : 
*******************************************************************************/
int MAX3010x_Init(void);

/******************************************************************************
Function   : MAX3010x_Get_ID
Description: 获取MAX3010x的ID
Input      : None
Output     : None
Return     : 0:获取失败
             其他:获取到的ID
Others     : 设备ID由两部分组成，高8位为REVISION_ID，低8位为PART_ID
*******************************************************************************/
uint16_t MAX3010x_Get_ID(void);

/******************************************************************************
Function   : MAX3010x_Get_DIETemp
Description: 获取MAX3010x的内部温度
Input      : None
Output     : integer:温度的整数部分
             fraction:温度的小数部分
Return     : None
Others     : 设备ID由两部分组成，高8位为REVISION_ID，低8位为PART_ID
*******************************************************************************/
void MAX3010x_Get_DIETemp(uint8_t *integer, uint8_t *fraction);


/********************************************************************************************
                       根据采集的数据计算心率血氧
********************************************************************************************/
#define true   1
#define false  0
#define FS            100
#define BUFFER_SIZE   (FS* 5)    //采集样本数量
#define HR_FIFO_SIZE  7
#define MA4_SIZE      4       // DO NOT CHANGE
#define HAMMING_SIZE  5       // DO NOT CHANGE
#define min(x,y) ((x) < (y) ? (x) : (y))

/******************************************************************************
Function   : Maxim_HeartRate_And_OxygenSaturation
Description: 根据采样值计算心率血氧
Input      : iR_Buffer:红外数据缓存
             iRDataLength:红外数据缓存长度(红外数据和红色数据长度一样)
             Red_Buffer:红色数据缓存
Output     : SpO2:血氧浓度值
             SpO2_valid:血氧浓度是否有效  (1:有效)
             HeartRate:心率值
             HeartRate_valid:心率值是否有效(1:有效)
Return     : None
Others     : 
*******************************************************************************/
void maxim_heart_rate_and_oxygen_saturation(uint32_t *pun_ir_buffer,  int32_t n_ir_buffer_length, uint32_t *pun_red_buffer, int32_t *pn_spo2, int8_t *pch_spo2_valid, 
                              int32_t *pn_heart_rate, int8_t  *pch_hr_valid);

/******************************************************************************
Function   : First_Sampling
Description: 第一次采样，获取样本
Input      : None
Output     : ir_databuff:采集的红外数据缓存
             red_databuff:采集的红色数据缓存
Return     : None
Others     : 第一次采样确定信号范围
*******************************************************************************/
void First_Sampling(int * ir_databuff, int *red_databuff);


/******************************************************************************
Function   : Get_Heart_SensorData
Description: 单次获取传感器原始数据，放回缓存
Input      : None
Output     : None
Return     : None
Others     : 
           1、每获取一个数据，将缓存空间向前移动一个空间出来，放入新数据。
           2、间隔10ms采集一个数据
*******************************************************************************/
void Get_SensorData(void);

/******************************************************************************
Function   : Get_Heart_Oxygen
Description: 获取心率血氧
Input      : None
Output     : None
Return     : None
Others     : 
*******************************************************************************/
void Get_Heart_Oxygen(uint8_t reset);

#endif




