/*******************************************************************************************
*          版权所属, C ,2019- ,成都传奇兄弟信息技术有限公司
*
*作者:嵌入式开发部              日期:2019-3-7            版本:V1.0
*说明:
   1.该工程基于，Nordic官方nRF5_SDK_12.1.0_0d23e2a.实现BLE主从一体通信。该
工程需要驱动LI3DH,OLED等硬件，实现计步，使用MAX30102实现心率血氧的采集等功能。
   2.该工程基于血压采集手环，修改过来。
   3.本工程完成的功能有实现显示时间，采集心率、血氧、温度等，采集步数，设置闹钟等功能。
*修改:
2019-3-12:
    增加行走距离，热量，血氧浓度，心率，温度，闹钟等界面。
2019-3-14:
    调试DFU(固件升级功能)。
2019-3-18:
    增加数据存储功能，将一些用户数据写入芯片内部Flash中。
2019-3-19:
    1.Flash存储，与DFU有冲突，导致不能广播，所以暂时不使用Flash。
		2.修该传感器使能控制的IO口，降低功耗。睡眠功耗为(150us-300us间波动)。持续广播。
		3.关闭NRF_LOG功能。
		4.使用bootloader的功耗比不使用bootloader功耗高100uA左右。
2019-3-20:
    1.修改心率算法，采集心率相比之前稳定快速。
		2.在采集完成时增加震动功能。
2019-3-25:
    1.增加蓝牙连接和断开界面。
2019-3-26:
    1.增加扫描Beacon功能。
2019-3-27:
    1.调试Beacon扫描
		2.修改显示操作逻辑
2019-3-29:
    1.修复操作逻辑上的BUG。
		2.更改了命令码。
		3.增加了控制Beacon扫描的命令码。
2019-4-1:
    1.增加获取Beacon是否扫描的命令。
2019-4-5:
    1.修改Bug:闹钟响应后。不返回之前的页面。
2019-4-23:
		1.修改手机采集时心率出现太慢的现象。
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
#include "ble_conn_state.h"
#include "fstorage.h"
#include "fds.h"
#include "nrf_drv_gpiote.h"
#include "nrf_delay.h"
#include "app_fds.h"
#include "ble_config.h"
#include "ble_salve.h"
#include "ble_master.h"
#include "timer.h"
#include "adc.h"
#include "utc.h"
#include "key.h"
#include "oled.h"
#include "stepcounter.h"
#include "oled_app.h"
#include "app_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "MAX3010x.h"

//#define FLASH        //需要掉电保存数据

/* 需要保存的参数 */
save_param_t save_param = {
	  .save_flag = SAVE_FLAG,
    .sex = 1,
	  .height = 170,
	  .weight = 65,
	  .age = 25,
#ifdef BLE_SCAN
	  .OpenBeaconFlag = 0,
	  .Beacon_Mac = {0x04, 0xED, 0x68, 0x69, 0x20, 0x68},
#endif
};


/* 蓝牙连接状态，电池电量状态等 */
sys_state_t sys_state = {
#ifdef BLE_SCAN
	  .Beacon_scan_state = 0,
#endif
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

#ifdef FLASH
/**********************************************************************
函数名:cheack_param
功能:校准系统需要保存的参数
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
#define SAVE_PARAM_ADDR       0     //存储参数的地址
static void cheack_param(void)
{
	  save_param_t temp_param;
	  uint32_t err_code;
	
	  memset( &temp_param, 0, sizeof( temp_param ) );
    err_code = Flash_Read( SAVE_PARAM_ADDR, (uint32_t *)&temp_param , sizeof( temp_param )/4+1  );   //读取保存的参数
    APP_ERROR_CHECK(err_code);
	
	  NRF_LOG_INFO("save_flag = 0x%02X\r\n",temp_param.save_flag);
	  if( temp_param.save_flag == SAVE_FLAG )     //如果参数有效
		{
		    memcpy( &save_param, &temp_param, sizeof( temp_param ) );
			  NRF_LOG_INFO("System param Read Success!\r\n");
			
			  NRF_LOG_INFO("height = %d\r\n",save_param.height);
			  NRF_LOG_INFO("weight = %d\r\n",save_param.weight);
			  NRF_LOG_INFO("age = %d\r\n",save_param.age);
			  NRF_LOG_INFO("Collect_interval = %d\r\n",save_param.Collect_interval);
			  NRF_LOG_INFO("Alarm time =%d:%d:%d\r\n",save_param.alarm[0],save_param.alarm[1],save_param.alarm[2]);
 
		}
    else     //参数无效，就使用默认参数，并写入到到flash
		{
		    err_code = Flash_Write(SAVE_PARAM_ADDR , (uint32_t *)&save_param , sizeof( save_param )/4+1); //写入参数
			  APP_ERROR_CHECK(err_code);
			  NRF_LOG_INFO("System param Read Fail!,Write param\r\n");
		}			
}

#endif

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
	
	  NRF_POWER->DCDCEN = 1;      //使能DCDC转换
	
    err_code = NRF_LOG_INIT(NULL);  //初始化日志功能
    APP_ERROR_CHECK(err_code);
	
	  NRF_LOG_INFO("Device Type:0x%02X,HardWare:%d,SoftWare:%d\r\n",PRODUCT_NUMBER,HARDWARE_VERSION,SOFTWARE_VERSION);
	
	
#ifdef FLASH	
	  Flash_Init();                                       //flash初始化
		cheack_param();                                     //校验参数
#endif		
	  timers_init();                                      //初始化时钟
		
		  /* 启用浮点数中断,不然功耗会比较高 */
	  NVIC_SetPriority(FPU_IRQn, APP_IRQ_PRIORITY_LOW);
    NVIC_EnableIRQ(FPU_IRQn);
		GPIO_UnInit_All();                                  //关闭所有的IO口
		
	
    ble_stack_init();                                   //初始化协议栈
    gap_params_init();                                  //初始化GAP参数
    conn_params_init();                                 //初始化连接参数
    services_init();                                    //初始化服务
    advertising_init();                                 //初始化广播	

#ifdef BLE_SCAN
	  db_discovery_init();
    nus_c_init();                                       //初始化客户端
#endif

    buttons_init();                                     //初始化按键
	  GPIO_Init();                                        //GPIO初始化
    timer_task_create();                                //创建定时器任务
    SetClock_UTC(&tm);                                  //设置UTC时间

		adv_scan_start();                                   //开启广播
		timers_start();                                     //开启定时器	

		Motor_Drive();
		
		for (;;)
    {
        power_manage();
    }
}
      
