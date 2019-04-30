/*******************************************************************************************
*          版权所属, C ,2018- ,成都传奇兄弟信息技术有限公司
*
*作者:嵌入式开发部              日期:2018-9-1            版本:V0.0
*说明:
    引脚分配
*修改:
*******************************************************************************************/
#ifndef __HART_CONFIG__
#define __HART_CONFIG__

#ifdef __cplusplus
extern "C" {
#endif

//IO分配
#define RX_PIN_NUMBER   8                           /*< 串口RX脚        */
#define TX_PIN_NUMBER   6                           /*< 串口TX脚        */
#define CTS_PIN_NUMBER  7                           /*< 串口CTS脚       */
#define RTS_PIN_NUMBER  5                           /*< 串口RTS脚       */

#define BATT_ADC        2                           /*< 电池ADC采集引脚 */

#define KEY_PIN         12                          /*< 按键引脚        */

#define OLED_D0         23                          /*< OLED_D0引脚     */
#define OLED_D1         22                          /*< OLED_D1引脚     */
#define OLED_RES        26                          /*< OLED_RES引脚    */
#define OLED_DC         24                          /*< OLED_DC引脚     */
#define OLED_CS         30                          /*< OLED_CS引脚     */

#define SPI_SS_PIN      29                          /*< SPI_SS引脚      */
#define SPI_MOSI_PIN    4                           /*< SPI_MOSI引脚    */
#define SPI_MISO_PIN    28                          /*< SPI_MISO引脚    */
#define SPI_SCK_PIN     3                           /*< SPI_SCK引脚     */

#define CHARGER_STATUS  11                          /*< 充电状态脚      */
#define MOTOR_DRIVE     25                          /*< 电机驱动脚      */
#define SENSOR_POWER_EN 13                          /*< 传感器电源使能  */

#define PN532_IIC_SDA         14
#define PN532_IIC_SCL         15


//定时器设置
#define APP_TIMER_PRESCALER         0               //RTC预设值
#define APP_TIMER_MAX_TIMERS        10              //定时器计数截至值
#define APP_TIMER_OP_QUEUE_SIZE     15              //定时器队列大小



//软件配置低频时钟
#define NRF_CLOCK_LFCLKSRC      {.source        = NRF_CLOCK_LF_SRC_XTAL,            \
                                 .rc_ctiv       = 0,                                \
                                 .rc_temp_ctiv  = 0,                                \
                                 .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM}

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
	
typedef signed char    s8;
typedef signed short   s16;
typedef signed int     s32;

typedef enum{ENABLE = 1, DISABLE = !ENABLE}Enable_e;

#define SET_FLAG(flag, bit)        (flag |= (1 << bit))
#define READ_FLAG(flag, bit)       ((flag >>bit) & 1)
#define CLEAN_FLAG(flag, bit)      (flag &= ~(1 << bit))

#ifdef __cplusplus
}
#endif

#endif


