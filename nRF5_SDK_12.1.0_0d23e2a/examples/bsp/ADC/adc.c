/*******************************************************************************************
*          版权所属, C ,2018- ,成都传奇兄弟信息技术有限公司
*
*作者:嵌入式开发部              日期:2018-9-1            版本:V0.0
*说明:
*    ADC采集电池电压。
*修改:
2019-2-27:
1、修改ADC的电压算法，怎加电池等级程序。  
*******************************************************************************************/
#include <stdio.h>
#include <string.h>
#include "adc.h"
#include "nrf_drv_saadc.h"
#include "nrf_saadc.h"
#include "boards.h"
#include "app_gpio.h"
#include "timer.h"
#include "ble_salve.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#ifdef ADC_DEBUG
 #define adc_Msg    NRF_LOG_INFO
#else
 #define adc_Msg(...)
#endif

/* 配置ADC通道 */
#define SAMPLES_IN_BUFFER        1                         //一个采集事件
nrf_saadc_value_t    adc_buffer[2][SAMPLES_IN_BUFFER];     //双缓存      
static uint8_t cache_num = 0;
static uint16_t adc_buff[10] = {0};    //缓存ADC采集的值
static uint8_t saadc_flag = 0;   //bit0:SAADC初始化标记   0:未初始化   1:已初始化
                                 //bit1: ADC被占用        0:未占用     1:占用
/**********************************************************************
函数名:ADC_Data_Dispose
功能:ADC数据处理
输入参数:None
输出参数:None
返回值:None
说明:
电压评级：
4级：4.0V-4.2V
3级：3.8V-4.0V
2级：3.6V-3.8V
1级：3.5V-3.6V
0级：3.4V-3.5V
3.4V以下为低电量，关闭大部分功能。
***********************************************************************/
void ADC_Data_Dispose(void)
{
	  uint8_t i = 0,j = 0;
	  uint16_t temp = 0;
	  uint32_t Sum = 0;
	  uint16_t mean = 0;    //均值
	
	  /* 对数据进行排序 */
    for( i = 0; i < sizeof(adc_buff)/sizeof(adc_buff[0]); i++ )
	  {
		    for( j = 0; j < sizeof(adc_buff)/sizeof(adc_buff[0]) - 1 - i; j++ )
			  {
				    if( adc_buff[j] > adc_buff[j+1] )
						{
						    temp = adc_buff[j+1];
							  adc_buff[j+1] = adc_buff[j];
							  adc_buff[j] = temp;
						}							
				}
		}
   
		/* 去掉最大值和最小值求平均值 */
    for( i = 1; i < sizeof(adc_buff)/sizeof(adc_buff[0]) - 1; i++ )
		    Sum += adc_buff[i];
		
		mean = Sum/(sizeof(adc_buff)/sizeof(adc_buff[0]) - 2);
//		adc_Msg("mean = %d\r\n",mean);
		
    /* 计算电压 */
    sys_state.batty_Voltage = 0.1935 * mean;
		adc_Msg("voltage = %d.%02d\r\n",sys_state.batty_Voltage/100,sys_state.batty_Voltage%100);
//		char buff[10] = {0};
//		sprintf( buff,"V=%d.%02d", sys_state.batty_Voltage/100,sys_state.batty_Voltage%100);
//    BLE_Salve_SendData((uint8_t *)buff,strlen(buff));
		/* 充电时只获取电压值，不获取电量 */
		/* 输出电量等级 */	
		if(CHARGER_SATE != 0)
		{
		    if( sys_state.batty_Voltage > 400 )
		    {
		        if( sys_state.batty_grabe <4 )     //如果当前电池等级小于4,现在检测的电池等级应该是4级,就必须满足此次检测的电压值大于等级电压0.1V
				    {
				        if( sys_state.batty_Voltage > 405 ) 
                   sys_state.batty_grabe = 4;							
				    } 
            else	
                sys_state.batty_grabe = 4;					
		    }
		   
        else if( sys_state.batty_Voltage > 380 )
		    {
		        if( sys_state.batty_grabe <3 )     //如果当前电池等级小于3,现在检测的电池等级应该是3级,就必须满足此次检测的电压值大于等级电压0.1V
				    {
				    	  if( sys_state.batty_Voltage > 385 ) 
                   sys_state.batty_grabe = 3;							
				    }
				    else
				        sys_state.batty_grabe = 3;
		   }

		   else if( sys_state.batty_Voltage > 360 )
		   {
		        if( sys_state.batty_grabe <2 )     //如果当前电池等级小于2,现在检测的电池等级应该是2级,就必须满足此次检测的电压值大于等级电压0.1V
				    {
				    	 if( sys_state.batty_Voltage > 365 ) 
                   sys_state.batty_grabe = 2;							
				    }
				    else
				       sys_state.batty_grabe = 2;
		   }
		
		  else if( sys_state.batty_Voltage > 350 )
		  {
		       if( sys_state.batty_grabe <1 )     //如果当前电池等级小于1,现在检测的电池等级应该是1级,就必须满足此次检测的电压值大于等级电压0.05V
				   {
				    	 if( sys_state.batty_Voltage > 355 ) 
                   sys_state.batty_grabe = 1;							
				   }
				   else
				       sys_state.batty_grabe = 1;
		  }

		  else
		      sys_state.batty_grabe = 0;   
	  }
		
    /* 判断电量是否过低(<3.4V),如果电量过低将只保留时间运行和ADC功能 */
    if( sys_state.batty_Voltage < 340 )		
		{
		    sys_state.power_low = 1;      //电量低
		}
		else 
		{
		    if(  sys_state.power_low == 1 )   //如果开始是低电量状态,当电压必须大于3.45V时才解除低电压状态
        {
				    if( sys_state.batty_Voltage > 345 )
                sys_state.power_low = 0;							
				}					
		}
    	
    /* 清除数据缓存 */		
		memset(adc_buff , 0, sizeof(adc_buff));
		cache_num = 0;
	  saadc_close();
		CLR_BIT( saadc_flag, 1 );
		timers_start_adc_timer();
		
		
}

/**********************************************************************
函数名:saadc_callback
功能:ADC回调函数
输入参数:
@p_event:事件指针
输出参数:None
返回值:None
说明:
***********************************************************************/
void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
    if(p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
        ret_code_t err_code;
        err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer,SAMPLES_IN_BUFFER);
			  APP_ERROR_CHECK(err_code);
			
//			  adc_Msg("ADC Value:%d\r\n",p_event->data.done.p_buffer[0]);
			  adc_buff[cache_num++] = p_event->data.done.p_buffer[0];    //填充缓存
					  
			  /* 对采集的电压进行处理 */
			  if( cache_num == sizeof(adc_buff)/sizeof(adc_buff[0]) )    //缓存区填满，处理数据
				{
				    ADC_Data_Dispose();

				}
				else
				{
				    APP_ERROR_CHECK(nrf_drv_saadc_sample());    //缓存区未填满,继续转换
				}
		
    }
}

/**********************************************************************
函数名:Change_Chatger_State
功能:电池充电状态的改变
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void Change_Chatger_State(void)
{
    /***************************************************** 充电状态切换 **************************************************/
    static uint8_t chatger_state_lock = 0;     //bit0:充电状态锁,0:未锁定，充电时,电池图标动态填充。  1:锁定，充电时，电池图标不懂。
		                                           //bit1:充电状态确定  0:此前是为充电    1：此前在充电
		                                           //bit2:由于充电快满时，充电状态一直变化，此标记用于充电状态演示判断。判断充电是否完成，需要延时判断
		static uint8_t debounce_time = 0;          //充电状态脚消抖时间
		//锁定条件:充电状态由充电(CHARGER_SATE = 0),变化为不充电(CHARGER_SATE = 1)，电压>4.1V时锁定。
		//解锁定条件:电压<4.1V时解锁定。
		
		/* 如果当前电压小于4.1V，解锁充电状态锁 */
		if( sys_state.batty_Voltage < 410 )
		    CLR_BIT(chatger_state_lock,0);
    
		/* 如果充电状态引脚显示在充电,且充电状态锁为0,动态显示充电符号 */
		if( (CHARGER_SATE == 0) && (IS_SET(chatger_state_lock,0) == 0))
    {
			  debounce_time = 0;
			
				sys_state.batty_grabe++;
        if( sys_state.batty_grabe > 4 )
            sys_state.batty_grabe = 0;	

        if( IS_SET(chatger_state_lock,1) == 0 )   //此前状态为未充电	    
				{
				     SET_BIT(chatger_state_lock,1);       //标记在充电    
              Motor_Drive();
											   
						 if( OLED_ctrl.oled_state_flag == 0 ) //如果当前屏幕是熄灭的
						 {
								 /* 点亮屏幕显示日期 */
					       OLED_Display_On();
						     OLED_ctrl.oled_off_key = 1;
						     OLED_ctrl.oled_state_flag = 1;
						     OLED_ctrl.time_off = OLED_OFF_TIME;
						     OLED_ctrl.page_num = DATE_P;
						 }					
				}					
		}		

    /* 如果当前充电引脚状态未充电*/
    if( CHARGER_SATE != 0 )
		{
			    if( (IS_SET(chatger_state_lock,1) != 0) && (IS_SET(chatger_state_lock,2) != 0) )   //此前状态为充电，且已判断充电完成延时
					{
						   debounce_time = 0;
					     CLR_BIT(chatger_state_lock,1);       //标记未充电 
						   CLR_BIT(chatger_state_lock,2); 	    //清除判断延时标记
                Motor_Drive();
						
						   if( OLED_ctrl.oled_state_flag == 0 ) //如果当前屏幕是熄灭的
						   {
								    sys_state.batty_grabe = 4;
								    /* 点亮屏幕显示日期 */
					          OLED_Display_On();
						        OLED_ctrl.oled_off_key = 1;
						        OLED_ctrl.oled_state_flag = 1;
						        OLED_ctrl.time_off = OLED_OFF_TIME;
						        OLED_ctrl.page_num = DATE_P;
						   }											
					}	
          
          /* 电压>400V ,表示充电已满 */					
			    if( sys_state.batty_Voltage > 400 )
					{
		          SET_BIT(chatger_state_lock,0);	   //锁定充电状态锁 
					}
				  start_adc_convert();                   //开启ADC转换,获取电压等级	
					
					if( (++debounce_time) > 3 )            //消抖时间3S
					    SET_BIT(chatger_state_lock,2);	   //表示已判断过一次充电满
       				
		}					
}

/**********************************************************************
函数名:saddc_init
功能:初始化ADC
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void saddc_init(void)
{
    /* 配置ADC */
	  nrf_drv_saadc_config_t saadc_config = NRF_DRV_SAADC_DEFAULT_CONFIG;
	  saadc_config.resolution = NRF_SAADC_RESOLUTION_12BIT;    //配置12bitADC
    APP_ERROR_CHECK(nrf_drv_saadc_init(&saadc_config, saadc_callback));
    
	  /* 配置电池ADC通道 */
    nrf_saadc_channel_config_t bat_channel_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(nrf_drv_saadc_gpio_to_ain(BATT_ADC));  //使用默认配置
    bat_channel_config.gain = NRF_SAADC_GAIN1;       //使用内部参考电压0.6V,所以此处要讲输入降低，选择1倍增益
    APP_ERROR_CHECK(nrf_drv_saadc_channel_init(nrf_drv_saadc_gpio_to_ain(BATT_ADC)-1, &bat_channel_config));

	  /* 配置双缓冲机制 */
	  APP_ERROR_CHECK(nrf_drv_saadc_buffer_convert(adc_buffer[0],SAMPLES_IN_BUFFER));
	  APP_ERROR_CHECK(nrf_drv_saadc_buffer_convert(adc_buffer[1],SAMPLES_IN_BUFFER));
}

/**********************************************************************
函数名:saadc_close
功能:关闭ADC功能
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void saadc_close(void)
{
	  if( IS_SET( saadc_flag, 0 ) != 0)
		{
        nrf_drv_saadc_uninit();
			  CLR_BIT( saadc_flag, 0 );
		}
}

/**********************************************************************
函数名:start_adc_convert
功能:开启转换
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void start_adc_convert(void)
{
	  if( IS_SET( saadc_flag, 0 ) == 0)
		{
	      saddc_init();
			  SET_BIT( saadc_flag, 0 );
		}
    
		if( IS_SET( saadc_flag, 1 ) == 0)
		{
		    APP_ERROR_CHECK(nrf_drv_saadc_sample()); 
        SET_BIT( saadc_flag, 1 );			
		}			
}




