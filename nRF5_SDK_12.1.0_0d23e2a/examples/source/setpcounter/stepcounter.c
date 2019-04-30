#include "nrf_gpio.h"
#include "nrf_error.h"
#include "nrf_delay.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "stepcounter.h"
#include "boards.h"
#include "lis3dh_driver.h"
#include "ble_salve.h"
#include "comm_protocol.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

Movement_data_t movement = {
    .stepcount = 0,
	  .distance = 0,
	  .calorie = 0,
};

long   StepDetector_tick = 0;        //计步检测的心跳时间

float oriValues[3] = {0};            //存放三轴数据的加速度，单位g 

int ValueNum = 4;                    //用于存放计算阈值的波峰波谷差值的个数
float tempValue[4] = {0};            //用于存放计算阈值的波峰波谷差值
int tempCount = 0;                   //已获取用于存放计算阈值的波峰波谷差值的个数

bool isDirectionUp = false;          //是否上升的标志位
int continueUpCount = 0;             //持续上升次数
int continueUpFormerCount = 0;       //上一点的持续上升的次数，为了记录波峰的上升次数
bool lastStatus = false;             //上一点的状态，上升还是下降
float peakOfWave = 0;                //波峰值
float valleyOfWave = 0;              //波谷值
long timeOfThisPeak = 0;             //此次波峰的时间

long timeOfLastPeak = 0;             //上次波峰的时间

long timeOfNow = 0;                  //当前的时间

float gravityNew = 0;                //当前传感器的值
float gravityOld = 0;                //上次传感器的值
float InitialValue = (float) 1.3;    //动态阈值需要动态的数据，这个值用于这些动态数据的阈值
float ThreadValue = (float) 2.0;     //初始阈值
int TimeInterval = 25;               //波峰波谷时间差 250ms

static int count = 0;                //临时计步变量，用于检测10步以下时使用
static long timeOfLastPeak_1;          //上一次波峰时间
static long timeOfThisPeak_1;          //这一次波峰时间


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
void Step_Updata(const UTCTime_t * tm)	
{
	   static uint32_t step_save = 0;
	
     /* 每天晚上00:00:00,清0步数 */
	  if( tm->hour == 0 && tm->minute == 0 && tm->second == 0 )
		{
		    step_save = movement.stepcount = 0;
			  movement.calorie = 0;
			  movement.distance = 0;
		}
    
	  /******************************************** 当步数有变化时，需要上传给APP ******************************************/
    if( step_save != movement.stepcount )
    {
         union{
             uint32_t a;
             uint8_t b[4];
         }c;
                     
         c.a = movement.stepcount;
         uint8_t buff[6] = {0};

         buff[0] = PRODUCT_NUMBER;
         buff[1] = CMD_Updata_Step;
         buff[2] = c.b[3];
         buff[3] = c.b[2];
         buff[4] = c.b[1];
         buff[5] = c.b[0];
         BLE_Salve_SendData(buff, 6);
         step_save = movement.stepcount;
    }    
}

/*************************************************************************
函数名:countStep
功能:计算步数
输入参数:None
输出参数:None
返回值:None
说明:
连续走十步才会开始计步
连续走了9步以下,停留超过3秒,则计数清空
*************************************************************************/
static void countStep(void) 
{
        timeOfLastPeak_1 = timeOfThisPeak_1;                  //把当前波峰时间赋值给上一次的波峰时间变量
        timeOfThisPeak_1 = StepDetector_tick;                 //把当前时间赋值给当前波峰事件变量

        if (timeOfThisPeak_1 - timeOfLastPeak_1 <= 300L)      //如果当前波峰时间和上一次波峰时间间隔小于3s(300*10ms)
        {
            if(count<9)                                       //如果临时计步小于9，就自加
            {
                count++;
            }
            else if(count == 9)                               //如果临时计步等于9，就把步数加在实际步数上
            {
                count++;
                movement.stepcount += count;
							  movement.distance = (int)((double)(((double)save_param.height/100) *movement.stepcount * (double)DISTANCE_CONST)/100);    //行走的距离(KM) *10
                movement.calorie = (int)(save_param.weight *(double)((double)(((double)save_param.height/100) *movement.stepcount * (double)DISTANCE_CONST)/1000) * (double)CALORIE_CONST);           //热量(kCal * 10)
            }
            else                                             //如果临时步数大于9，实际步数就自加
            {
                movement.stepcount++;
//                NRF_LOG_INFO("stepcount = %d\r\n", stepcount);
							
							  movement.distance = (int)((double)(((double)save_param.height/100) *movement.stepcount * (double)DISTANCE_CONST)/100);    //行走的距离(KM) *10
                movement.calorie = (int)(save_param.weight *(double)((double)(((double)save_param.height/100) *movement.stepcount * (double)DISTANCE_CONST)/1000) * (double)CALORIE_CONST);           //热量(kCal * 10)
                
            }
        }
        else                                                //如果当前波峰时间和上一次波峰时间间隔大于3s(300*10ms)，清除临时步数
        {
            count = 1;                                      //为1,不是0
        }
}


/*************************************************************************
函数名:setSteps
功能:设置步数
输入参数:
@initValue:设置的步数
输出参数:None
返回值:None
说明:
*************************************************************************/
void setSteps(int initValue) 
{
        movement.stepcount = initValue;
        count = 0;
        timeOfLastPeak_1 = 0;
        timeOfThisPeak_1 = 0;      
}

/*************************************************************************
函数名:detectorPeak
功能:检测波峰,记录波谷值
输入参数:
@newValue:新的加速度信息
@oldValue:旧的加速度信息
输出参数:None
返回值:None
说明:
以下四个条件判断为波峰：
1.目前点为下降的趋势：isDirectionUp为false
2.之前的点为上升的趋势：lastStatus为true
3.到波峰为止，持续上升大于等于2次
4.波峰值大于1.2g (11.76),小于2g (19.6)
记录波谷值
1.观察波形图，可以发现在出现步子的地方，波谷的下一个就是波峰，有比较明显的特征以及差值
2.所以要记录每次的波谷值，为了和下次的波峰做对比
*************************************************************************/
static bool detectorPeak(float newValue, float oldValue) 
{
    lastStatus = isDirectionUp;                                         //获取上一点的状态

    if (newValue >= oldValue)                                           //新的加速度信息大于等于旧的加速度信息
    {
        isDirectionUp = true;                                           //标记波形状态在上升
        continueUpCount++;                                              //持续上升次数+1
    }
    else                                                                //新的加速度信息小于旧的加速度信息
    {
        continueUpFormerCount = continueUpCount;                        //保存上一点持续上升次数                   
        continueUpCount = 0;                                            //将持续上升次数清0
        isDirectionUp = false;                                          //标记波形没有上升
    }

    /* 如果波形在上升，且当前状态还在上升，持续上升次数大于2，上一次加速度信息在1.2G~2G之间 */
    if (!isDirectionUp && lastStatus && (continueUpFormerCount >= 2 && (oldValue >= (float)11.76 && oldValue < (float)19.6))) 
    {
        peakOfWave = oldValue;                                          //把上一次加速度值赋值给波峰值
        return true;
    } 
    else if (!lastStatus && isDirectionUp)                              //如果上一点在下降，这一次在上升
    {
        valleyOfWave = oldValue;                                        //把上一次加速度值赋值给波谷值                  
        return false;
    } 
    else 
    {
        return false;
    }
}

/*************************************************************************
函数名:averageValue
功能:通过梯度法计算新的阈值
输入参数:
@value[]:保存的波峰与波谷之差
@n:缓存空间大小
输出参数:None
返回值:计算的新的阈值
说明:
*************************************************************************/
static float averageValue(float value[], int n) 
{
    float ave = 0;

    /* 计算波峰与波谷之差的平均值 */
    for (int i = 0; i < n; i++) 
    {
        ave += value[i];
    }
    ave = ave / ValueNum;

    /* 根据梯度返回新的阈值 */
    if (ave >= 8)
        ave = (float)(4.3);
    else if (ave >= 7 && ave < 8)
        ave = (float)(3.3);
    else if (ave >= 4 && ave < 7)
        ave = (float)(2.3);
    else if (ave >= 3 && ave < 4)
        ave = (float)(2.0);
    else 
        ave = (float)(1.3);
        
		return ave;
}


/*************************************************************************
函数名:peakValleyThread
功能:阈值的计算
输入参数:
@value:波峰与波谷之差
输出参数:None
返回值:计算的新的阈值
说明:
阈值的计算
1.通过波峰波谷的差值计算阈值
2.记录4个值，存入tempValue[]数组中
3.在将数组传入函数averageValue中计算阈值
*************************************************************************/
static float peakValleyThread(float value) 
{
    float tempThread = ThreadValue;                                  //定义一个临时阈值，将当前初始阈值给他
    
    if (tempCount < ValueNum)                                        //如果获取的波峰与波谷之差数量不够
    {
        tempValue[tempCount] = value;                                //将此次波峰与波谷之差，继续填入缓存
        tempCount++;
    }
    else                                                             //如果获取的波峰与波谷之差数量够了
    {
        tempThread = averageValue(tempValue, ValueNum);              //计算新的初始阈值

        /* 将波峰与波谷之差缓存空间左移，填入新值，以便重新计算新的初始阈值 */
        for (int i = 1; i < ValueNum; i++)
        {
            tempValue[i - 1] = tempValue[i];
        }
        tempValue[ValueNum - 1] = value;
    }
    return tempThread;                                               //返回计算的新的阈值

}

/*************************************************************************
函数名:detectorNewStep
功能:检测步子，并开始计步
输入参数:
@values:加速度信息
输出参数:None
返回值:None
说明:
1.传入sersor中的数据
2.如果检测到了波峰，并且符合时间差以及阈值的条件，则判定为1步
3.符合时间差条件，波峰波谷差值大于initialValue，则将该差值纳入阈值的计算中
*************************************************************************/
static void detectorNewStep(float values) 
{
    if (gravityOld == 0)                                                            //如果上一次传感器值为0，就把此次的值赋值给上一次传感器值保存变量
    {
         gravityOld = values;
    } 
    else 
    {
        if (detectorPeak(values, gravityOld))                                      //如果检测到波峰 
        {
            timeOfLastPeak = timeOfThisPeak;                                       //当前波峰值赋值给上一次波峰值
            timeOfNow = StepDetector_tick;                                         //获取当前时间                         
            if (timeOfNow - timeOfLastPeak >= TimeInterval && (peakOfWave - valleyOfWave >= ThreadValue))  //此次波峰与上一次波峰间隔时间大于预设值且，波峰与波谷之差大于初始阈值
            {
                timeOfThisPeak = timeOfNow;                                        //保存当前时间为当前波峰时间
                countStep();                                                       //计算步数                                                 
            }
            if (timeOfNow - timeOfLastPeak >= TimeInterval && (peakOfWave - valleyOfWave >= InitialValue)) //此次波峰与上一次波峰间隔时间大于预设值且，波峰与波谷之差大于动态阈值
            {
                    timeOfThisPeak = timeOfNow;                                   //保存当前时间为当前波峰时间
                    ThreadValue = peakValleyThread(peakOfWave - valleyOfWave);    //重新获取初始阈值
            }
       }
    }
    gravityOld = values;                                                          //把此次的值赋值给上一次传感器值保存变量                           
}

/*************************************************************************
函数名:get_Gsenson_Value
功能:获取传感器值
输入参数:None
输出参数:None
返回值:None
说明:
*************************************************************************/
void get_Gsenson_Value(void)
{
     AxesRaw_t dataXYZ;

     if(LIS3DH_GetAccAxesRaw(&dataXYZ) == MEMS_SUCCESS)                           //采集传感器x/y/z加速度值
     {
         oriValues[0]= ((float)dataXYZ.AXIS_X/GSENSOR_RESOLUTION)*GRAVITY_EARTH;         //获取X轴相对中立加速的的加速度值(单位:G) 
			   oriValues[1]= ((float)dataXYZ.AXIS_Y/GSENSOR_RESOLUTION)*GRAVITY_EARTH;         //获取Y轴相对中立加速的的加速度值(单位:G)
			   oriValues[2]= ((float)dataXYZ.AXIS_Z/GSENSOR_RESOLUTION)*GRAVITY_EARTH;         //获取Z轴相对中立加速的的加速度值(单位:G)

         gravityNew = (float) sqrt(oriValues[0] * oriValues[0] + oriValues[1] * oriValues[1] + oriValues[2] * oriValues[2]);

         detectorNewStep(gravityNew);                                              //检测步子 
     }
}


