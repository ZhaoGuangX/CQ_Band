#ifndef __OLED_H__
#define __OLED_H__

#include "boards.h"

#define Max_Column	64
#define Max_Row		48

typedef enum{
    OLED_CMD  = 0,
    OLED_DATA = 1,
}OLED_Operation_t;



#define     SCLK_Set()        nrf_gpio_pin_set(OLED_D0)
#define     SCLK_Reset()      nrf_gpio_pin_clear(OLED_D0)

#define     SDIN_Set()        nrf_gpio_pin_set(OLED_D1)
#define     SDIN_Reset()      nrf_gpio_pin_clear(OLED_D1)

#define     RES_Set()         nrf_gpio_pin_set(OLED_RES)
#define     RES_Reset()       nrf_gpio_pin_clear(OLED_RES)

#define     DC_Set()          nrf_gpio_pin_set(OLED_DC)
#define     DC_Reset()        nrf_gpio_pin_clear(OLED_DC)

#define     CS_Set()          nrf_gpio_pin_set(OLED_CS)
#define     CS_Reset()        nrf_gpio_pin_clear(OLED_CS)

/**********************************************************************
函数名:OLED_Refresh_Gram
功能:刷新OLED的显示
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void OLED_Refresh_Gram(void);

/**********************************************************************
函数名:OLED_Display_On
功能:开启OLED显示
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void OLED_Display_On(void);

/**********************************************************************
函数名:OLED_Display_Off
功能:关闭OLED显示
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/    
void OLED_Display_Off(void);

/**********************************************************************
函数名:OLED_Clear
功能:清除显示
输入参数:None
输出参数:None
返回值:None
说明:
清屏函数,清完屏,整个屏幕是黑色的!
***********************************************************************/
void OLED_Clear(void);

/**********************************************************************
函数名:OLED_Set_Pos
功能:操作一个点
输入参数:
@x:显示的X轴
@y:显示的Y轴
输出参数:None
返回值:None
说明:
***********************************************************************/
static void OLED_Set_Pos(uint8_t x,uint8_t y);

/**********************************************************************
函数名:OLED_DrawLine
功能:显示一个点
输入参数:
@x1:显示的X轴起点
@y1:显示的Y轴起点
@x2:显示的X轴终点
@y2:显示的Y轴终点
输出参数:None
返回值:None
说明:
***********************************************************************/
void OLED_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/**********************************************************************
函数名:OLED_ShowChar
功能:显示一个字符
输入参数:
@x:显示的X轴
@y:显示的Y轴
@chr:字符
@size:显示字体大小16/12 
@mode:0,反白显示;1,正常显示
输出参数:None
返回值:None
说明:
***********************************************************************/
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode);

/**********************************************************************
函数名:OLED_ShowNum
功能:显示数字
输入参数:
@x:显示的X轴
@y:显示的Y轴
@num:数字
@len:数字长度
@size:字体大小
输出参数:None
返回值:None
说明:
num:数值(0~4294967295);	 
***********************************************************************/
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size);

/**********************************************************************
函数名:OLED_ShowString
功能:显示字符串
输入参数:
@x:显示的X轴
@y:显示的Y轴
@p:字符串
@len:数字长度
@size:字体大小
@mode:0,反白显示;1,正常显示
输出参数:None
返回值:None
说明: 
***********************************************************************/
#define MAX_CHAR_POSX 64
#define MAX_CHAR_POSY 48 
void OLED_ShowString(uint8_t x, uint8_t y, const char *p, uint8_t size, uint8_t mode);

/**********************************************************************
函数名:OLED_ShowCHinese
功能:显示汉字
输入参数:
@x:显示的X轴
@y:显示的Y轴
@no:字符串
@size:字体大小
@mode:0,反白显示;1,正常显示
输出参数:None
返回值:None
说明: 
***********************************************************************/
void OLED_ShowCHinese(uint8_t x, uint8_t y, uint8_t no, uint8_t size, uint8_t mode);

/**********************************************************************
函数名:OLED_DrawBMP
功能:显示显示BMP图片
输入参数:
@x:放置图片的起始点x坐标
@y:放置图片的起始点y坐标
@x_1:图片的截至坐标x轴
@y_1:图片的截至坐标y轴
@BMP:图片数据
@dataLen:图片数据长度
@mode:显示模式
输出参数:None
返回值:None
说明: 
填写x,y，x_1,y_1时必须填准确，不然图片显示容易出问题
***********************************************************************/
void OLED_DrawBMP(uint8_t x, uint8_t y,uint8_t x_1, uint8_t y_1, const unsigned char *BMP, uint16_t dataLen, uint8_t mode);

/**********************************************************************
函数名:OLED_Init
功能:初始化OLED
输入参数:None
输出参数:None
返回值:None
说明: 
***********************************************************************/
void OLED_Init(void);

















#endif

