/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#ifndef BOARDS_H
#define BOARDS_H

#include "nrf_gpio.h"

#if defined(BOARD_NRF6310)
  #include "nrf6310.h"
#elif defined(BOARD_PCA10000)
  #include "pca10000.h"
#elif defined(BOARD_PCA10001)
  #include "pca10001.h"
#elif defined(BOARD_PCA10002)
  #include "pca10000.h"
#elif defined(BOARD_PCA10003)
  #include "pca10003.h"
#elif defined(BOARD_PCA20006)
  #include "pca20006.h"
#elif defined(BOARD_PCA10028)
  #include "pca10028.h"
#elif defined(BOARD_PCA10031)
  #include "pca10031.h"
#elif defined(BOARD_PCA10036)
  #include "pca10036.h"
#elif defined(BOARD_PCA10040)
  #include "pca10040.h"
#elif defined(BOARD_CQXD_NETWORK)
  #include "hard_config.h"
#else
#error "Board is not defined"

#endif

#define PRODUCT_NUMBER       0X01  //产品代号
#define HARDWARE_VERSION     2     //硬件版本
#define SOFTWARE_VERSION     8     //软件版本,BLE4.1
/*
软件版本说明:
0 : 2019-3-19打包。基本功能完成，不包含flash存储，功耗还能继续优化。此版本程序为测试程序。不能用于量产。
1 : 2019-3-20打包。功能完成，修改一下BUG。
        1.修改心率算法，采集心率相比之前稳定快速。
		    2.在采集完成时增加震动功能。
2 : 2019-3-21打包。修改BUG.
        1.修改热量计算公式。
3 : 2019-3-25打包，增加蓝牙连接，断开的图标
4 : 2019-3-26打包，测试信标扫描使用。
5 : 2019-3-29打包，新增Beacon功能。
6 : 2019-4-3打包。更新了命令码。
7 : 2019-4-4打包，修改Bug.闹钟响应后。不反回之前的页面。
8 : 2019-4-23打包，修改手机采集时心率出现太慢的现象。
*/

/* 当前系统状态 */
typedef struct{
#ifdef BLE_SCAN
	  uint8_t Beacon_scan_state:1;    //信标扫描状态  0:没有信标   1有信标
#endif
	  uint8_t ble_connect_state:1;    //蓝牙连接状态  0:蓝牙未连接   1:蓝牙已连接
	  uint8_t batty_grabe:3;
	  uint8_t power_low:1;            //电池电量低  0:电量不低    1:电量低
	  uint8_t bool_preeeure_valid:1;  //血压数据有效标记  0:无效    1:有效
	  uint8_t bool_preeeure_flag:1;   //血压数据显示      0:不显示   1:显示
	  volatile uint16_t batty_Voltage;
}sys_state_t;


#define SAVE_FLAG     0x55         //用于存放在存储标记中，每次开机启动都判断该值，
                                   //如果相等表示存储的参数有效，如果不相等表示存储的参数无效，将采用默认参数。
/* 需要保存的参数 */
typedef struct{
	  uint8_t save_flag;               //存储标记  
    uint8_t sex;                   //性别 0:男    1:女 
    uint8_t height;                //身高(cm)
    uint8_t	weight;                //体重(kg)
	  uint8_t age;                   //年龄
	  uint8_t alarm[3];              // alarm[0]-->时    alarm-->分    alarm-->秒
	  uint16_t Collect_interval;     //自动采集间隔
#ifdef BLE_SCAN
	  uint8_t OpenBeaconFlag;        //打开Beacon标记   0:未打开   1:打开
	  uint8_t Beacon_Mac[6];         //绑定的BeaconMAC地址,最多绑定1个。
#endif
}save_param_t;

extern save_param_t save_param;
extern sys_state_t sys_state;
#endif
