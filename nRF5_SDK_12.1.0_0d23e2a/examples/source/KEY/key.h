#ifndef __KEY_H__
#define __KEY_H__

#include "boards.h"

//#define KEY_DEBUG

/**********************************************************************
函数名：bsp_button_event_handler
功能:按键事件回调函数
输入参数:
    pin_no:按键IO
    button_action:按键活动
输出参数:None
返回值：None
说明：
    按键活动的响应，结合定时器，可以区分长按还是短按。
***********************************************************************/
void buttons_init(void);


#endif

