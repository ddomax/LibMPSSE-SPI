#ifndef AD9957Parameter_H
#define AD9957Parameter_H

/* Register Address definition */
#define CFR1        0x00
#define CFR2        0x01
#define CFR3        0x02
#define AUX_DAC_CTR 0x03
#define IO_UP_RATE  0x04
#define RAM_SEG_R1  0x05
#define RAM_SEG_R2  0x06
#define ASF_R       0x09
#define SYNC_R      0x0A
#define PROFILE0    0x0E
#define PROFILE1    0x0F
#define PROFILE2    0x10
#define PROFILE3    0x11
#define PROFILE4    0x12
#define PROFILE5    0x13
#define PROFILE6    0x14
#define PROFILE7    0x15
#define RAM_R       0x16
#define GPIO_CONFIG 0x18
#define GPIO_DATA   0x19

/* Register Size (Bytes) Definition */
#define CFR1_W        4
#define CFR2_W        4
#define CFR3_W        4
#define AUX_DAC_CTR_W 4
#define IO_UP_RATE_W  4
#define RAM_SEG_R1_W  6
#define RAM_SEG_R2_W  6
#define ASF_R_W       4
#define SYNC_R_W      4
#define PROFILE0_W    8
#define PROFILE1_W    8
#define PROFILE2_W    8
#define PROFILE3_W    8
#define PROFILE4_W    8
#define PROFILE5_W    8
#define PROFILE6_W    8
#define PROFILE7_W    8
#define RAM_R_W       4
#define GPIO_CONFIG_W 2
#define GPIO_DATA_W   2

/* Resignter Default Value definition */
#define CFR1_DEFAULT    0x00

/* Pins bound to GPIO */
/* ACBUS[7:0] -> GPIOH[7:0] */
#define IO_RESET        0
#define EXT_PWR_DWN     1
#define MASTER_RESET    2
#define IO_UPDATE       3

#endif