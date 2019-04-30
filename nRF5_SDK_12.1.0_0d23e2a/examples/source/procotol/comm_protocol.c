 /*******************************************************************************************
*          版权所属, C ,2018- ,成都传奇兄弟信息技术有限公司
*
*作者:嵌入式开发部              日期:2018-9-1            版本:V0.0
*说明:
*    该工程基于，Nordic官方nRF5_SDK_12.1.0_0d23e2a.实现BLE主从一体通信。该
*工程需要驱动MPU6050,OLED等硬件，实现计步，心率采集等功能。
*修改:
2019-3-13:
    1、增加血氧，心率连续采集命令。
		2、增加自动采集间隔命令。
		3、增加闹钟。
2019-4-1:
    1.增加获取Beacon是否扫描的命令。
*******************************************************************************************/

#include "comm_protocol.h"
#include "ble_salve.h"
#include "nrf_delay.h"
#include "app_error.h"
#include <string.h>
#include <stdio.h>
#include "utc.h"
#include "heat_sensor.h"
#include "oled_app.h"
#include "ble_config.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

extern uint16_t collect_interval;       //自动采集间隔 
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
void BLE_Data_analysis(const uint8_t *data, const uint8_t len )
{
	   if(data[0] == 0xFF)                      //查询设备类型
				{
					    NRF_LOG_INFO("CMD_Device_Device\r\n");
					    const uint8_t buff[] = {DEVICE_TYPE};
				
				     BLE_Salve_SendData(buff,1);   
				}
    else if( data[0] == PRODUCT_NUMBER )     //判断产品号是否正确
    {
        switch( data[1] )
        {
            /********************************* 重启设备 **************************************/
            case CMD_Device_Reset:
            {
                uint8_t buff[3] = {0};
                
                if( data[2] == 0 )    //判断数据是否正确
                {
                    NRF_LOG_INFO("CMD_Device_Reset\r\n");

                    buff[0] = PRODUCT_NUMBER;
                    buff[1] = CMD_Device_Reset;
                    buff[2] = Ret_Success;
                    BLE_Salve_SendData(buff,3);
                    nrf_delay_ms(1000);
                    APP_ERROR_CHECK(!NRF_SUCCESS);    //通过错误校验重启
                }
            }
            break;

            /******************************* 获取版本号 ************************************/
            case CMD_Get_Version:
            {
                uint8_t buff[4] = {0};
                
                if( data[2] == 0 )    //判断数据是否正确
                {
                    NRF_LOG_INFO("CMD_Get_Version\r\n");

                    buff[0] = PRODUCT_NUMBER;
                    buff[1] = CMD_Get_Version;
                    buff[2] = HARDWARE_VERSION;
                    buff[3] = SOFTWARE_VERSION;
                    
                    BLE_Salve_SendData(buff,4);
                }
            }
            break;

            /******************************* 获取MAC地址 **********************************/
            case CMD_Get_Mac:
            {
                uint8_t buff[8] = {0};
                ble_gap_addr_t device_addr;
                
                if( data[2] == 0 )    //判断数据是否正确
                {
                    NRF_LOG_INFO("CMD_Get_Mac\r\n");

	                  sd_ble_gap_addr_get(&device_addr);   //获取MAC地址
                    buff[0] = PRODUCT_NUMBER;
                    buff[1] = CMD_Get_Mac;
									  buff[2] = device_addr.addr[5];
									  buff[3] = device_addr.addr[4];
									  buff[4] = device_addr.addr[3];
									  buff[5] = device_addr.addr[2];
									  buff[6] = device_addr.addr[1];
									  buff[7] = device_addr.addr[0];
                    BLE_Salve_SendData(buff,8);
                }
            }
            break;

            /***************************** 校准时间 *************************************/
            case CMD_Adjust_Time:
            {
                NRF_LOG_INFO("CMD_Adjust_Time\r\n");
													   uint8_t buff[3] = {0};
																
                if(len == 9)
                {
                   UTCTime_t tm;
                   union{
                       uint16_t a;
                       uint8_t b[2];
                   }c;

                   c.b[1] = data[2];
                   c.b[0] = data[3];
                  
                   tm.year   = c.a;
                   tm.month  = data[4]-1;
                   tm.day    = data[5]-1;
                   tm.hour   = data[6];
                   tm.minute = data[7];
                   tm.second = data[8];
                   NRF_LOG_INFO("%d-%d-%d %d:%d:%d\r\n",tm.year,tm.month+1,tm.day+1,tm.hour,tm.minute,tm.second);
                   SetClock_UTC(&tm);              //设置UTC时间

                   buff[0] = PRODUCT_NUMBER;
                   buff[1] = CMD_Adjust_Time;
                   buff[2] = Ret_Success;
                   BLE_Salve_SendData(buff,3);
         
                }
                else
                {
                   buff[0] = PRODUCT_NUMBER;
                   buff[1] = CMD_Adjust_Time;
                   buff[2] = 1;
                   BLE_Salve_SendData(buff,3);
                }
                
            }
            break;
            
            /****************************** 获取时间 *************************************/
            case CMD_Get_Time:
            {
                uint8_t buff[9] = {0};
                UTCTime_t tm;
                
                if( data[2] == 0 )    //判断数据是否正确
                {
                   union{
                       uint16_t a;
                       uint8_t b[2];
                   }c;
                   
                    NRF_LOG_INFO("CMD_Get_Time\r\n");
                    GetClock_UTC(&tm);
                    c.a = tm.year;

                    buff[2] = c.b[1];
                    buff[3] = c.b[0];
                    buff[4] = tm.month+1;
                    buff[5] = tm.day+1;
                    buff[6] = tm.hour;
                    buff[7] = tm.minute;
                    buff[8] = tm.second;
                    
                    buff[0] = PRODUCT_NUMBER;
                    buff[1] = CMD_Get_Time;
	                  
                    BLE_Salve_SendData(buff,9);
                }    
            }
            break;
						
						/********************* 设置是否扫描Beacon ***************************/
						case CMD_Set_Beacon:
						{
							  uint8_t buff[3] = {0};
								
						    save_param.OpenBeaconFlag = data[2];
							  
								/* 返回数据 */	
                buff[0] = data[0];
                buff[1] = CMD_Set_Beacon;
                buff[2] = 0x00;	

                BLE_Salve_SendData(buff,3);		
						}
						break;

						/********************* 设置目标Beacon的MAC ***************************/
						case CMD_Set_Beacon_Mac:
						{
						    uint8_t buff[3] = {0};   

								memcpy(save_param.Beacon_Mac, &data[2], 6);
								
                	/* 返回数据 */	
                buff[0] = data[0];
                buff[1] = CMD_Set_Beacon_Mac;
                buff[2] = 0x00;	

                BLE_Salve_SendData(buff,3);		

                NRF_LOG_INFO("Beacon New MAC:%02X:%02X:%02X:%02X:%02X:%02X",save_param.Beacon_Mac[0],
								             save_param.Beacon_Mac[1],save_param.Beacon_Mac[2],save_param.Beacon_Mac[3],
								             save_param.Beacon_Mac[4],save_param.Beacon_Mac[5]);								
						}
						break;
						
						/************************* 获取Beacon是否扫描 ******************************/
						case CMD_Get_Beacon:
						{
						    if( data[2] == 0 )
								{
								    uint8_t buff[3] = {0}; 
  
                    /* 返回数据 */	
                    buff[0] = data[0];
                    buff[1] = CMD_Get_Beacon;
                    buff[2] = save_param.OpenBeaconFlag;	

                    BLE_Salve_SendData(buff,3);		
										
								}									
						}
						break;
						
            /************************* 连续采集 *********************************/
            case CMD_Continuous_Collect:
            {
							  /* 不管当前在上面界面，手机采集优先级最高 */
                if( data[2] == 0x01 )    
								{
								    timers_start_Max30102_timer();    //开启连续采集
									
									  if( OLED_ctrl.oled_state_flag == OLED_OFF )
									      OLED_Display_On();  
										
										SetNewPageInfo(COLLECT_P, OLED_ON, OLED_CLOSE_EN, OLED_OFF_TIME);
										JumpPage();
								}
                else                     
								{	
								    timers_stop_Max30102_timer();    //关闭连续采集

									  SetLastPageInfo(DATE_P, OLED_OFF, OLED_CLOSE_EN, OLED_OFF_TIME);
									 
  									if( OLED_ctrl.oled_state_flag == 0)
									      OLED_Display_On();
										
										SetNewPageInfo(DATE_P, OLED_ON, OLED_CLOSE_EN, OLED_OFF_TIME);
										JumpPage();  //显示	
										
										NRF_LOG_INFO("Finish Collect...\r\n");
								}									
            }
            break;

            /**************************** 设置自动采集间隔 *************************/
            case CMD_Collect_Interval:
            {
                 collect_interval = save_param.Collect_interval =  data[2] * 60; 
								 
								 uint8_t buff[3] = {0};
									
									  /* 返回数据 */	
                buff[0] = data[0];
                buff[1] = CMD_Collect_Interval;
                buff[2] = 0x00;	

                BLE_Salve_SendData(buff,3);			
            }
            break;
						
						 /************************* 设置闹钟 *********************************/
            case CMD_Set_Alarm:
            {
							  uint8_t buff[3] = {0};
								
                save_param.alarm[0] = data[2];     //设置闹钟的时 
                save_param.alarm[1] = data[3];     //设置闹钟的分 
                save_param.alarm[2] = data[4];     //设置闹钟的秒 		

                /* 返回数据 */	
                buff[0] = data[0];
                buff[1] = CMD_Set_Alarm;
                buff[2] = 0x00;	

                BLE_Salve_SendData(buff,3);								
            }
            break;
        }

    }
    
}








