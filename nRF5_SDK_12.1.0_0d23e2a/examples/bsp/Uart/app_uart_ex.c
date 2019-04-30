/*******************************************************************************************
*          版权所属, C ,2018- ,成都传奇兄弟信息技术有限公司
*
*作者:嵌入式开发部              日期:2018-9-1            版本:V0.0
*说明:
*    串口相关函数
*修改:
*******************************************************************************************/
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "boards.h"
#include "app_uart.h"
#include "app_uart_ex.h"
#include "ble_salve.h"
#include "timer.h"
#include "app_timer.h"

#include "ble_config.h"
#include "ble_master.h"
#include "ble_salve.h"

#include "heat_sensor.h"
#include "oled_app.h"
#include "hard_config.h"
#include "comm_protocol.h"
#include "stepcounter.h"
#include "app_gpio.h"


#include "nrf_log.h"
#include "nrf_log_ctrl.h"

/**********************************************************************
函数名:uart_event_handle
功能:串口事件回调函数
输入参数:
@p_event:串口事件
输出参数:None
返回值:None
说明:
***********************************************************************/
extern uint8_t adjust_flag ;
extern heart_sensor_t Save_heart_sensor_data;  //保留上次获取的心率血压数据，用于广播
static void uart_event_handle(app_uart_evt_t * p_event)
{
   static uint8_t data[UART_RX_BUF_SIZE] = {0}; 
   static uint16_t index = 0;
   static uint8_t rec_flag = 0;     //0:不可接收        1:可接受
		                                   /*
                                       bit0:心率血压数据可接收
                                       bit1:校准数据返回
                                      */
		static uint8_t collect_num = 0;  //采集数据的次数
		static uint8_t collect_data[3][3];//采集的数据缓存
    uint8_t buff[9] = {0};
                   
    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
         {
            UNUSED_VARIABLE(app_uart_get(&data[index]));      //读串口数据

            /* 采集心率血压返回 */
            if( data[index]  == 0xFD || READ_FLAG(rec_flag,0))
            {
                SET_FLAG(rec_flag,0);
                index++;
						    
                if(index == 6)
                {
                    NRF_LOG_HEXDUMP_INFO(data,6);            //调试信息，打印接收的数据
//										BLE_Salve_SendData(data,6);             //返回给APP 		  

                    if((data[1] != 0 && data[1] != 0xFF && data[2] != 0 && data[2] != 0xFF && data[3] != 0 && data[3] != 0xFF) &&   //确保数据有效,心率小于150
                        ((OLED_ctrl.page_num == HEART_P && heart_sensor_data.heart >= 0 )) )         //确保已过倒计时       
                    {
										    NRF_LOG_INFO("data[1] = %02x,data[2] = %02x,data[3] = %02x",data[1],data[2],data[3]);
												sys_state.bool_preeeure_valid = 1;
											
											  /* 将数据放入缓存 */
												collect_data[0][collect_num] = data[1];    //0空间存放收缩压
                        collect_data[1][collect_num] = data[2];    //1空间存放舒张压
                        collect_data[2][collect_num] = data[3];    //2空间存放心率
                        collect_num++;	
																					  
												if( collect_num >= 3 )
												{
												    Close_Heat_Sensor();
														Motor_Drive(); 
														collect_num = 0;
																									
														heart_sensor_data.high   = collect_data[0][0];       //收缩压
														heart_sensor_data.low    = collect_data[1][0];       //舒张压
														heart_sensor_data.heart  = collect_data[2][0];	      //心率
																									
														/* 有效数据取采集的数据的最小值 */
														for(uint8_t i = 0; i < 3; i++)
														{
														    if( heart_sensor_data.high > collect_data[0][i] )
																    heart_sensor_data.high  = collect_data[0][i];       //收缩压
																																
																if( heart_sensor_data.low > collect_data[1][i] )
                                    heart_sensor_data.low = collect_data[1][i];       //舒张压
																																
																if( heart_sensor_data.heart > collect_data[2][i] )
                                    heart_sensor_data.heart = collect_data[2][i];	      //心率
	
                            }
																												
                            union{
                                uint32_t a;
                                uint8_t b[4];
                            }c;

                            c.a = stepcount;
                            buff[0] = PRODUCT_NUMBER;
                            buff[1] = CMD_Adjust_Heart_Sensor;
                            buff[2] = c.b[3];
                            buff[3] = c.b[2];
                            buff[4] = c.b[1];
                            buff[5] = c.b[0];
                            Save_heart_sensor_data.high = buff[6] = heart_sensor_data.high;
                            Save_heart_sensor_data.low = buff[7] = heart_sensor_data.low;
                             Save_heart_sensor_data.heart = buff[8] = heart_sensor_data.heart;
                        
                            BLE_Salve_SendData(buff,9);           //返回给APP 
										   }
																						 
                   }
									 else     //还没有获取到数据
									 {
										    sys_state.bool_preeeure_valid = 0;
										    if( sys_state.bool_preeeure_flag != 0 )
												{
										        if( data[1] == 0xFF && data[2] == 0xFF &&data[3] == 0xFF )     //未接触皮肤
												    {
												        heart_sensor_data.heart = 0xFF;
                                heart_sensor_data.high = 0xFF;
                                heart_sensor_data.low = 0xFF;													
												    }
												    else if( (data[1] == 0) || (data[2] == 0) || (data[3] == 0) )        //有效数据还没有出来
												    {
												        /* 给一个随机数 */
                            		heart_sensor_data.heart = 50+rand()%30;    //50+30内的随机数
                                heart_sensor_data.high = 100+rand()%40;    //100+40内的随机数
                                heart_sensor_data.low = 50+rand()%30;    //50+30内的随机数													
												    }
												}
									 }

                   index = 0;                                     //清除接收标记
									 memset(data,0,UART_RX_BUF_SIZE);               //清理接收缓存
                   CLEAN_FLAG(rec_flag,0);                        //清除可接收标记
								}
					  }

            /* 传感器校准返回 */
            if( data[index]  == 0xFE || READ_FLAG(rec_flag,1))
            {
                SET_FLAG(rec_flag,1);
                index++;
						    
                if(index == 6)
                {
                    NRF_LOG_HEXDUMP_INFO(data,6);                //调试信息，打印接收的数据
                
                    if( data[3] == 0x00 || data[3] == 0x01 )
                    {
										    if( data[3] == 0x00 )       //校准成功
												    Close_Heat_Sensor();
												else
												    adjust_flag = 1;        //校准中
											
																								
                        buff[0] = PRODUCT_NUMBER;
                        buff[1] = CMD_Adjust_Heart_Sensor;
	                      buff[2] = data[3];
                        BLE_Salve_SendData(buff,3);          //返回给APP
                    }

                    
                    index = 0;                               //清除接收标记
									  memset(data,0,UART_RX_BUF_SIZE);         //清理接收缓存
                    CLEAN_FLAG(rec_flag,1);                  //清除可接收标记
							 }
					}
        }
        break;

        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}

/**********************************************************************
函数名:UartInit
功能:串口初始化
输入参数:None
输出参数:None
返回值:None
说明:
初始化串口时，串口的RX和TX必须上拉。
***********************************************************************/
void UartInit(void)
{
    uint32_t err_code;
	  
    app_uart_comm_params_t comm_params =
    {
        .rts_pin_no = RTS_PIN_NUMBER,
        .cts_pin_no = CTS_PIN_NUMBER,
        .rx_pin_no = RX_PIN_NUMBER,
        .tx_pin_no = TX_PIN_NUMBER,
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
        .use_parity = false,
        .baud_rate = UARTE_BAUDRATE_BAUDRATE_Baud115200,
    };
		
    APP_UART_FIFO_INIT( &comm_params,           //初始化串口
                        UART_RX_BUF_SIZE,
                        UART_TX_BUF_SIZE,
                        uart_event_handle,
                        APP_IRQ_PRIORITY_LOWEST,
                        err_code);
    APP_ERROR_CHECK(err_code);
}

/**********************************************************************
函数名:fputc
功能:支持C标准库printf
输入参数:
@ch:需要发送的数据
p_file：
输出参数:None
返回值:发送的数据
说明:
***********************************************************************/
//int fputc(int ch, FILE * p_file)
//{
//    UNUSED_PARAMETER(p_file);

//    UNUSED_VARIABLE(app_uart_put((uint8_t)ch));
//    return ch;
//}

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
void Uart_SendData(const uint8_t *pData, const uint16_t Len)
{
    for(uint16_t i = 0; i < Len; i++)
	  {
		    app_uart_put((uint8_t)pData[i]);    
		}
}
