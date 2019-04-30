/***************************************************************************
 版权 (c), 2018- ,成都传奇兄弟信息技术有限公司

作者:硬件部                日期:2018-10-11              版本:V0.0


描述:
    用于从心率/血压传感器中读取心率值,血压值
***************************************************************************/
#include "app_uart_ex.h"
#include "nrf_delay.h"
#include "heat_sensor.h"
#include "timer.h"
#include "app_timer.h"
#include "oled_app.h"
#include "stepcounter.h"
#include "comm_protocol.h"
#include "ble_salve.h"
#include "app_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

/* 命令 */
                                /*收缩压 舒张压 心率       */
static uint8_t adjust[]   = {0xFE, 0x6E, 0x4C, 0x46, 0x00, 0x00 };     //校准血压脉搏
static uint8_t readData[] = {0xFD, 0x00, 0x00, 0x00, 0x00, 0x00 };     //读取血压脉搏
static uint8_t readECG[]  = {0xF9, 0x00, 0x00, 0x00, 0x00, 0x00 };     //读取心电数据

Send_Cmd_type_e SendCMDType = SENSOR_READ;      //定义血压发送的命令类型，默认为读取数据

/* 定义一个定时器 */
APP_TIMER_DEF(m_sensor_task_timer_id);              //1S定时器
APP_TIMER_DEF(m_CloseSensor_task_timer_id);         //关闭传感器功能定时器
static uint8_t sensor_state = 0;                    //传感器状态         0:没有采集        1:采集中 
static uint8_t sendNum[SENSOR_MAX] = {0};           //各命令发送的超时次数

/*************************************************************************
函数名:Open_Heat_Sensor
功能:打开心率血压传感器采集
输入参数:
@ms:采集间隔
@mode:定时器运行模式
APP_TIMER_MODE_SINGLE_SHOT     //单次
APP_TIMER_MODE_REPEATED        //循环
输出参数:None
返回值:None
*************************************************************************/
void Open_Heat_Sensor(uint32_t ms,app_timer_mode_t mode)
{
	   if( sensor_state == 0 )
		 {
			   SENSOR_ON;             //开启传感器电源
			   nrf_delay_ms(500);     //等待500ms
			   /* 初始化串口 */
		     UartInit();                     //初始化串口		 
			 
	       /* 开启采集定时器 */
         app_timer_start(m_sensor_task_timer_id, APP_TIMER_TICKS(ms, mode), NULL);    //开启按键定时器
			   sensor_state = 1; 
		 }
}

/*************************************************************************
函数名:Close_Heat_Sensor
功能:关闭心率血压传感器采集
输入参数:None
输出参数:None
返回值:None
*************************************************************************/
void Close_Heat_Sensor(void)
{
	  if( sensor_state == 1 )
		{
        /* 关闭采集定时器 */
	      (void)app_timer_stop(m_sensor_task_timer_id);
	      
			  sensor_state = 0;     //标记没有采集

        /* 开启关闭传感器电源定时器 */
        app_timer_start(m_CloseSensor_task_timer_id,  APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER), NULL);     //1S后关闭
		}
}

/*************************************************************************
函数名:sensor_timeout_handler
功能:发送相应的命令
输入参数:
@pcontext:传递参数的指针
输出参数:None
返回值:None
说明:
    通过命令的发送次数和设置的定时器事件，确定一条命令的超时时间
*************************************************************************/
uint8_t adjust_flag = 0;   //0:未校准     1:校准中
static void sensor_timeout_handler(void * pcontext)
{       
    switch(SendCMDType)
    {
        // 校准传感器
        case SENSOR_ADJUST:    
        {
            sendNum[SENSOR_ADJUST]++;                    //发送次数++
            if(sendNum[SENSOR_ADJUST] <= 30)              //发送没有超过30次
						{
						     if( adjust_flag == 0 )
                    Uart_SendData(adjust, sizeof(adjust));   //发送命令
						}
            else              //发送次数超过3次
            {
                NRF_LOG_INFO("adjust sensor timout...\r\n");
                /* 0. 关闭定时器,停止发送。关闭定时器有两种可能:<1>接收到正确的返回        <2>超时，此处是属于第二种      */
                Close_Heat_Sensor();
                
								uint8_t buff[3] = {0};
                /* 1. 向手机返回错误 */
								buff[0] = PRODUCT_NUMBER;
                buff[1] = CMD_Adjust_Heart_Sensor;
	              buff[2] = 0x02;
                BLE_Salve_SendData(buff,3);          //返回给APP    
            }
                
        }
        break;  

        // 读取血压心率数据 
        case SENSOR_READ  :   
        {
				     Uart_SendData(readData, sizeof(readData));	
					
            
            if(sendNum[SENSOR_READ] < 15)               //发送没有超过10次(超时事件时3*1 = 30s),开始显示假数据
						{
						    sendNum[SENSOR_READ]++;                      //发送次数++
                sys_state.bool_preeeure_flag = 0;							
						}
						else
						     sys_state.bool_preeeure_flag = 1;		    
						
//            else
//            {
//                 NRF_LOG_INFO("read data sensor timout...\r\n");
//                 /* 0. 关闭定时器,停止发送。关闭定时器有两种可能:<1>接收到正确的返回        <2>超时，此处是属于第二种        */
//                 Close_Heat_Sensor();    

//                 /* 1. 显示采集失败 */
//                 if( OLED_ctrl.page_num == HEART_P )    //如果当前在心率页面
//                 {
//                     heart_sensor_data.heart = 1;       //显示采集失败
//                 }
//                 else if(OLED_ctrl.page_num == BLOOD_P) //当前在心率血压页面
//                 {
//                     heart_sensor_data.high = 1;       //显示采集失败
//                 }
//                 /* 2.向APP发送数据 */
//                  union{
//                            uint32_t a;
//                            uint8_t b[4];
//                  }c;
//                  uint8_t buff[9] = {0};

//                  c.a = stepcount;
//                  buff[0] = PRODUCT_NUMBER;
//                  buff[1] = CMD_Adjust_Heart_Sensor;
//                  buff[2] = c.b[3];
//                  buff[3] = c.b[2];
//                  buff[4] = c.b[1];
//                  buff[5] = c.b[0];
//                  buff[6] = 0xFF;
//                  buff[7] = 0XFF;
//                  buff[8] = 0XFF;
//                        
//                  BLE_Salve_SendData(buff,9);          //返回给APP 
//            }
        }
        break; 

        // 读取心电图数据
        case SENSOR_ECG   :  
        {
            Uart_SendData(readECG, sizeof(readECG));  
        }break;     
    } 
}

/*************************************************************************
函数名:Close_sensor_handler
功能:关闭传感器功能的定时器
输入参数:
@pcontext:传递参数的指针
输出参数:None
返回值:None
说明:
    需要关闭的有串口，和传感器电源。由于关闭传感器功能有一部分是在串口回调函数执行，
这样会出现串口错误，所以需要单独创建任务，进行回调，关闭串口和电源。
*************************************************************************/
static void Close_sensor_handler(void * pcontext)
{
    /* 数据采集完成关闭串口 */
    app_uart_close();      //关闭采集
					    
    /* 关闭传感器电源 */
    SENSOR_OFF;  
    NRF_LOG_INFO("Close Sensor Power");	
}

/*************************************************************************
函数名:Heat_sensor_Init
功能:初始化心率血压传感器采集
输入参数:None
输出参数:None
返回值:None
描述:初始化采集传感器，是创建采集定时器
*************************************************************************/
void Heat_sensor_Init(void)
{
    uint32_t err_code;
	
	  /* 周期采集定时器 */
    err_code = app_timer_create(&m_sensor_task_timer_id,         //定时器句柄
								APP_TIMER_MODE_REPEATED,                         //周期性定时器
								sensor_timeout_handler);                         //定时器回调函数
	  APP_ERROR_CHECK(err_code);  
	
	  /* 关闭采集时,使用定时器，延迟1S关闭 */
	  err_code = app_timer_create(&m_CloseSensor_task_timer_id,    //定时器句柄
								APP_TIMER_MODE_SINGLE_SHOT,                      //单次定时器
								Close_sensor_handler);                           //定时器回调函数
	  APP_ERROR_CHECK(err_code);  
}

/*************************************************************************
函数名:Sensor_Adjust
功能:校准传感器
输入参数:
@sysstolic:收缩压
@diastole:舒张压
@heart:心率
输出参数:None
返回值:None
说明:
    校准传感器，3S发送一次命令
*************************************************************************/
void Sensor_Adjust(const uint8_t sysstolic, const uint8_t diastole, const uint8_t heart )
{
    /* 1.填充数据 */
    adjust[1] = sysstolic;
    adjust[2] = diastole;
    adjust[3] = heart;

    /* 2.修改发送命令类型,归零发送次数 */
    SendCMDType = SENSOR_ADJUST;
    sendNum[SENSOR_ADJUST] = 0;

    /* 3.开启定时器，循环发送 */
    Open_Heat_Sensor(2000,APP_TIMER_MODE_REPEATED);
}

/*************************************************************************
函数名:Sensor_ReadData
功能:读取心率血压值
输入参数:None
输出参数:None
返回值:None
说明：
    读取数据，1S读一次
*************************************************************************/
void Sensor_ReadData(void)
{
    /* 1.修改发送命令类型,归零发送次数 */
    SendCMDType = SENSOR_READ;
    sendNum[SENSOR_READ] = 0;

    /* 2.开启定时器，循环发送 */
    Open_Heat_Sensor(1000,APP_TIMER_MODE_REPEATED);
}

/*************************************************************************
函数名:Sensor_ReadECG
功能:读取心电数据
输入参数:
@EN:打开采集或关闭采集
输出参数:None
返回值:None
说明：
    读取心电数据，10mS读一次
    ECG值需要连续采集，所以需要设置开始和结束
*************************************************************************/
void Sensor_ReadECG(Enable_e EN)
{
    if( EN == ENABLE )
    {
        /* 1.修改发送命令类型,归零发送次数 */
        SendCMDType = SENSOR_READ;
        sendNum[SENSOR_READ] = 0;

        /* 2.开启定时器，循环发送 */
        Open_Heat_Sensor(10,APP_TIMER_MODE_REPEATED);
    }
    else
    {
        Close_Heat_Sensor();     
    }
}



