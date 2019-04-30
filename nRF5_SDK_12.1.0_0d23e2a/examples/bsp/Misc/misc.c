#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <ctype.h>
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "app_uart.h"
#include "boards.h"
#include "app_uart_ex.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "misc.h"


void get_mac_addr(uint8_t *p_mac_addr)
{
    uint64_t mac_address = 0;
    int8_t i = 0;
    ble_gap_addr_t device_addr;

    if (p_mac_addr == NULL)
    {
        return ;
    }
#ifdef S130
    if(NRF_SUCCESS == sd_ble_gap_address_get(&device_addr))
#else
    if(NRF_SUCCESS == sd_ble_gap_addr_get(&device_addr))
#endif
    {
        for(i = 0; i < BLE_GAP_ADDR_LEN; ++i) {
           mac_address |= ((uint64_t)device_addr.addr[i]) << (i * 8); 
        }
    }
    else
    {
        mac_address = ((((uint64_t)(NRF_FICR->DEVICEADDR[1] & 0xFFFF)) << 32) | ((uint64_t)NRF_FICR->DEVICEADDR[0]));
    }
    
//    for(i = (BLE_GAP_ADDR_LEN - sizeof(uint16_t) - 1); i>=0; --i)
//    {
//        p_mac_addr[(BLE_GAP_ADDR_LEN - sizeof(uint16_t) - 1) - i] =  (mac_address >> 8*i) & 0xff;
//    }
    for ( uint8_t i = 6; i >0;)
	{	
		i--;
		p_mac_addr[5-i]= (mac_address >> 8*i) & 0xff;;
	}
}

int misc_atoi (char s[], uint8_t len)
{ 
	int i,n = 0,sign = 1;
	for(i=0;isspace(s[i]);i++) //跳过空白符 ; 
		sign=(s[i]=='-')?-1:1;
	if(s[i]=='+' || s[i] == '-')    //跳过符号
		i++; 
	for(n=0;isdigit(s[i]), i < len;i++)
		n=10*n+(s[i]-'0');       //将数字字符转换成整形数字

	return sign *n;
}


