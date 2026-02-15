#pragma once

#define SCL_PIN 44      // SLC = D7 = 44
#define SDA_PIN 7       // SDA = D8 = 7
#define TP1_PIN 8       // TP1 = D9 = 8
#define TP2_PIN 9       // TP2 = D10 = 9

#define MAX_I2CBYTE 10
typedef struct  {
  int I2Cstate;
	int BitCount;
	int ByteCount;
	unsigned DataByte;
	int data_addr;
  int DataAvailable;
	unsigned char I2CBuffer[MAX_I2CBYTE];
} i2c_data_t;
extern i2c_data_t i2c_data; 
