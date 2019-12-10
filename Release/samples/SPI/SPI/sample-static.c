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
#else
#include<unistd.h>
#endif

/* Include D2XX header*/
#include "ftd2xx.h"

/* Include libMPSSE header */
#include "libMPSSE_spi.h"

/* Include AD9957 header */
#include "AD9957Parameter.h"

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

/******************************************************************************/
/*								Global variables							  	    */
/******************************************************************************/
static FT_HANDLE ftHandle;
static uint8 buffer[SPI_DEVICE_BUFFER_SIZE] = {0};

/*!
 * \brief Reads from AD9957
 *
 * This function reads a register from a specified address within the device
 *
 * \param[in] address: Address of the register to read
 * \param[in] *data: Data read from register
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa Operating guide see AD9957 Datasheet
 * \note
 * \warning
 */
static FT_STATUS readReg(uint8 address, uint8 *data, uint8 size)
{
	uint32 sizeToTransfer = 0;
	uint32 sizeTransfered = 0;
	FT_STATUS status;
	static const uint8 AD9957_RWBIT_WRITE = 0x80;

	/* Write register */
	/*     buffer[0]          |      buffer[1] */
	/* 7   6 5 4  3   2  1  0 | 7 6 5 4 3 2 1 0*/
	/*R/Wn X X A4 A3 A2 A1 A0 | D7...........D0*/  
	/*>start-------------------------------end>*/

	/*	Loading Instruction Byte */
	if(address>31){
		printf("Wrong chipAddress or Register Address!\n");
		// return status;
	}
	buffer[0]=(address | AD9957_RWBIT_WRITE);

	/* Write Instruction Byte */
	sizeToTransfer = 1;
	status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE);
	APP_CHECK_STATUS(status);

	/*Read data bytes*/
	sizeToTransfer = size;
	status = SPI_Read(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
	APP_CHECK_STATUS(status);
	for(int i=0;i<size;i++){
		data[i] = buffer[i];
	}

	return status;
}

/******************************************************************************/
/*						Public function definitions						  		   */
/******************************************************************************/
/*!
 * \brief Writes to AD9957 
 *
 * This function writes a register on specified address within the device
 *
 * \param[in] address: Address of the register to write
 * \param[in] data: Data write to register
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa Operating guide see AD9957 Datasheet
 * \note
 * \warning
 */
static FT_STATUS writeReg(uint8 address, uint8 *data, uint8 size)
{
	uint32 sizeToTransfer = 1 + size;
	uint32 sizeTransfered = 0;
	// uint8 buffer[2]; //Localize SPI Buffer here!
	FT_STATUS status;
	static const uint8 AD9957_RWBIT_WRITE = 0x00;

	/* Write register */
	/*     buffer[0]          |      buffer[1] */
	/* 7   6 5 4  3   2  1  0 | 7 6 5 4 3 2 1 0*/
	/*R/Wn X X A4 A3 A2 A1 A0 | D7...........D0*/  
	/*>start-------------------------------end>*/

	/*	Loading Instruction Byte */
	if(address>31){
		printf("Wrong chipAddress or Register Address!\n");
		// return status;
	}
	buffer[0]=(address | AD9957_RWBIT_WRITE);

	/*	Loading Data Byte to */
	for(int i=0;i<size;i++){
		buffer[i+1]=data[i];
	}

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

/******************************************************************************/
/*						Public function definitions						  		   */
/******************************************************************************/
/*!
 * \brief Writes to AD9957 
 *
 * This function writes a register on specified address within the device
 *
 * \param[in] address: Address of the register to write
 * \param[in] data: Data write to register
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa Operating guide see AD9957 Datasheet
 * \note
 * \warning
 */
static FT_STATUS WriteGPIOPin(uint8 dir,uint8 *val,uint8 pin, bool pinval)
{
	FT_STATUS status;
	uint8 writeVal = 0x00;
	uint8 originVal = *val;
	if(pin > 7){
		printf("Pin number out of range!\n");
		return status;
	}
	if(pinval)
		writeVal = ((uint8)(1 << pin)) | originVal;
	else
		writeVal = (~(uint8)(1 << pin)) & originVal;

	status = FT_WriteGPIO(ftHandle,dir,writeVal);
	APP_CHECK_STATUS(status);
	*val = writeVal;

#ifndef __linux__
	Sleep(10);
#endif
	return status;
}

/******************************************************************************/
/*						Public function definitions						  		   */
/******************************************************************************/
/*!
 * \brief Writes to AD9957 
 *
 * This function writes a register on specified address within the device
 *
 * \param[in] address: Address of the register to write
 * \param[in] data: Data write to register
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa Operating guide see AD9957 Datasheet
 * \note
 * \warning
 */
static FT_STATUS WriteRegBit(uint8 address,uint8 *val,uint8 size,uint8 bytenum,uint8 bitnum, bool bitval)
{
	FT_STATUS status;
	if(bitnum > 7){
		printf("Pin number out of range!\n");
		return status;
	}
	bytenum = size-1-bytenum;
	if(bitval)
		val[bytenum] = ((uint8)(1 << bitnum)) | val[bytenum];
	else
		val[bytenum] = (~(uint8)(1 << bitnum)) & val[bytenum];


	status = writeReg(address,val,size);
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
	uint8 size = 0;
	uint8 *data;
	uint8 bytenum = 0;
	uint8 bitnum = 1;
	bool bitval = 1;
	uint32 channels = 0;
	uint8 i = 0;
	uint8 latency = 255;
	uint8 GPIODir = 0xFF;
	uint8 GPIOVal = 0x00;	

	uint8 CFR1Val[CFR1_W] = {0x00,0x00,0x00,0x00};
	uint8 CFR2Val[CFR2_W] = {0x00,0x00,0x00,0x00};
	uint8 IO_UP_RATEVal[IO_UP_RATE_W] = {0x00,0x00,0x01,0x00}; //256 div
	uint8 PROFILE0Val[PROFILE0_W] = {0x3F,0xFF, 0x00,0x00, 0x11,0xEB,0x85,0x1F}; //2'b00,14'd16383,16'd0,32'd300647711
	
	channelConf.ClockRate = 5000; //div 5 for sclk frequency on FT232D
	channelConf.LatencyTimer = latency;
	channelConf.configOptions = SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW;// | SPI_CONFIG_OPTION_CS_ACTIVELOW;
	channelConf.Pin = 0x00000000;/*FinalVal-FinalDir-InitVal-InitDir (for dir 0=in, 1=out)*/

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

		/* Initialize GPIO Pins */
		status = FT_WriteGPIO(ftHandle,GPIODir,GPIOVal);
		status = WriteGPIOPin(GPIODir,&GPIOVal,EXT_PWR_DWN,0);
		status = WriteGPIOPin(GPIODir,&GPIOVal,IO_RESET,0);
		status = WriteGPIOPin(GPIODir,&GPIOVal,IO_UPDATE,0);
		status = WriteGPIOPin(GPIODir,&GPIOVal,MASTER_RESET,0);
		usleep(100);

		/* Master Reset */
		status = WriteGPIOPin(GPIODir,&GPIOVal,MASTER_RESET,1);
		usleep(10);	//Minimum pulse width is 5 SYSCLK period
		status = WriteGPIOPin(GPIODir,&GPIOVal,MASTER_RESET,0);
		usleep(150); //Recovery time from full sleep mode

		/* Enable SDO */
		address = CFR1;
		size = CFR1_W;
		data = CFR1Val;
		bytenum = 0;
		bitnum = 1;
		bitval = 1;
		WriteRegBit(address,data,size,bytenum,bitnum,bitval);
		printf("Write Reg %2x done : ",address);
		for(int i=0;i<size;i++){
			printf("%2x ",data[i]);
		}
		printf("\n");
		status = WriteGPIOPin(GPIODir,&GPIOVal,IO_UPDATE,1);
		usleep(10);
		status = WriteGPIOPin(GPIODir,&GPIOVal,IO_UPDATE,0);
		// getchar();

		readReg(address,data,size);
		printf("Read  Reg %2x done : ",address);
		for(int i=0;i<size;i++){
			printf("%2x ",data[i]);
		}
		printf("\n");

		/* Set IO_UPDATE Rate */
		address = IO_UP_RATE;
		size = IO_UP_RATE_W;
		data = IO_UP_RATEVal; // div SYSCLK/4/2^A/B , 256 here
		writeReg(address,data,size);
		printf("Write Reg %2x done : ",address);
		for(int i=0;i<size;i++){
			printf("%2x ",data[i]);
		}
		printf("\n");
		status = WriteGPIOPin(GPIODir,&GPIOVal,IO_UPDATE,1);
		usleep(10);
		status = WriteGPIOPin(GPIODir,&GPIOVal,IO_UPDATE,0);
		// getchar();

		readReg(address,data,size);
		printf("Read  Reg %2x done : ",address);
		for(int i=0;i<size;i++){
			printf("%2x ",data[i]);
		}
		printf("\n");

		/* Enable Interal Generated IO_UPDATE */
		address = CFR2;
		size = CFR2_W;
		data = CFR2Val;
		readReg(address,data,size);
		printf("Read  Reg %2x done : ",address);
		for(int i=0;i<size;i++){
			printf("%2x ",data[i]);
		}
		printf("\n");

		bytenum = 2;
		bitnum = 7;
		bitval = 1;
		WriteRegBit(address,data,size,bytenum,bitnum,bitval);
		printf("Write Reg %2x done : ",address);
		for(int i=0;i<size;i++){
			printf("%2x ",data[i]);
		}
		printf("\n");
		status = WriteGPIOPin(GPIODir,&GPIOVal,IO_UPDATE,1);
		usleep(10);
		status = WriteGPIOPin(GPIODir,&GPIOVal,IO_UPDATE,0);
		// getchar();
		GPIODir = GPIODir & 0xF7; //set IO_UPDATE as input pin
		status = WriteGPIOPin(GPIODir,&GPIOVal,IO_UPDATE,1);

		readReg(address,data,size);
		printf("Read  Reg %2x done : ",address);
		for(int i=0;i<size;i++){
			printf("%2x ",data[i]);
		}
		printf("\n");

		/* Set mode to Single-Tone */
		address = CFR1;
		size = CFR1_W;
		data = CFR1Val;
		bytenum = 3;
		bitnum = 0;
		bitval = 1;
		WriteRegBit(address,data,size,bytenum,bitnum,bitval);
		printf("Write Reg %2x done : ",address);
		for(int i=0;i<size;i++){
			printf("%2x ",data[i]);
		}
		printf("\n");
		usleep(10);
		// getchar();

		readReg(address,data,size);
		printf("Read  Reg %2x done : ",address);
		for(int i=0;i<size;i++){
			printf("%2x ",data[i]);
		}
		printf("\n");

		/* Set Profile0 */
		address = PROFILE0;
		size = PROFILE0_W;
		data = PROFILE0Val; // div SYSCLK/4/2^A/B , 256 here
		writeReg(address,data,size);
		printf("Write Reg %2x done : ",address);
		for(int i=0;i<size;i++){
			printf("%2x ",data[i]);
		}
		printf("\n");
		usleep(10);
		// getchar();

		readReg(address,data,size);
		printf("Read  Reg %2x done : ",address);
		for(int i=0;i<size;i++){
			printf("%2x ",data[i]);
		}
		printf("\n");

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

