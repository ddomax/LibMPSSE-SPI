Modify the example(sample-static.c) for control AD9957.  
AD9957 should be configure in a 4-wired SPI mode with seperated IO pins rather than a bi-directional 3-wired port mode.  
------
SDIO—Serial Data Input/Output
Data is always written into the AD9957 on this pin. However,
this pin can be used as a bidirectional data line. Bit 1 of CFR1,
Register Address 0x00, controls the configuration of this pin.
The default is cleared, which configures the SDIO pin as
bidirectional.
------
Use I/O_RESET to reset the SPI Port State Machine without affecting the addressable registers contents.  
A rising edge on IO_UPDATE with a pulse width that is longer than one SYNC_CLK make data in the port buffer to be written to active registers. (automatic update can be configured through Internal I/O Update Active bit)   
------  
55 SYNC_CLK O Output System Clock/4, Digital Output (Clock). Set up the I/O_UPDATE and
PROFILE2/PROFILE1/PROFILE0 pins to the rising edge of this signal.  
------  
On evalutaion borad, switch W4 to DISABLE(VCC) to put the IO port controlled by tri-state gate U12 into float mode. No any pull-down or pull-up resistors are connected to these ports.  
In the single tone mode (also single chip), only the follwing pins are controlled by the FTXXX device:  
PROFILE[2:0]  
I/O_UPDATE  
SIDO  
SDO  
SCLK  
I/O_RESET  
CSn  
OSK(default disabled, omitting by the chip)  
EXT_PWR_DWN  
MASTER_RESET  
PLL_LOCK(for monitor)  
------
SYSCLK cycle refers to the actual clock frequency used on-chip by the DDS. If the reference clock multiplier is used to multiply the external reference clock frequency,
the SYSCLK frequency is the external frequency multiplied by the reference clock multiplication factor. If the reference clock multiplier and divider are not used, the
SYSCLK frequency is the same as the external reference clock frequency.
------
  
After plug in the FTXXX device, run the following command first to remove the pre-built virtual COM driver.  
    > sudo rmmod ftdi_sio  
    > sudo rmmod usbserial  
Examples see ./Release/samples/SPI/SPI  
Run d.bash (located at ./Release/samples/SPI/) for automatically build and run examples. (sudo ./d.bash)  
