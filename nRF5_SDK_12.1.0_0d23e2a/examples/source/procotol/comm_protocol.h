#ifndef __COMM_PROTOCOL_H__
#define __COMM_PROTOCOL_H__


#include "boards.h"

/* 命令类型 */
typedef enum{
    CMD_Device_Reset           = 0x00,//重启手环
    CMD_Get_Version            = 0x01,//获取设备版本号
    CMD_Get_Mac                = 0x02,//获取MAC地址
    CMD_Adjust_Time            = 0x03,//校准时间
    CMD_Get_Time               = 0x04,//获取时间
	  CMD_Updata_Step            = 0x05,//实时上传计步数据
	  CMD_Set_Beacon             = 0x06,//设置是否打开Beacon扫描
	  CMD_Set_Beacon_Mac         = 0x07,//设置目标Beacon的MAC地址
	  CMD_Get_Beacon             = 0x08,//获取Beacon扫描是否打开
    CMD_Continuous_Collect     = 0x80,//手机采集
    CMD_Collect_Interval       = 0x81,//设置采集间隔
	  CMD_Set_Alarm              = 0x82,//设置闹钟
}CMD_type_e;

/* 操作类型 */
typedef enum{
    Ret_Success   = 0,                //返回成功
    Ret_operation = 1,                //操作中
    Ret_Error     = 2,                //返回错误
}Operation_type_t;


/**********************************************************************
函数名:BLE_Data_analysis
功能:蓝牙数据按照协议解析
输入参数:
@data:蓝牙接收的数据
@len:蓝牙接收的数据长度
输出参数:None
返回值:None
说明:
***********************************************************************/
void BLE_Data_analysis(const uint8_t *data, const uint8_t len );











#endif

