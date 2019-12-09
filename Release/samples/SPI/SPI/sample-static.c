/*!
 * \file sample-static.c
 *
 * \author FTDI
 * \date 20110512
 *
 * Copyright � 2000-2014 Future Technology Devices International Limited
 *
 *
 * THIS SOFTWARE IS PROVIDED BY FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Project: libMPSSE
 * Module: SPI Sample Application - Interfacing 94LC56B SPI EEPROM
 *
 * Rivision History:
 * 0.1  - 20110512 - Initial version
 * 0.2  - 20110801 - Changed LatencyTimer to 255
 * 					 Attempt to open channel only if available
 *					 Added & modified macros
 *					 Included stdlib.h
 * 0.3  - 20111212 - Added comments
 * 0.41 - 20140903 - Fixed compilation warnings
 *					 Added testing of SPI_ReadWrite()
 */

/******************************************************************************/
/* 							 Include files										   */
/******************************************************************************/
/* Standard C libraries */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
/* OS specific libraries */
#ifdef _WIN32
#include<windows.h>
#endif

/* Include D2XX header*/
#include "ftd2xx.h"

/* Include libMPSSE header */
#include "libMPSSE_spi.h"

/******************************************************************************/
/*								Macro and type defines							   */
/******************************************************************************/
/* Helper macros */

#define APP_CHECK_STATUS(exp) {if(exp!=FT_OK){printf("%s:%d:%s(): status(0x%x) \
!= FT_OK\n",__FILE__, __LINE__, __FUNCTION__,exp);exit(1);}else{;}};
#define CHECK_NULL(exp){if(exp==NULL){printf("%s:%d:%s():  NULL expression \
encountered \n",__FILE__, __LINE__, __FUNCTION__);exit(1);}else{;}};

/* Application specific macro definations */
#define SPI_DEVICE_BUFFER_SIZE		256
#define CHANNEL_TO_OPEN			0	/*0 for first available channel, 1 for next... */
#define CHIP_ADDR_ALL			0

/******************************************************************************/
/*								Global variables							  	    */
/******************************************************************************/
static FT_HANDLE ftHandle;
static uint8 buffer[SPI_DEVICE_BUFFER_SIZE] = {0};

/*!
 * \brief Reads from Hittie RF PLLs with integrated VCO (Read HMC Device)
 *
 * This function reads a register from a specified address within the HMC Devices
 *
 * \param[in] chipAddress: 3bit device address for OpenMode only, 3'b000 for all devices)
 * \param[in] address: Address of the register to read
 * \param[in] *data: Data read from register
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa Operating guide see https://www.analog.com/media/en/technical-documentation/user-guides/pll_operating_guide_rf_vcos.pdf
 * \note
 * \warning
 */
static FT_STATUS read_reg(uint8 chipAddress, uint8 address, uint16 *data)
{
	uint32 sizeToTransfer = 0;
	uint32 sizeTransfered;
	uint8 writeComplete=0;
	uint32 retry=0;
	FT_STATUS status;

	/* CS_High + Write command + Address */
	sizeToTransfer=1;
	sizeTransfered=0;
	buffer[0] = 0xC0;/* Write command (3bits)*/
	buffer[0] = buffer[0] | ( ( address >> 3) & 0x0F );/*5 most significant add bits*/
	status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE);
	APP_CHECK_STATUS(status);

	/*Write partial address bits */
	sizeToTransfer=4;
	sizeTransfered=0;
	buffer[0] = ( address & 0x07 ) << 5; /* least significant 3 address bits */
	status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BITS);
	APP_CHECK_STATUS(status);

	/*Read 2 bytes*/
	sizeToTransfer=2;
	sizeTransfered=0;
	status = SPI_Read(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
	APP_CHECK_STATUS(status);

	*data = (uint16)(buffer[1]<<8);
	*data = (*data & 0xFF00) | (0x00FF & (uint16)buffer[0]);

	return status;
}

/******************************************************************************/
/*						Public function definitions						  		   */
/******************************************************************************/
/*!
 * \brief Writes to Hittie RF PLLs with integrated VCO (Write HMC Device) 
 *
 * This function writes a register on specified address within the HMC Devices
 *
 * \param[in] chipAddress: 3bit device address for OpenMode only, 3'b000 for all devices)
 * \param[in] address: Address of the register to write
 * \param[in] data: Data write to register
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa Operating guide see https://www.analog.com/media/en/technical-documentation/user-guides/pll_operating_guide_rf_vcos.pdf
 * \note
 * \warning
 */
static FT_STATUS write_reg(uint8 chipAddress, uint8 address, uint32 data)
{
	uint32 sizeToTransfer = 0;
	uint32 sizeTransfered=0;
	FT_STATUS status;

	/* Write register */
	sizeToTransfer=4;
	sizeTransfered=0;
	/*     buffer[0]   |      buffer[1]  |  ...*/
	/*       bit                bit         ...*/
	/* 7 6 5 4 3 2 1 0 | 7 6 5 4 3 2 1 0 |  ...*/
	/*>start-------------------------------end>*/
	/*              Example:                   */
	/*      0x9F       |        0xAA     |  ...*/
	/* 1 0 0 1 1 1 1 1 | 1 0 1 0 1 0 1 0 |  ...*/

	/*	Loading 24bit data to [31:8]	*/
	buffer[0]=(uint8)(data>>16);/* MSB First, stop at the specified length(bits)*/
	buffer[1]=(uint8)(data>>8);
	buffer[2]=(uint8)(data>>0);

	/*	Loading 5bit Reg address and 3bit Chip address to [7:0] */
	/*  RegAddress[7:4] ChipAddress[3:0] */
	if(chipAddress>7 || address>31){
		printf("Wrong chipAddress or Register Address!\n");
		// return status;
	}
	buffer[3]=(address<<3 | chipAddress);

	status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
	APP_CHECK_STATUS(status);

#ifndef __linux__
	Sleep(10);
#endif
	return status;
}

/*!
 * \brief Main function / Entry point to the sample application
 *
 * This function is the entry point to the sample application. It opens the channel, writes to the
 * EEPROM and reads back.
 *
 * \param[in] none
 * \return Returns 0 for success
 * \sa
 * \note
 * \warning
 */
int main()
{
	FT_STATUS status = FT_OK;
	FT_DEVICE_LIST_INFO_NODE devList = {0};
	ChannelConfig channelConf = {0};
	uint8 address = 0;
	uint32 channels = 0;
	uint32 data = 0;
	uint8 i = 0;
	uint8 latency = 255;	
	
	channelConf.ClockRate = 5000;
	channelConf.LatencyTimer = latency;
	channelConf.configOptions = SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW;// | SPI_CONFIG_OPTION_CS_ACTIVELOW;
	channelConf.Pin = 0xffFFffFF;/*FinalVal-FinalDir-InitVal-InitDir (for dir 0=in, 1=out)*/

	/* init library */
#ifdef _MSC_VER
	Init_libMPSSE();
#endif
	status = SPI_GetNumChannels(&channels);
	APP_CHECK_STATUS(status);
	printf("Number of available SPI channels = %d\n",(int)channels);

	if(channels>0)
	{
		for(i=0;i<channels;i++)
		{
			status = SPI_GetChannelInfo(i,&devList);
			APP_CHECK_STATUS(status);
			printf("Information on channel number %d:\n",i);
			/* print the dev info */
			printf("		Flags=0x%x\n",devList.Flags);
			printf("		Type=0x%x\n",devList.Type);
			printf("		ID=0x%x\n",devList.ID);
			printf("		LocId=0x%x\n",devList.LocId);
			printf("		SerialNumber=%s\n",devList.SerialNumber);
			printf("		Description=%s\n",devList.Description);
			printf("		ftHandle=0x%x\n",(unsigned int)devList.ftHandle);/*is 0 unless open*/
		}

		/* Open the first available channel */
		status = SPI_OpenChannel(CHANNEL_TO_OPEN,&ftHandle);
		APP_CHECK_STATUS(status);
		printf("\nhandle=0x%x status=0x%x SPI Channel Opened. All pins are inputs(float) now!\n",(unsigned int)ftHandle,status);
		status = SPI_InitChannel(ftHandle,&channelConf);
		APP_CHECK_STATUS(status);
		printf("\nhandle=0x%x status=0x%x SPI Channel Init done. All pins are configured!\n",(unsigned int)ftHandle,status);

		address = 0x04;
		data = 0xF0F0F0;
		printf("writing address = %02d data = %x\n",address,data);
		write_reg(CHIP_ADDR_ALL, address, data);
		printf("Write done.\n");

		// read_reg(SPI_SLAVE_0, address,&data);
		// printf("reading address = %02d data = %d\n",address,data);

		status = SPI_CloseChannel(ftHandle);
		printf("SPI Channel Closed.\n");
	}

#ifdef _MSC_VER
	Cleanup_libMPSSE();
#endif

#ifndef __linux__
	system("pause");
#endif
	return 0;
}

