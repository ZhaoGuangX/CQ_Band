/*******************************************************************************************
*          版权所属, C ,2018- ,成都传奇兄弟信息技术有限公司
*
*作者:嵌入式开发部              日期:2018-9-1            版本:V0.0
*说明:
*    该工程基于，Nordic官方nRF5_SDK_12.1.0_0d23e2a.实现BLE主从一体通信。该
*工程需要驱动MPU6050,OLED等硬件，实现计步，心率采集等功能。
*修改:
*
*******************************************************************************************/


#include "spi.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#define   SPI_INSTANCE  0 /**< SPI instance index. */
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */
static volatile bool   spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */

#define   SPI_BUFSIZE   8

static uint8_t        m_tx_buf[SPI_BUFSIZE] = {0};             /**< TX buffer. */
static uint8_t        m_rx_buf[SPI_BUFSIZE] = {0};             /**< RX buffer. */
static const uint8_t  m_length = SPI_BUFSIZE;                  /**< Transfer length. */
static uint8_t        SPIReadLength, SPIWriteLength;

#define SPI_READBIT           (0x80)

/*************************************************************************
函数名:spi_event_handler
功能:SPI事件回调函数
输入参数:
@p_event:传递事件的指针
输出参数:None
返回值:None
说明:
*************************************************************************/
void spi_event_handler(nrf_drv_spi_evt_t const * p_event)
{
    spi_xfer_done = true;
//    NRF_LOG_INFO("Transfer completed.\r\n");
    if (m_rx_buf[0] != 0)
    {
//        NRF_LOG_INFO(" Received:0x%02x\r\n",m_rx_buf[0]);
    }
}

/*************************************************************************
函数名:SPI_Init
功能:SPI事件回调函数
输入参数:
@p_event:传递事件的指针
输出参数:None
返回值:None
说明:
*************************************************************************/
void SPI_Init(void)
{
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin   = SPI_SS_PIN;
    spi_config.miso_pin = SPI_MISO_PIN;
    spi_config.mosi_pin = SPI_MOSI_PIN;
    spi_config.sck_pin  = SPI_SCK_PIN;
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spi_event_handler));
}

/*************************************************************************
函数名:SPI_UnInit
功能:关闭SPI
输入参数:None
输出参数:None
返回值:None
说明:
*************************************************************************/
void SPI_UnInit(void)
{
	   /* 关闭SPI功能 */
	   nrf_drv_spi_uninit(&spi);            
	
	   /* 关闭SPI使用的串口功能 */
	   nrf_gpio_cfg_default(SPI_SS_PIN);
	   nrf_gpio_cfg_default(SPI_MISO_PIN);
   	 nrf_gpio_cfg_default(SPI_MOSI_PIN);
	   nrf_gpio_cfg_default(SPI_SCK_PIN);
}

/*******************************************************************************
* Function Name		: SPI_WriteReg
* Description		: Generic Writing function. It must be fullfilled with either
*			: I2C or SPI writing function
* Input			: Register Address, Data to be written
* Output		: None
* Return		: None
*******************************************************************************/
uint8_t SPI_WriteReg(uint8_t WriteAddr, uint8_t Data) 
{
  
    memset(m_rx_buf, 0, m_length);
    spi_xfer_done = false;
  
    m_tx_buf[0] = WriteAddr;
	  m_tx_buf[1] = Data;
    SPIWriteLength = 2;
    SPIReadLength = 0;
  
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, m_tx_buf, SPIWriteLength, m_rx_buf, SPIReadLength));

    while (!spi_xfer_done)
    {
       __WFE();
    } 
    return 1;
}

/*******************************************************************************
* Function Name		: LIS3DH_ReadReg
* Description		: Generic Reading function. It must be fullfilled with either
*			: I2C or SPI reading functions					
* Input			: Register Address
* Output		: Data REad
* Return		: None
*******************************************************************************/
uint8_t SPI_ReadReg(uint8_t Reg, uint8_t* Data) 
{
  
    // Reset rx buffer and transfer done flag
    memset(m_rx_buf, 0, m_length);
    spi_xfer_done = false;
  
    m_tx_buf[0] = Reg | SPI_READBIT;
	  m_tx_buf[1] = 0x56;
    SPIWriteLength = 2;
    SPIReadLength = 2;
  
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, m_tx_buf, SPIWriteLength, m_rx_buf, SPIReadLength));

    while (!spi_xfer_done)
    {
       __WFE();
    } 
    *Data =  m_rx_buf[1];
    return m_rx_buf[1];
}


