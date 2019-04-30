#ifndef _STEPCOUNTER_H_
#define _STEPCOUNTER_H_

#include <stdint.h>
#include "boards.h"
#include "utc.h"

#define DISTANCE_CONST        0.4
#define CALORIE_CONST         6.050//8.214      //(0.8214 * 10)

/* 运动数据结构体 */
typedef struct{
    uint32_t stepcount;      //步数
	  uint16_t distance;       //行走的距离(km)*10    距离 = 升高(Cm) * K(系数) *步数   K = 0.4
	  uint16_t calorie;        //卡路里(kcal)*10      热量 = 体重(kg) * 距离(KM) * K(系数)   K = 0.8214
}Movement_data_t;

extern Movement_data_t movement;              //用来记录运动的数据
extern long   StepDetector_tick;              //计步检测的心跳时间

#define  GRAVITY_EARTH         9.80665f      /* 重力加速度常量        */
#define  GSENSOR_RESOLUTION    16384         /* 1G加速度对应的输出值 */

void setSteps(int initValue) ;
void get_Gsenson_Value(void);

/*************************************************************************
函数名:Step_Updata
功能:通过蓝牙上传步数
输入参数:None
输出参数:None
返回值:None
说明:
连续走十步才会开始计步
连续走了9步以下,停留超过3秒,则计数清空
*************************************************************************/
void Step_Updata(const UTCTime_t * tm);

#endif

