/*******************************************************************************************
*          版权所属, C ,2018- ,成都传奇兄弟信息技术有限公司
*
*作者:嵌入式开发部              日期:2018-9-1            版本:V0.0
*说明:
*    UTC时钟相关功能头文件，定义一些宏和需要的数据结构
*修改:
*
*******************************************************************************************/
#ifndef __UTC_H__
#define __UTC_H__

#include <stdint.h>

/* 一天多少秒 */
#define	SECOND_PER_DAY   86400UL  // 24 hours * 60 minutes * 60 seconds

/* 计算闰年 */
#define	IsLeapYear(yr)	(!((yr) % 400) || (((yr) % 100) && !((yr) % 4)))
#define	YearLength(yr)	(IsLeapYear(yr) ? 366 : 365)

/* 时间基础，从1970/1/1 0：0：0开始计数 */
#define	BEGYEAR	        1970     // UTC started at 00:00:00 January 1, 1970   max year 2100

/* 时间类型 */
typedef uint32_t UTCTime;

/* 星期 */
typedef enum{
    week_Monday = 0,    //星期一
    week_Tuesday,
    week_Wednesday,
    week_Thursday,
    week_Friday,
    week_Saturday,
    week_Sunday        //星期日
}week_day_t;

/* 时间 */
typedef struct{
    uint16_t    year;    // 1970+
    uint8_t     month;   // 0-11
    uint8_t     day;     // 0-30
    uint8_t     weekday; // 0:Monday
    uint8_t     hour;    // 0-23
    uint8_t     minute;  // 0-59
    uint8_t     second;  // 0-59
}UTCTime_t;

/******************************************************************************
Function   : Time_Run
Description: 时钟运行
Input      : Null
Output     : Null
Return     : 
Others     : 时钟以1s计，运行一个1s的定时器 
*******************************************************************************/
void Time_Run(void);

/******************************************************************************
Function   : SetClock_seconds
Description: 以秒设置时间
Input      : newTime:用秒记的新时间
Output     : Null
Return     : 
Others     : 
*******************************************************************************/
void SetClock_seconds( UTCTime newTime );

/******************************************************************************
Function   : SetClock_UTC
Description: 以时间格式设置时间
Input      : tm:时间格式的新时间
Output     : Null
Return     : 
Others     : 
*******************************************************************************/
void SetClock_UTC(UTCTime_t *tm);

/******************************************************************************
Function   : GetClock_seconds
Description: 以秒获取时间
Input      : Null
Output     : Null
Return     : 获取到的时间 
Others     : 
*******************************************************************************/
UTCTime GetClock_seconds(void );

/******************************************************************************
Function   : SetClock_UTC
Description: 以时间格式获取时间
Input      : tm:时间格式的新时间
Output     : Null
Return     : 
Others     : 
*******************************************************************************/
void GetClock_UTC(UTCTime_t *tm);

/******************************************************************************
Function   : monthLength
Description: 根据年月获取天数
Input      : lpyr:一年的天数
             mon:月
Output     : Null
Return     : 转换后的天数
Others     : 
*******************************************************************************/
void seconds_To_UTC( UTCTime_t *tm, UTCTime secTime );

/******************************************************************************
Function   : seconds_To_UTC
Description: 秒转换成时间格式
Input      : secTime:时间秒
Output     : tm：返回之间格式
Return     : 
Others     : 
*******************************************************************************/
UTCTime UTC_To_seconds( UTCTime_t *tm );

#endif

