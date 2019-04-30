/**************************************************************************
    版权 (C), 2018- ,成都传奇信息技术有限公司

作者:硬件部        
邮箱:polaris.zhao@foxmail.com
版本:V0.0
创建日期:2019-1-14
文件描述:
   心率传感器MAX3010x的驱动程序
修改:
**************************************************************************/
#include "MAX3010x.h"
#include "IIC.h"
#include <stdio.h>
#include <string.h>
#include "nrf_delay.h"
#include "app_gpio.h"

/* 数据缓存 */
static int ir_buff[BUFFER_SIZE];
static int red_buff[BUFFER_SIZE];

/* 传感器数据 */
MAX3010x_Data_t max30102_data = {
    .SpO2 = 0,
	  .Heart = 0,
	  .Temper = 0,
};


/* max30102使用的标记 */
static uint8_t max30102_flag = 0;    
/*
bit0:是否接触皮肤   0:没有    1:接触

*/
/******************************************************************************
Function   : MAX3010x_Bus_Write
Description: 向MAX3010x的寄存器写数据
Input      : Reg_Addr:寄存器地址
             Data:需要写入的数据
Output     : None
Return     : 0:成功
             其他:失败
Others     : 
*******************************************************************************/
static uint8_t MAX3010x_Bus_Write(uint8_t Reg_Addr, uint8_t Data)
{
    IIC_Start();                //启动IIC总线
	   
	   IIC_SendByte(MAX3010x_WR_BASE_ADDR | MAX3010x_WR);     //写指令
	   if( IIC_WaitACK() != 0)     //等待ACK信号
				{
				    goto fail;
				}
				
				IIC_SendByte(Reg_Addr);     //发送寄存器地址
				if( IIC_WaitACK() != 0)     //等待ACK信号
				{
				    goto fail;
				}
				
				IIC_SendByte(Data);         //发送数据
				if( IIC_WaitACK() != 0)     //等待ACK信号
				{
				    goto fail;
				}
				
				IIC_Stop();                 //发送停止信号
				return 0;
	
fail:
	   IIC_Stop();      //发送停止信号
	   NRF_LOG_INFO("MAX3010x_Bus_Write Fail\r\n");
	   return 1;
}

/******************************************************************************
Function   : MAX3010x_Bus_Read
Description: 从MAX3010x的寄存器读数据
Input      : Reg_Addr:寄存器地址
Output     : None
Return     : 0:失败
             其他:读取的数据
Others     : 
*******************************************************************************/
static uint8_t MAX3010x_Bus_Read(uint8_t Reg_Addr)
{
    uint8_t data = 0;
	   
	   IIC_Start();     //启动IIC总线
	   
	   IIC_SendByte(MAX3010x_WR_BASE_ADDR | MAX3010x_WR);     //写指令
	   if( IIC_WaitACK() != 0)     //等待ACK信号
				{
				    goto fail;
				}
				
				IIC_SendByte(Reg_Addr);     //发送寄存器地址
				if( IIC_WaitACK() != 0)     //等待ACK信号
				{
				    goto fail;
				}
	
	   IIC_Start();     //启动IIC总线，开始读数据
				
				IIC_SendByte(MAX3010x_WR_BASE_ADDR | MAX3010x_RD);     //读指令
	   if( IIC_WaitACK() != 0)     //等待ACK信号
				{
				    goto fail;
				}
				
				data = IIC_ReadByte();      //读取1个字节
				IIC_NACK();                 //发送NACK
				
				IIC_Stop();
				return data;

fail:
	   IIC_Stop();      //发送停止信号
     NRF_LOG_INFO("MAX3010x_Bus_Read Fail\r\n");
	   return 0;
}

/******************************************************************************
Function   : MAX3010x_FIFO_Read
Description: 从MAX3010x的FIFO中，读取数据
Input      : None
Output     : iR:红外信号值
             Red:红色信号值
Return     : None
Others     : 
*******************************************************************************/
void MAX3010x_FIFO_Read(int *iR, int *Red)
{
	   uint32_t temp_iR = 0,temp_Red = 0;
	   uint8_t data_temp[6] = {0};
		 uint8_t i = 0;

		 IIC_Start();     //启动IIC总线
	   
	   IIC_SendByte(MAX3010x_WR_BASE_ADDR | MAX3010x_WR);     //写指令
	   if( IIC_WaitACK() != 0)     //等待ACK信号
				{
				    goto fail;
				}
				
				IIC_SendByte(FIFO_DATA_REG);//发送寄存器地址
				if( IIC_WaitACK() != 0)     //等待ACK信号
				{
				    goto fail;
				}
	
	      IIC_Start();     //启动IIC总线，开始读数据
				
				IIC_SendByte(MAX3010x_WR_BASE_ADDR | MAX3010x_RD);     //读指令
	      if( IIC_WaitACK() != 0)     //等待ACK信号
				{			
				    goto fail;
				}
				
				/* 读取数据 */
        for(i = 0; i < sizeof(data_temp)/sizeof(data_temp[0]); i++)
				{
				    data_temp[i] = IIC_ReadByte();
					   if( i < (sizeof(data_temp)/sizeof(data_temp[0]) -1 ))
					       IIC_ACK();
						else
								 IIC_NACK();
				}
        IIC_Stop();
				
				temp_Red = (((uint32_t)((data_temp[0]<<16) + (data_temp[1]<<8) + data_temp[2])) & 0x3FFFFF);
				temp_iR = (((uint32_t)((data_temp[3]<<16) + (data_temp[4]<<8) + data_temp[5])) & 0x3FFFFF);
				
//				NRF_LOG_INFO("iR = %d, Red = %d\r\n",temp_iR,temp_Red);
				
				if(iR != NULL && Red != NULL)
				{
				    *iR = temp_iR ;
					   *Red = temp_Red ;
				}

				return;

fail:
	   IIC_Stop();      //发送停止信号
     NRF_LOG_INFO("MAX3010x_Bus_Read Fail\r\n"); 
}

/******************************************************************************
Function   : MAX3010x_Init
Description: 初始化MAX3010x传感器
Input      : None
Output     : None
Return     : 0:成功
             其他:失败
Others     : 
*******************************************************************************/
int MAX3010x_Init(void)
{
    if( MAX3010x_Bus_Write(INTERRUPT_ENABLE1,0xC0) )      //中断使能设置1
				    return -1;
		if( MAX3010x_Bus_Write(INTERRUPT_ENABLE2,0x02) )      //中断使能设置2
				    return -1;
		if( MAX3010x_Bus_Write(FIFO_WRITE_POINTER,0x00) )     //FIFO_WR_PTR[4:0]    
				    return -1;
		if( MAX3010x_Bus_Write(FIFO_OVERFLOW_COUNTER,0x00) )  //OVF_COUNTER[4:0]
				    return -1;
		if( MAX3010x_Bus_Write(FIFO_READ_POINTER,0x00) )        //FIFO_RD_PTR[4:0]
				    return -1;
		if( MAX3010x_Bus_Write(FIFO_CONFIG,0x0F) )            //sample avg = 1, fifo rollover=false, fifo almost full = 17
				    return -1;
		if( MAX3010x_Bus_Write(MODE_CONFIG,0x03) )            //0x02 for Red only, 0x03 for SpO2 mode, 0x07 multimode LED
				    return -1;
		if( MAX3010x_Bus_Write(SPO2_CONFIG,0x27) )            // SPO2_ADC range = 4096nA, SPO2 sample rate (100 Hz), LED pulseWidth (400uS)
				    return -1;
		if( MAX3010x_Bus_Write(LED1_PULSE_AMPLITUDE,0x24) )   //Choose value for ~ 7mA for LED1
				    return -1;
		if( MAX3010x_Bus_Write(LED2_PULSE_AMPLITUDE,0x24) )   // Choose value for ~ 7mA for LED2
				    return -1;
		if( MAX3010x_Bus_Write(PROXIMITY_LED_PULSE_AMPLITUDE,0x7F) ) // Choose value for ~ 25mA for Pilot LED
				    return -1;
		if( MAX3010x_Bus_Write(DIE_TEMP_CONFIG,0x01) )
				    return -1;
			
//     First_Sampling(ir_buff, red_buff);       //第一次采集样本
		
    return 0;				
				
}

/******************************************************************************
Function   : MAX3010x_Get_ID
Description: 获取MAX3010x的ID
Input      : None
Output     : None
Return     : 0:获取失败
             其他:获取到的ID
Others     : 设备ID由两部分组成，高8位为REVISION_ID，低8位为PART_ID
*******************************************************************************/
uint16_t MAX3010x_Get_ID(void)
{
    union{
				    uint16_t id;
					   uint8_t temp_id[2];
	 }ID;
    
	 ID.temp_id[0] = MAX3010x_Bus_Read(REVISION_ID);            
	 ID.temp_id[1] = MAX3010x_Bus_Read(PART_ID); 
				
	 return ID.id;
}

/******************************************************************************
Function   : MAX3010x_Get_DIETemp
Description: 获取MAX3010x的内部温度
Input      : None
Output     : integer:温度的整数部分
             fraction:温度的小数部分
Return     : None
Others     : 设备ID由两部分组成，高8位为REVISION_ID，低8位为PART_ID
*******************************************************************************/
void MAX3010x_Get_DIETemp(uint8_t *integer, uint8_t *fraction)
{
    uint8_t temp_integer,temp_fraction;
				
	   MAX3010x_Bus_Write(INTERRUPT_STATUS1,0x00);       //清楚全部中断
	   MAX3010x_Bus_Write(INTERRUPT_ENABLE2,0x02);       //使能内部温度中断
	   MAX3010x_Bus_Write(DIE_TEMP_CONFIG,0x01);         //使能内部温度
	
     temp_integer = MAX3010x_Bus_Read(DIE_TEMP_INTEGER);   //读取整数部分
	   temp_fraction = MAX3010x_Bus_Read(DIE_TEMP_FRACTION); //读取小数部分
	   
	   NRF_LOG_INFO("Temper = %d.%d\r\n",temp_integer, temp_fraction);
	   if( integer != NULL && fraction != NULL )
		 {
		     *integer = temp_integer;
			   *fraction = temp_fraction;
		 }
		 
		 if( IS_SET(max30102_flag, 0) != 0 )
		     max30102_data.Temper = (int)(temp_integer*10 + temp_fraction);
		 else
			   max30102_data.Temper = 0;
			 
}

/********************************************************************************************
                       根据采集的数据计算心率血氧
********************************************************************************************/

/******************************************************************************
Function   :Maxim_Sort_Ascend
Description: 升序排列数组
Input      : pn_x:波数据
             n_size:波数据的数量         
Output     : None
Return     : None
Others     : 
*******************************************************************************/
void maxim_sort_ascend(int32_t *pn_x,int32_t n_size) 
{
    int32_t i, j, n_temp;
    
    for (i = 1; i < n_size; i++) 
    {
        n_temp = pn_x[i];
        for (j = i; j > 0 && n_temp < pn_x[j-1]; j--)
            pn_x[j] = pn_x[j-1];
        pn_x[j] = n_temp;
    }
}


/******************************************************************************
Function   :Maxim_Sort_Ascend
Description: 降序排列数组
Input      : pn_x:波数据
             n_size:波数据的数量         
Output     : pn_indx:记录波峰的缓存
Return     : None
Others     : 
*******************************************************************************/
void maxim_sort_indices_descend(int32_t *pn_x, int32_t *pn_indx, int32_t n_size)
{
    int32_t i, j, n_temp;
    
    for (i = 1; i < n_size; i++) 
    {
        n_temp = pn_indx[i];
        for (j = i; j > 0 && pn_x[n_temp] > pn_x[pn_indx[j-1]]; j--)
            pn_indx[j] = pn_indx[j-1];
        pn_indx[j] = n_temp;
    }
}



/******************************************************************************
Function   : Maxim_Peaks_Above_Min_Height
Description: 找出所有超过阀值得波峰
Input      : pn_x:波数据
             n_size:波数据的数量
             n_min_height:波峰阀值
Output     : pn_locs:记录波峰的缓存
             pn_npks:记录波峰的缓存的序号
Return     : None
Others     : 
*******************************************************************************/
void maxim_peaks_above_min_height(int32_t *pn_locs, int32_t *pn_npks, int32_t  *pn_x, int32_t n_size, int32_t n_min_height)
{
    int32_t i = 1, n_width;
    *pn_npks = 0;
    
    while (i < n_size-1){
        if (pn_x[i] > n_min_height && pn_x[i] > pn_x[i-1]){            // find left edge of potential peaks
            n_width = 1;
            while (i+n_width < n_size && pn_x[i] == pn_x[i+n_width])    // find flat peaks
                n_width++;
            if (pn_x[i] > pn_x[i+n_width] && (*pn_npks) < 15 ){                            // find right edge of peaks
                pn_locs[(*pn_npks)++] = i;        
                // for flat peaks, peak location is left edge
                i += n_width+1;
            }
            else
                i += n_width;
        }
        else
            i++;
    }
}


/******************************************************************************
Function   : Maxim_Remove_Close_Peaks
Description: 移除间距小于设定的波峰间距的波峰
Input      : pn_x:波数据
             n_min_distance:最小波峰间距
Output     : pn_locs:记录波峰的缓存
             pn_npks:记录波峰的缓存的序号
Return     : None
Others     : 
*******************************************************************************/
void maxim_remove_close_peaks(int32_t *pn_locs, int32_t *pn_npks, int32_t *pn_x, int32_t n_min_distance)
{
    
    int32_t i, j, n_old_npks, n_dist;
    
    /* Order peaks from large to small */
    maxim_sort_indices_descend( pn_x, pn_locs, *pn_npks );

    for ( i = -1; i < *pn_npks; i++ ){
        n_old_npks = *pn_npks;
        *pn_npks = i+1;
        for ( j = i+1; j < n_old_npks; j++ ){
            n_dist =  pn_locs[j] - ( i == -1 ? -1 : pn_locs[i] ); // lag-zero peak of autocorr is at index -1
            if ( n_dist > n_min_distance || n_dist < -n_min_distance )
                pn_locs[(*pn_npks)++] = pn_locs[j];
        }
    }

    // Resort indices longo ascending order
    maxim_sort_ascend( pn_locs, *pn_npks );
}


/******************************************************************************
Function   : Maxim_Find_Peaks
Description: 查找波峰
Input      : pn_x:波数据
             n_size:波数据的数量
             n_min_height:波峰阀值
             n_min_distance:最小波峰间距
             n_max_num:最大峰数
Output     : pn_locs:记录波峰的缓存
             pn_npks:记录波峰的缓存的序号
Return     : None
Others     : 
*******************************************************************************/
void maxim_find_peaks(int32_t *pn_locs, int32_t *pn_npks, int32_t *pn_x, int32_t n_size, int32_t n_min_height, int32_t n_min_distance, int32_t n_max_num)
{
    maxim_peaks_above_min_height( pn_locs, pn_npks, pn_x, n_size, n_min_height );
    maxim_remove_close_peaks( pn_locs, pn_npks, pn_x, n_min_distance );
    *pn_npks = min( *pn_npks, n_max_num );
}

const uint16_t auw_hamm[31]={ 41,    276,    512,    276,     41 }; //Hamm=  long16(512* hamming(5)');
//uch_spo2_table is computed as  -45.060*ratioAverage* ratioAverage + 30.354 *ratioAverage + 94.845 ;
const uint8_t uch_spo2_table[184]={ 95, 95, 95, 96, 96, 96, 97, 97, 97, 97, 97, 98, 98, 98, 98, 98, 99, 99, 99, 99, 
                            99, 99, 99, 99, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 
                            100, 100, 100, 100, 99, 99, 99, 99, 99, 99, 99, 99, 98, 98, 98, 98, 98, 98, 97, 97, 
                            97, 97, 96, 96, 96, 96, 95, 95, 95, 94, 94, 94, 93, 93, 93, 92, 92, 92, 91, 91, 
                            90, 90, 89, 89, 89, 88, 88, 87, 87, 86, 86, 85, 85, 84, 84, 83, 82, 82, 81, 81, 
                            80, 80, 79, 78, 78, 77, 76, 76, 75, 74, 74, 73, 72, 72, 71, 70, 69, 69, 68, 67, 
                            66, 66, 65, 64, 63, 62, 62, 61, 60, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50, 
                            49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 31, 30, 29, 
                            28, 27, 26, 25, 23, 22, 21, 20, 19, 17, 16, 15, 14, 12, 11, 10, 9, 7, 6, 5, 
                            3, 2, 1 } ;
static  int32_t an_dx[ BUFFER_SIZE-MA4_SIZE]; // delta
static  int32_t an_x[ BUFFER_SIZE]; //ir
static  int32_t an_y[ BUFFER_SIZE]; //red

/******************************************************************************
Function   : Maxim_HeartRate_And_OxygenSaturation
Description: 根据采样值计算心率血氧
Input      : iR_Buffer:红外数据缓存
             iRDataLength:红外数据缓存长度(红外数据和红色数据长度一样)
             Red_Buffer:红色数据缓存
Output     : SpO2:血氧浓度值
             SpO2_valid:血氧浓度是否有效  (1:有效)
             HeartRate:心率值
             HeartRate_valid:心率值是否有效(1:有效)
Return     : None
Others     : 
						心率值=60s/相邻波峰间隔(T)
						血氧通过计算R值查表(uch_spo2_table)获取。R = (RED_ac/RED_red)/(IR_ac/IR_dc)
*******************************************************************************/
void maxim_heart_rate_and_oxygen_saturation(uint32_t *pun_ir_buffer,  int32_t n_ir_buffer_length, uint32_t *pun_red_buffer, int32_t *pn_spo2, int8_t *pch_spo2_valid, 
                              int32_t *pn_heart_rate, int8_t  *pch_hr_valid)
{
    uint32_t un_ir_mean ,un_only_once ;
    int32_t k ,n_i_ratio_count;
    int32_t i, s, m, n_exact_ir_valley_locs_count ,n_middle_idx;
    int32_t n_th1, n_npks,n_c_min;      
    int32_t an_ir_valley_locs[15] ;
    int32_t an_exact_ir_valley_locs[15] ;
    int32_t an_dx_peak_locs[15] ;
    int32_t n_peak_interval_sum;
    
    int32_t n_y_ac, n_x_ac;
    int32_t n_spo2_calc; 
    int32_t n_y_dc_max, n_x_dc_max; 
    int32_t n_y_dc_max_idx, n_x_dc_max_idx; 
    int32_t an_ratio[5],n_ratio_average; 
    int32_t n_nume,  n_denom ;

    /* 计算心率 */
    //删除iR的直流信号    
    un_ir_mean =0; 
    for (k=0 ; k<n_ir_buffer_length ; k++ ) 
	      un_ir_mean += pun_ir_buffer[k] ;
				
    un_ir_mean =un_ir_mean/n_ir_buffer_length ;
    for (k=0 ; k<n_ir_buffer_length ; k++ )  
		    an_x[k] =  pun_ir_buffer[k] - un_ir_mean ; 
    
    //4个数据一组，算移动平均数
    for(k=0; k< BUFFER_SIZE-MA4_SIZE; k++)
		{
        n_denom= ( an_x[k]+an_x[k+1]+ an_x[k+2]+ an_x[k+3]);
        an_x[k]=  n_denom/(int32_t)4; 
    }

    //得到平滑后的iR数据差值
    for( k=0; k<BUFFER_SIZE-MA4_SIZE-1;  k++)
        an_dx[k]= (an_x[k+1]- an_x[k]);

    //2个平滑后的iR的差值数据一组，算移动平均数
    for(k=0; k< BUFFER_SIZE-MA4_SIZE-2; k++){
        an_dx[k] =  ( an_dx[k]+an_dx[k+1])/2 ;
    }
    
    //使用海明窗
    // 翻转波的形式，方便检测波峰波谷
    for ( i=0 ; i<BUFFER_SIZE-HAMMING_SIZE-MA4_SIZE-2 ;i++)
		{
        s= 0;
        for( k=i; k<i+ HAMMING_SIZE ;k++)
			  {
            s -= an_dx[k] *auw_hamm[k-i] ; 
        }
        an_dx[i]= s/ (int32_t)1146; // divide by sum of auw_hamm 
    }

 
    n_th1=0;        // 计算阀值
    for ( k=0 ; k<BUFFER_SIZE-HAMMING_SIZE ;k++){
        n_th1 += ((an_dx[k]>0)? an_dx[k] : ((int32_t)0-an_dx[k])) ;
    }
    n_th1= n_th1/ ( BUFFER_SIZE-HAMMING_SIZE);

    //查找波峰  
    maxim_find_peaks( an_dx_peak_locs, &n_npks, an_dx, BUFFER_SIZE-HAMMING_SIZE, n_th1, 40, 5 );//peak_height, peak_distance, max_num_peaks 

    n_peak_interval_sum =0;
    if (n_npks>=2)      //如果检测到超过两个波峰
		{
        for (k=1; k<n_npks; k++)
            n_peak_interval_sum += (an_dx_peak_locs[k] - an_dx_peak_locs[k -1]);   //所有波峰的时间差之和
        n_peak_interval_sum=n_peak_interval_sum/(n_npks-1);                        //平均两个波峰的时间差

        *pn_heart_rate=(int32_t)((6000 - 1000)/n_peak_interval_sum);                       // beats per minutes，10ms采集一个数据  60*1000/10=6000
        *pch_hr_valid  = 1;
    }
    else               //检测出错
		{
        *pn_heart_rate = -999;
        *pch_hr_valid  = 0;
    }


    /* 计算血氧 */
    for ( k=0 ; k<n_npks ;k++)
        an_ir_valley_locs[k] = an_dx_peak_locs[k] + HAMMING_SIZE/2; 


    //原始值: RED(=y) and IR(=X)
    for (k=0 ; k<n_ir_buffer_length ; k++ )  
		{
        an_x[k] =  pun_ir_buffer[k] ; 
        an_y[k] =  pun_red_buffer[k] ; 
    }

    //在iR波谷附近查找精确的最小值
    n_exact_ir_valley_locs_count =0; 
    for(k=0 ; k<n_npks ;k++)
		{
        un_only_once = 1;
        m = an_ir_valley_locs[k];
        n_c_min = 16777216;          //2^24;
			
        if ( ((m+5) <  BUFFER_SIZE-HAMMING_SIZE)  && ((m-5) > 0) )
				{
            for(i= m-5; i<m+5; i++)
					  {
                if (an_x[i] < n_c_min)
								{
                    if (un_only_once > 0)
										{
                       un_only_once = 0;
                    } 
                    n_c_min= an_x[i] ;                 //保存最小值
                    an_exact_ir_valley_locs[k]= i;     //保存最小值得序号
                }
					  }
            if (un_only_once ==0)
               n_exact_ir_valley_locs_count ++ ;       //查找到的最小值的数量
        }
    }
    if (n_exact_ir_valley_locs_count <2 )              //最小值数量不够
		{
       *pn_spo2 =  -999 ; // do not use SPO2 since signal ratio is out of range
       *pch_spo2_valid  = 0; 
       return;
    }
		
    // 4个数据一组，计算移动平均数
    for(k=0; k< BUFFER_SIZE-MA4_SIZE; k++)
		{
        an_x[k]=( an_x[k]+an_x[k+1]+ an_x[k+2]+ an_x[k+3])/(int32_t)4;
        an_y[k]=( an_y[k]+an_y[k+1]+ an_y[k+2]+ an_y[k+3])/(int32_t)4;
    }

    //using an_exact_ir_valley_locs , find ir-red DC andir-red AC for SPO2 calibration ratio
    //finding AC/DC maximum of raw ir * red between two valley locations
    n_ratio_average =0; 
    n_i_ratio_count =0; 
    
    for(k=0; k< 5; k++) 
		    an_ratio[k]=0;
		
    for (k=0; k< n_exact_ir_valley_locs_count; k++)
		{
        if (an_exact_ir_valley_locs[k] > BUFFER_SIZE )    //获取的序号，超过总序号，返回
				{             
            *pn_spo2 =  -999 ; // do not use SPO2 since valley loc is out of range
            *pch_spo2_valid  = 0; 
            return;
        }
    }
    
		// 寻找所有波谷中的最大值
    // and use ratio betwen AC compoent of Ir & Red and DC compoent of Ir & Red for SPO2 
    for (k=0; k< n_exact_ir_valley_locs_count-1; k++)
		{
        n_y_dc_max= -16777216 ; 
        n_x_dc_max= - 16777216; 
			
        if (an_exact_ir_valley_locs[k+1] - an_exact_ir_valley_locs[k] >10)    //如果后面一个波谷的序号比前一个波谷的序号大10以数据以上
				{
            for ( i= an_exact_ir_valley_locs[k]; i< an_exact_ir_valley_locs[k+1]; i++ )    //在两个波谷间寻找最大的值    
					  {
                if (an_x[i]> n_x_dc_max) 
                {
								    n_x_dc_max =an_x[i];
									  n_x_dc_max_idx =i; 
								}
                if (an_y[i]> n_y_dc_max) 
                {
								    n_y_dc_max =an_y[i];
								    n_y_dc_max_idx=i;
								}
            }
            n_y_ac= ( an_y[an_exact_ir_valley_locs[k+1]] - an_y[an_exact_ir_valley_locs[k] ] )*( n_y_dc_max_idx - an_exact_ir_valley_locs[k] ); //red
            n_y_ac=   an_y[an_exact_ir_valley_locs[k]] + n_y_ac/ (an_exact_ir_valley_locs[k+1] - an_exact_ir_valley_locs[k]);     
            n_y_ac=  an_y[n_y_dc_max_idx] - n_y_ac;      //获取红色光的AC信号
						
            n_x_ac= (an_x[an_exact_ir_valley_locs[k+1]] - an_x[an_exact_ir_valley_locs[k] ] )*(n_x_dc_max_idx -an_exact_ir_valley_locs[k]); // ir
            n_x_ac=  an_x[an_exact_ir_valley_locs[k]] + n_x_ac/ (an_exact_ir_valley_locs[k+1] - an_exact_ir_valley_locs[k]); 
            n_x_ac=  an_x[n_y_dc_max_idx] - n_x_ac;      //获取红外光的AC信号
            
						n_nume= ( n_y_ac *n_x_dc_max) >>7 ;            
            n_denom= ( n_x_ac *n_y_dc_max)>>7;
						
            if (n_denom>0  && n_i_ratio_count <5 &&  n_nume != 0)
            {   
                an_ratio[n_i_ratio_count]= (n_nume*100)/n_denom ; //formular is ( n_y_ac *n_x_dc_max) / ( n_x_ac *n_y_dc_max) ;   计算R值
                n_i_ratio_count++;
            }
        }
    }

    maxim_sort_ascend(an_ratio, n_i_ratio_count);   //将获取的R排序
    n_middle_idx= n_i_ratio_count/2;

    if (n_middle_idx >1)
        n_ratio_average =( an_ratio[n_middle_idx-1] +an_ratio[n_middle_idx])/2; // use median
    else
        n_ratio_average = an_ratio[n_middle_idx ];

    if( n_ratio_average>2 && n_ratio_average <184)
		{
        n_spo2_calc= uch_spo2_table[n_ratio_average] ;    //查表获取血氧值
        *pn_spo2 = n_spo2_calc ;
        *pch_spo2_valid  = 1;       //  float_SPO2 =  -45.060*n_ratio_average* n_ratio_average/10000 + 30.354 *n_ratio_average/100 + 94.845 ;  // for comparison with table
    }
    else
		{
        *pn_spo2 =  -999 ; // do not use SPO2 since signal ratio is out of range
        *pch_spo2_valid  = 0; 
    }
}


/******************************************************************************
Function   : First_Sampling
Description: 第一次采样，获取样本
Input      : None
Output     : ir_databuff:采集的红外数据缓存
             red_databuff:采集的红色数据缓存
Return     : None
Others     : 第一次采样确定信号范围
*******************************************************************************/
void First_Sampling(int * ir_databuff, int *red_databuff)
{
    uint16_t i = 0;
    
    if( ir_databuff == NULL || red_databuff == NULL )
        return;
    
    /* 采样 */
    for(i = 0; i < BUFFER_SIZE; i++)
    {
        MAX3010x_FIFO_Read(&ir_databuff[i], &red_databuff[i]);  //采集数据
        nrf_delay_ms(10);
    }
}

/******************************************************************************
Function   : Get_Heart_SensorData
Description: 单次获取传感器原始数据，放回缓存
Input      : None
Output     : None
Return     : None
Others     : 
           1、每获取一个数据，将缓存空间向前移动一个空间出来，放入新数据。
           2、间隔10ms采集一个数据
*******************************************************************************/
void Get_SensorData(void)
{
    uint16_t i = 0;
     
    /* 缓存空间向前溢出1个数据 */
    for(i = 0; i < BUFFER_SIZE-1; i++)
    {
        ir_buff[i] = ir_buff[i+1];
        red_buff[i] = red_buff[i+1];
    }

    /* 重新采样1个值 */
    MAX3010x_FIFO_Read(&ir_buff[BUFFER_SIZE-1], &red_buff[BUFFER_SIZE-1]);  //采集数据
		
		 uint32_t red_sum = 0,ir_sum = 0;
	
	   for( int i = 0; i < BUFFER_SIZE; i++ )
	   {
		     red_sum += red_buff[i];
			   ir_sum += ir_buff[i];
		 }
		 
		 red_sum = red_sum/BUFFER_SIZE;
		 ir_sum = ir_sum/BUFFER_SIZE;
		 
		 if( ir_sum < 100000 || red_sum < 100000 )
		     CLR_BIT(max30102_flag, 0);    //标记未接触皮肤 
		 else
		     SET_BIT(max30102_flag, 0);    //标记已接触皮肤
  
}


/******************************************************************************
Function   : Get_Heart_Oxygen
Description: 获取心率血氧
Input      : reset:复位数据   0:不复位数据   1:复位数据
Output     : None
Return     : None
Others     : 该函数1S调用一次
*******************************************************************************/
#define BUFF_NUM_MAX    6

/* 定义是否是有缓存算法 */
//#define SPO2_USER_CACHE
#define HEART_USER_CACHE
void Get_Heart_Oxygen(uint8_t reset)
{
     int8_t SpO2_valid = 0, Heart_valid = 0;
     int SpO2 , Heart ;
	   static uint8_t collect_num = 0;               //如果连续3次采集都是无效就讲显示数据为0
#ifdef SPO2_USER_CACHE
     static int SpO2_buff[BUFF_NUM_MAX] = {0};     //保存历史血氧值	
	   static int SpO2_num = 0;
	   static uint16_t SpO2_Sum = 0;
#endif
	
	
	   static int Heart_buff[BUFF_NUM_MAX] = {0};    //保存历史心率值
		 static int Heart_num = 0;
		 static uint16_t Heart_Sum = 0;
#if defined SPO2_USER_CACHE || defined  HEART_USER_CACHE 
		 int8_t i = 0;
#endif
		 
		 if( reset == 1 )
		 {
#ifdef SPO2_USER_CACHE
          memset( SpO2_buff, 0,  BUFF_NUM_MAX );		
          SpO2_num = 0;			 
			    SpO2_Sum = 0;
#endif

#ifdef HEART_USER_CACHE			 
					memset( Heart_buff, 0,  BUFF_NUM_MAX );
					Heart_num = 0;
					Heart_Sum = 0;
#endif
		 }

	
	   if( IS_SET(max30102_flag, 0) != 0 )
		 {
			   collect_num = 0;
			 
         /* 计算血氧浓度和心率 */
         maxim_heart_rate_and_oxygen_saturation((uint32_t *)ir_buff, BUFFER_SIZE, (uint32_t *)red_buff, &SpO2, &SpO2_valid, &Heart, &Heart_valid); 

#ifdef 	SPO2_USER_CACHE		 
			   /* 将采集的血氧浓度写入缓存 */
	       if( SpO2_valid != 0 )    
				 {
				     if( SpO2_num < BUFF_NUM_MAX && ( SpO2 > 90 ))          //如果缓存空间未填满
						    SpO2_buff[SpO2_num++] = SpO2;
						 else                                                   //如果缓存空间已满就向前移动数据
						 {
						     for( i = 0; i < BUFF_NUM_MAX -1; i++ )
                     SpO2_buff[i] = SpO2_buff[i+1];

                 SpO2_buff[BUFF_NUM_MAX -1] = SpO2;									 
						 }
				 }
	       
				 /* 计算有效的血氧浓度值 */
				 if( SpO2_num == BUFF_NUM_MAX )     //如果血氧缓存空间已满
				 {
             //算平均值
					   SpO2_Sum = 0;
             for( i = 0 ; i < BUFF_NUM_MAX ; i++ )
								 SpO2_Sum += SpO2_buff[i];
					  
					   max30102_data.SpO2 = SpO2_Sum/(BUFF_NUM_MAX);
				 }
#else
         if( SpO2_valid != 0 && SpO2 > 95 )     //血氧浓度必须大于90%才有效，有效后直接赋值
				      max30102_data.SpO2 = SpO2;  
#endif

#ifdef HEART_USER_CACHE				 
				 /* 将心率有效值写入缓存 */
				 static uint16_t last_heart = 120;         //记录上一次的心率值
		     if( Heart_valid != 0 && (Heart < 120))    //数据有效
         {
					 if( ((last_heart + 15) > Heart) &&  ( Heart > 40 ) &&  //这一次采集的值不能比上一次采集的值大超过15
							 ((max30102_data.Heart == 0) || ( (max30102_data.Heart + 10) > Heart) ))      //这一次采集的值不能比平均值大超过5 
						 {
							   last_heart = Heart;     
				         if( Heart_num < BUFF_NUM_MAX )   //如果缓存空间未填满
						        Heart_buff[Heart_num++] = Heart;
						     else                             //如果缓存空间已满就向前移动数据
						     {
						         for( i = 0; i < BUFF_NUM_MAX -1; i++ )
                         Heart_buff[i] = Heart_buff[i+1];

                     Heart_buff[BUFF_NUM_MAX -1] = Heart;	   					 
						     }
						}
				 }
	
				 /* 计算心率有效值 */
				 if( Heart_num == BUFF_NUM_MAX )    //如果心率缓存空间已满
				 {
					   maxim_sort_ascend(Heart_buff,BUFF_NUM_MAX);
					   //算平均值
					   Heart_Sum = 0;
             for( i = 1 ; i < BUFF_NUM_MAX - 1; i++ )
								 Heart_Sum += Heart_buff[i];
					   
					   if( max30102_data.Heart == 0 )
					       max30102_data.Heart = Heart_Sum/(BUFF_NUM_MAX-2);
						 else
							 max30102_data.Heart = (Heart_Sum + max30102_data.Heart )/(BUFF_NUM_MAX-1);    //计算平均心率时将上一次的心率加进去一起算
						 
						 last_heart = 120;
						    
				 }
#else
if( Heart_valid != 0 && (Heart < 120))    //如果心率数据有效且心率值小于120
         {
				     max30102_data.Heart = Heart;    
				 }				 
				 
#endif
				 
	       NRF_LOG_INFO("SpO2 = %3d, Heart = %3d\r\n",SpO2, Heart);
		}
		else   //检测到未接触皮肤
		{
			   if( ++collect_num > 3)     //连续3S检测到未接触皮肤
				 {
				      max30102_data.Heart = 0;
					    max30102_data.SpO2 = 0;
#ifdef 	SPO2_USER_CACHE				 
					   memset( SpO2_buff, 0,  BUFF_NUM_MAX );
					   SpO2_num = 0;
					   SpO2_Sum = 0;
#endif

#ifdef 	HEART_USER_CACHE						 
					   memset( Heart_buff, 0,  BUFF_NUM_MAX );
					   Heart_num = 0;
					   Heart_Sum = 0;
#endif
				 }
	  }
	
}







