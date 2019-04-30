/*******************************************************************************************
*          版权所属, C ,2018- ,成都传奇兄弟信息技术有限公司
*
*作者:嵌入式开发部              日期:2018-11-6            版本:V0.0
*说明:
*    对芯片内部flash操作。
*    操作时传入的地址时相对地址
*修改:
*    2018-11-5:增加固件升级功能。
*******************************************************************************************/
#include "sdk_config.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "boards.h"
#include "ble_nus.h"
#include "fstorage.h"
#include "fstorage_internal_defs.h"
#include "fds.h"
#include "app_fds.h"
#include "app_uart_ex.h"
#include "nrf_delay.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#ifdef FLASH_DEBUG
    #define mem_dbg NRF_LOG_INFO
#else
    #define mem_dbg(...)
#endif

#define NUM_PAGES 1                                   //分配四页
#define PAGE_SIZE_WORDS     FS_PAGE_SIZE_WORDS        //每页大小 4096/4 字

static uint8_t fs_callback_flag;                      //回调函数标识
static void fs_evt_handler(fs_evt_t const *const evt, fs_ret_t result);

/* 默认配置FS */
FS_REGISTER_CFG(fs_config_t fs_config) =
{
    .callback= fs_evt_handler,                        // Function for event callbacks.
    .num_pages= NUM_PAGES,                            // Number of physicalflash pages required.
    .priority = 0xFE                                  // Priority for flash usage.
};
/**********************************************************************
函数名:fs_evt_handler
功能:flash事件回调函数
输入参数:
@evt:事件
@result:结果
输出参数:None
返回值:None
说明:
***********************************************************************/
static void fs_evt_handler(fs_evt_t const *const evt, fs_ret_t result)
{
    if (result != FS_SUCCESS)
    {
//        mem_dbg("\r\nfstorage command Error full ycompleted\r\n")
    }
    else
    {
//        mem_dbg("\r\nfstorage command success full ycompleted\r\n");
        fs_callback_flag = 0;
    }
}

/**********************************************************************
函数名:Flash_Init
功能:初始化Flash
输入参数:None
输出参数:None
返回值:None
说明:
***********************************************************************/
void Flash_Init(void)
{
    fs_ret_t ret = fs_init();
    if(ret != FS_SUCCESS)
    {
        mem_dbg("fs init error\r\n");
					   return;
    }

    mem_dbg("Flash address 0x%X: \r\n", (uint32_t)fs_config.p_start_addr);
}


/**********************************************************************
函数名:Flash_Read
功能:从指定地址读取，指定长度的数据
输入参数:
@start_addr:开始读取的地址(相对地址,以0开始)
@len:需要读取的长度(以字位单位)
输出参数:
@pData:读取的数据返回
返回值:fs_ret_t
说明:
Flash读取数据，都是以字读(32位)
***********************************************************************/
fs_ret_t Flash_Read(const uint32_t start_addr, uint32_t *pData, const uint16_t len)
{
	   if( (start_addr + len) > (NUM_PAGES * PAGE_SIZE_WORDS))
				{
					   mem_dbg("fs Read ADDR ERR\r\n");
				    return FS_ERR_INVALID_ADDR;
				}
				
	   mem_dbg("Flash Read Data:0x%X\r\n",(uint32_t)(fs_config.p_start_addr + start_addr));
    for(int i=0; i<len; i++)
    {
        pData[i]= *(fs_config.p_start_addr + start_addr + i);
    }
				
				return FS_SUCCESS;
}

/**********************************************************************
函数名:Flash_Erase_Page
功能:从指定的页数开始，连续擦除页
输入参数:
@start_page:开始的页
@number:需要擦出的页数
输出参数:None
返回值:fs_ret_t
说明:
***********************************************************************/
fs_ret_t Flash_Erase_Page(const uint8_t start_page, const uint8_t number )
{
	   if( (start_page + number) > NUM_PAGES )
				{
					   mem_dbg("fs Erase ADDR ERR\r\n");
				    return FS_ERR_INVALID_ADDR;
				}
				
    mem_dbg("Erasinga flash page at address 0x%X\r\n", (uint32_t)(fs_config.p_start_addr + start_page * PAGE_SIZE_WORDS));

    fs_callback_flag= 1;

    fs_ret_t ret= fs_erase(&fs_config, (uint32_t *)(fs_config.p_start_addr + start_page * PAGE_SIZE_WORDS), number, NULL);
    if(ret != FS_SUCCESS)
    {
        mem_dbg("fs erase error\r\n");
					   return ret;
    }

				uint32_t time_out = 1000;
    while(fs_callback_flag== 1 && time_out--)
    {
					    nrf_delay_ms(1);
    }
				if( time_out != 0 )
        return FS_SUCCESS;		
    else
				{
					   mem_dbg("fs erase timeout\r\n");
        return FS_ERR_OPERATION_TIMEOUT;		
				}					
}

/**********************************************************************
函数名:Flash_Write
功能:向指定地址连续写数据
输入参数:
@start_addr:写入的开始地址(相对地址,以0开始)
@p_data:需要写入的数据(以字位单位)
@len:需要写入的数据的长度(以字位单位)
输出参数:None
返回值:fs_ret_t
说明:
***********************************************************************/
fs_ret_t Flash_Write(const uint32_t start_addr, const uint32_t *p_data, const uint16_t len )
{
	   uint16_t i = 0;
	
	   if( (start_addr + len) > (NUM_PAGES * PAGE_SIZE_WORDS))
				{
					   mem_dbg("fs Write ADDR ERR\r\n");
				    return FS_ERR_INVALID_ADDR;
				}
				
				uint32_t time_out = 1000;
				
        mem_dbg("Flash Write Data:0x%X\r\n", (uint32_t)(fs_config.p_start_addr + start_addr));
    
				uint8_t start_page_number  = start_addr/PAGE_SIZE_WORDS;                                        //计算数据存储的开始页
				uint8_t end_page_number = (start_addr + len)/PAGE_SIZE_WORDS;                                   //计算数据存储的结束页
				
				mem_dbg("start_page_nuber = %d, end_page_nuber = %d\r\n",start_page_number, end_page_number);
				uint32_t data_buff[PAGE_SIZE_WORDS] = {0};                                                      //定义页数据缓存
					
				/* Read page data */
				uint32_t read_addr = start_page_number * PAGE_SIZE_WORDS;                                       //计算该页的首地址
				Flash_Read(read_addr, data_buff, PAGE_SIZE_WORDS);                                                 //读取整个页的数据
				
				Flash_Erase_Page(start_page_number, 1);                                                         //擦出这个页
    
				/* 将数据添加到缓存 */
				uint16_t write_len = ((start_page_number + 1) * PAGE_SIZE_WORDS - start_addr);
				write_len = ( write_len > len )?len:write_len;
				
				for( i = 0; i < write_len; i++ )
				{
				     data_buff[(start_addr - start_page_number*PAGE_SIZE_WORDS)+i] = p_data[i];    
				}
   
				/* 写入数据 */
				fs_callback_flag = 1;
				fs_ret_t ret= fs_store(&fs_config, (fs_config.p_start_addr + start_page_number * PAGE_SIZE_WORDS), (uint32_t *)data_buff, FS_PAGE_SIZE_WORDS, NULL);
    if(ret != FS_SUCCESS)
    {
        mem_dbg("fs write error\r\n");
					   return ret;
    }

    while(fs_callback_flag== 1 && time_out--)
    {
					    nrf_delay_ms(1);
    }
				time_out = 1000;
				if( time_out == 0 )
				{
					   mem_dbg("fs write timeout\r\n");
        return FS_ERR_OPERATION_TIMEOUT;	
				}
				
				/* 如果此次存储需要使用多个页 */
				while(start_page_number != end_page_number)  
				{
					   start_page_number++;                                                          //跳到下一页
					   memset(data_buff, 0, sizeof(data_buff));                                      //清除缓存
				    	
					   /* Read page data */
				    uint32_t read_addr = start_page_number * PAGE_SIZE_WORDS;                                       //计算该页的首地址
				    Flash_Read(read_addr, data_buff, PAGE_SIZE_WORDS);                                                 //读取整个页的数据
				
				    Flash_Erase_Page(start_page_number, 1);                                                         //擦出这个页
    
			    	/* 将数据添加到缓存 */
				    uint16_t write_len = len - start_page_number * PAGE_SIZE_WORDS;
					   write_len = ( write_len > PAGE_SIZE_WORDS )?PAGE_SIZE_WORDS:write_len;
				
				    for( i = 0; i < write_len; i++ )
				    {
				         data_buff[i] = p_data[start_page_number * PAGE_SIZE_WORDS - start_addr + i];    
				    }
   
				    /* 写入数据 */
				    fs_callback_flag = 1;
				    fs_ret_t ret= fs_store(&fs_config, (fs_config.p_start_addr + start_page_number * PAGE_SIZE_WORDS), (uint32_t *)data_buff, FS_PAGE_SIZE_WORDS, NULL);
        if(ret != FS_SUCCESS)
        {
            mem_dbg("fs write error\r\n");
					       return ret;
        }

        while(fs_callback_flag== 1 && time_out--)
        {
					       nrf_delay_ms(1);
        }
				    time_out = 1000;
				    if( time_out == 0 )
				    {
					       mem_dbg("fs write timeout\r\n");
            return FS_ERR_OPERATION_TIMEOUT;	
				    }
				}
				return FS_SUCCESS;
}


