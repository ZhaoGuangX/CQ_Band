#ifndef __OLED_APP_H__
#define __OLED_APP_H__

#include "oled.h"
#include "utc.h"

#define OLED_OFF_TIME     10     //息屏时间
/* 页面 */
typedef enum{
  DATE_P = 0,      //日期页面         0
	STEP_P ,         //步数页面         1
	DISTANCE_P,      //行走的距离页面   2
	CALORIE_P,       //热量界面         3
	HEART_P,         //心率界面         4
	OXYGEN_P,        //血氧界面         5
	TEMPER_P,        //温度界面         6
	MAC_P,           //MAC地址界面      7
	ALARM_P,         //闹钟界面         8
	COLLECT_P,       //手机采集页面     9
	BLE_CONNECT_P,   //蓝牙连接         10
	BLE_DISCONNECT_P,//蓝牙断开连接     11
#ifdef BLE_SCAN
	BEACON_LOST_P,   //信标丢失         12
#endif
	
}page_e;

/* 屏幕电源开关 */
#define OLED_OFF          0
#define OLED_ON           1

/* 允许息屏开关 */
#define OLED_CLOSE_DISEN    0
#define OLED_CLOSE_EN       1

typedef struct
{
    uint8_t oled_state_flag:1;       //当前屏幕是亮还是灭  0:灭    1:亮
    uint8_t oled_off_key:1;          //0：不能开始倒计时      1:可以开始倒计时
    uint8_t time_off:6;              //息屏倒计时   max = 64
	  uint8_t lastPageOledState:1;     //进入当前页面，屏幕是亮着的还是灭着的。  0:灭    1:亮
	  uint8_t lastPageKey:1;           //上一个页面能否息屏      //0：不能开始倒计时      1:可以开始倒计时
	  uint8_t lastPageTime:6;          //上一个页面剩余的倒计时时间
	  page_e  lastPageNum;             //保存上一个页面，如果开始屏幕是熄灭的，上一个页面就是日期页面
	  page_e  page_num ;               //当前处于哪一个页面
	  
}oled_ctrl_t;

extern oled_ctrl_t OLED_ctrl;

typedef struct{  
    volatile s16 heart;                   //心率
    volatile s16 high;                    //血压高
    volatile s16 low;                     //血压低
}heart_sensor_t;
extern heart_sensor_t heart_sensor_data;

/**********************************************************************
函数名:OLED_Change_Display
功能:改变页面和动态刷新页面
输入参数：
@tm:时间日期
输出参数:None
返回值:None
说明:
***********************************************************************/
void OLED_Change_Display(const UTCTime_t *tm);

#ifdef BLE_SCAN
/**********************************************************************
函数名:OLEDE_dispaly_date
功能:显示时间和连接状态
输入参数：
@tm:时间日期
@connect_state:连接状态
@scan_state:扫描状态
@batty:电量等级 1:1格        2:2格   3:三格    4:4格  0：0格
输出参数:None
返回值:None
说明:
***********************************************************************/
void OLEDE_dispaly_date(const UTCTime_t *tm, const uint8_t scan_state, const uint8_t connect_state, const uint8_t batty);
#else
/**********************************************************************
函数名:OLEDE_dispaly_date
功能:显示时间和连接状态
输入参数：
@tm:时间日期
@state:连接状态  1；连接
@batty:电量等级 1:1格        2:2格   3:三格    4:4格  0：0格
输出参数:None
返回值:None
说明:
***********************************************************************/
void OLEDE_dispaly_date(const UTCTime_t *tm, const uint8_t state, const uint8_t batty);
#endif

/**********************************************************************
函数名:OLEDE_dispaly_step
功能:显示步数
输入参数：
@step_num:步数
输出参数:None
返回值:None
说明:
***********************************************************************/
void OLEDE_dispaly_step(uint32_t step_num);

/**********************************************************************
函数名:OLEDE_dispaly_Distance
功能:显示行走的距离
输入参数：
@distance:行走的距离，(km*10)
输出参数:None
返回值:None
说明:
***********************************************************************/
void OLEDE_dispaly_Distance(uint32_t distance);

/**********************************************************************
函数名:OLEDE_dispaly_Calorie
功能:显示消耗的卡路里
输入参数：
@Cal:热量(cal)
输出参数:None
返回值:None
说明:
    根据行走的距离计算消耗的热量
***********************************************************************/
void OLEDE_dispaly_Calorie(uint32_t Cal);

/**********************************************************************
函数名:OLEDE_dispaly_HeartRate
功能:显示心率
输入参数：
@heart_num:心率
输出参数:None
返回值:None
说明:
heart_num < 0,表示心率采集准备，倒计时
heart_num = 0,表示正在采集
***********************************************************************/
void OLEDE_dispaly_HeartRate(s16 heart_num);

/**********************************************************************
函数名:OLEDE_dispaly_Oxygen
功能:显示血氧
输入参数：
@oxygen:血氧
输出参数:None
返回值:None
说明:
***********************************************************************/
void OLEDE_dispaly_Oxygen(s16 oxygen);

/**********************************************************************
函数名:OLEDE_dispaly_Temperature
功能:显示温度
输入参数：
@oxygen:血氧
输出参数:None
返回值:None
说明:
***********************************************************************/
void OLEDE_dispaly_Temperature(s16 temp);

/**********************************************************************
函数名:OLEDE_dispaly_alarm
功能:显示闹钟
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void OLEDE_dispaly_alarm(void);

/**********************************************************************
函数名:OLEDE_dispaly_mac
功能:显示MAC
输入参数:None
输出参数:None
返回值:None
说明:
hig, 0x< 0,表示心率采集准备，倒计时
hig, 0x= 0,表示正在采集
***********************************************************************/
void OLEDE_dispaly_mac(void);

/**********************************************************************
函数名:OLEDE_dispaly_Temperature
功能:显示温度
输入参数：
@oxygen:血氧
输出参数:None
返回值:None
说明:
***********************************************************************/
void OLEDE_dispaly_Phone_Collect(void);

/**********************************************************************
函数名:OLEDE_dispaly_BLE_Connect
功能:显示蓝牙连接界面
输入参数：
@time:显示时间，为0时跳入之前的页面
输出参数:None
返回值:None
说明:
***********************************************************************/
void OLEDE_dispaly_BLE_Connect(int time);

/**********************************************************************
函数名:OLEDE_dispaly_BLE_Disconnect
功能:显示蓝牙断开界面
输入参数：
输出参数:None
返回值:None
说明:
    蓝牙断开页面需要手动切换，或者倒计时时间到。且在显示的时候还需要震动。
***********************************************************************/
void OLEDE_dispaly_BLE_Disconnect(void);

#ifdef BLE_SCAN
/**********************************************************************
函数名:OLEDE_dispaly_BeaconLost
功能:显示蓝牙信标丢失
输入参数：
输出参数:None
返回值:None
说明:
    蓝牙断开页面需要手动切换，或者倒计时时间到。且在显示的时候还需要震动。
***********************************************************************/
void OLEDE_dispaly_BeaconLost(void);

/**********************************************************************
函数名:JumpPage
功能:页面跳转,刚跳转的时候刷新页面
输入参数：None
输出参数:None
返回值:None
说明:
***********************************************************************/
void JumpPage(void);

/**********************************************************************
函数名:SetLastPageInfo
功能:设置上一个页面的信息
输入参数：
@page:需要跳转的页面
@state:页面状态，亮屏还是熄灭
@closeSwitch:息屏开关。能否息屏
@closeTime:亮屏时间
输出参数:None
返回值:None
说明:
***********************************************************************/
void SetLastPageInfo(const page_e page, const int8_t state, const int8_t closeSwitch, const int8_t closeTime);

/**********************************************************************
函数名:SetNewPageInfo
功能:设置当前页面的信息
输入参数：
@page:需要跳转的页面
@state:页面状态，亮屏还是熄灭
@closeSwitch:息屏开关。能否息屏
@closeTime:亮屏时间
输出参数:None
返回值:None
说明:
***********************************************************************/
void SetNewPageInfo(const page_e page, const int8_t state, const int8_t closeSwitch, const int8_t closeTime);

#endif

















#endif

