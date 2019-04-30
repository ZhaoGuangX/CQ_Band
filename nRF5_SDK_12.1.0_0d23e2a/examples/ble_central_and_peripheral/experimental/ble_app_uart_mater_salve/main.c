/*******************************************************************************************
*          版权所属, C ,2018- ,成都传奇兄弟信息技术有限公司
*
*作者:嵌入式开发部              日期:2018-9-1            版本:V0.0
*说明:
*    该工程基于，Nordic官方nRF5_SDK_12.1.0_0d23e2a.实现BLE主从一体通信。该
*工程需要驱动LI3DH,OLED等硬件，实现计步，心率采集等功能。
*修改:
2018-11-5:
1、增加flash存储功能。
2019-2-26:
1、串口需要上拉，不然初始化不成功(硬件上拉)。
2、将心率血压页面整合为一个页面。
2019-2-27:
1、修改电池电压采集，和电量等级算法。
2、增加低电量程序。当电池电量低压3.4V时将关闭广播。关闭计步。关闭显示。用于时间的运行。
2019-3-5：
1、屏蔽蓝牙输出的一些调试信息
*******************************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "app_uart.h"
#include "boards.h"
#include "app_uart_ex.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "ble_db_discovery.h"
#include "ble_nus.h"
#include "ble_nus_c.h"
#include "app_uart_ex.h"
#include "ble_conn_state.h"
#include "fstorage.h"
#include "fds.h"
#include "nrf_drv_gpiote.h"
#include "nrf_delay.h"
#include "app_fds.h"

#include "ble_config.h"
#include "ble_master.h"
#include "ble_salve.h"
#include "timer.h"
#include "adc.h"
#include "utc.h"
#include "key.h"
#include "oled.h"
#include "stepcounter.h"
#include "oled_app.h"
#include "heat_sensor.h"
#include "app_gpio.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

/* 蓝牙连接状态，电池电量状态等 */
sys_state_t sys_state = {
    .ble_connect_state = 0,             //蓝牙连接状态
	  .batty_grabe = 4,                   //电池等级
	  .power_low = 0,                     //标记电池电量低
};

/**********************************************************************
函数名:assert_nrf_callback
功能:系统固装回调函数
输入参数:
@line_num:行号
@p_file_name:文件名
输出参数:None
返回值:None
说明:
该函数具体实现什么功能，未知
***********************************************************************/
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(0xDEADBEEF, line_num, p_file_name);
}

/**********************************************************************
函数名:power_manage
功能:电源管理
输入参数:None
输出参数:None
返回值:None
说明:
该函数通过电源管理，实现芯片低功耗运行
***********************************************************************/
static void power_manage(void)
{
    ret_code_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

/**********************************************************************
函数名:FPU_IRQHandler
功能:浮点数中断，清楚fpscr
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
#define FPU_EXCEPTION_MASK 0x0000009F
void FPU_IRQHandler(void)
{
    uint32_t *fpscr = (uint32_t *)(FPU->FPCAR+0x40);
    (void)__get_FPSCR();

    *fpscr = *fpscr & ~(FPU_EXCEPTION_MASK);
}

/**********************************************************************
函数名:main
功能:主函数，工程运行入口，对各功能模块进行初始化
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
int main(void)
{
    uint32_t err_code;
    /****************utc test*******************************/
    UTCTime_t tm;
    tm.year   = 2019;
    tm.month  = 2-1;
    tm.day    = 26-1;
    tm.hour   = 11;
    tm.minute = 8;
    tm.second = 20;
    /*******************************************************/
	
    err_code = NRF_LOG_INIT(NULL);  //初始化日志功能
    APP_ERROR_CHECK(err_code);
		
	  /* 启用浮点数中断 */
	  NVIC_SetPriority(FPU_IRQn, APP_IRQ_PRIORITY_LOW);
    NVIC_EnableIRQ(FPU_IRQn);
	
	  timers_init();                  //初始化时钟
    ble_stack_init();               //初始化协议栈
    gap_params_init();              //初始化GAP参数
    conn_params_init();             //初始化连接参数
    services_init();                //初始化服务
    advertising_init();             //初始化广播	
   

    buttons_init();                 //初始化按键
//    Flash_Init();                   //flash初始化
	  GPIO_Init();                    //GPIO初始化
    timer_task_create();            //创建定时器任务
		Heat_sensor_Init();             //初始化心率传感器采集
    SetClock_UTC(&tm);              //设置UTC时间
   
		adv_scan_start();               //开启广播
		timers_start();                 //开启定时器	

		Motor_Drive();
		
		for (;;)
    {
        power_manage();
    }
}
      
