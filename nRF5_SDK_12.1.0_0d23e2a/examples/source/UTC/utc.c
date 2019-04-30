/*******************************************************************************************
*          版权所属, C ,2018- ,成都传奇兄弟信息技术有限公司
*
*作者:嵌入式开发部              日期:2018-9-1            版本:V0.0
*说明:
*    UTC时钟相关功能函数
*修改:
*
*******************************************************************************************/
#include "utc.h"

static UTCTime base_time = 0;     //时钟基数

/******************************************************************************
Function   : Time_Run
Description: 时钟运行
Input      : Null
Output     : Null
Return     : 
Others     : 时钟以1s计，运行一个1s的定时器 
*******************************************************************************/
void Time_Run(void)
{
    base_time++;    
}

/******************************************************************************
Function   : SetClock_seconds
Description: 以秒设置时间
Input      : newTime:用秒记的新时间
Output     : Null
Return     : 
Others     : 
*******************************************************************************/
void SetClock_seconds( UTCTime newTime )
{
    base_time = newTime;
}

/******************************************************************************
Function   : SetClock_UTC
Description: 以时间格式设置时间
Input      : tm:时间格式的新时间
Output     : Null
Return     : 
Others     : 
*******************************************************************************/
void SetClock_UTC(UTCTime_t *tm)
{
    base_time = UTC_To_seconds(tm);   
}

/******************************************************************************
Function   : GetClock_seconds
Description: 以秒获取时间
Input      : Null
Output     : Null
Return     : 获取到的时间 
Others     : 
*******************************************************************************/
UTCTime GetClock_seconds(void )
{
     return base_time;
}

/******************************************************************************
Function   : SetClock_UTC
Description: 以时间格式获取时间
Input      : tm:时间格式的新时间
Output     : Null
Return     : 
Others     : 
*******************************************************************************/
void GetClock_UTC(UTCTime_t *tm)
{
    seconds_To_UTC(tm, base_time);   
}

/******************************************************************************
Function   : monthLength
Description: 根据年月获取天数
Input      : lpyr:一年的天数
             mon:月
Output     : Null
Return     : 转换后的天数
Others     : 
*******************************************************************************/
static uint8_t monthLength( uint8_t lpyr, uint8_t mon )
{
    uint8_t days = 31;

    if ( mon == 1 )     //如果是二月
        days = ( 28 + lpyr );
    else
    {
        if ( mon > 6 )   // 8月--12月
            mon--;

        if ( mon & 1 )  //奇数月
            days = 30;
    }
    return ( days );
}

/******************************************************************************
Function   : seconds_To_UTC
Description: 秒转换成时间格式
Input      : secTime:时间秒
Output     : tm：返回之间格式
Return     : 
Others     : 
*******************************************************************************/
void seconds_To_UTC( UTCTime_t *tm, UTCTime secTime )
{
    uint32_t day = secTime % SECOND_PER_DAY;
    
    /* 获取时分秒 */
    tm->second = day % 60UL;
    tm->minute = (day % 3600UL) / 60UL;
    tm->hour = (day / 3600UL);
  
    uint16_t numDays = secTime / SECOND_PER_DAY;
    
    /* 获取周 */
    tm->weekday = (numDays + week_Thursday) % 7;    // 1970-1-1 is Thursday
    
    /* 获取年 */
    tm->year = BEGYEAR;
    while ( numDays >= YearLength( tm->year ) )
    {
      numDays -= YearLength( tm->year );
      tm->year++;
    }

    /* 获取月 */
    tm->month = 0;
    while ( numDays >= monthLength( IsLeapYear( tm->year ), tm->month ) )
    {
      numDays -= monthLength( IsLeapYear( tm->year ), tm->month );
      tm->month++;
    }

    /* 获取天 */
    tm->day = numDays;
}

/******************************************************************************
Function   : UTC_To_seconds
Description: 时间格式转换成秒
Input      : Null
Output     : tm：时间格式的时间
Return     : 
Others     : 
*******************************************************************************/
UTCTime UTC_To_seconds( UTCTime_t *tm )
{
    uint32_t second;

    /* 获取一天已经过了多少秒 */
    second = (((tm->hour * 60UL) + tm->minute) * 60UL) + tm->second;

    uint16_t days = tm->day;
    int8_t month = tm->month;
     
    /* 计算天 */
    while ( --month >= 0 )
    {
         days += monthLength( IsLeapYear( tm->year ), month );
    }
    
    uint16_t year = tm->year;
    while ( --year >= BEGYEAR )
    {
        days += YearLength( year );
    }
    
    /* 获取年、月、日计算的秒 */
    second += (days * SECOND_PER_DAY);
	 
    return ( second );
}

