#include <soc/gpio_pins.h>
#include <soc/gpio_reg.h>
#include <esp_attr.h>
#include <soc/gpio_struct.h>
#include "i2cSniffer.h"


#define SCL_PIN16   (SCL_PIN-32)
#define SDA_PIN16   SDA_PIN

#define SCL_PIN_MASK (1 << SCL_PIN16)
#define SDA_PIN_MASK (1 << SDA_PIN16)        
#define TP1_PIN_MASK (1 << TP1_PIN)     
#define TP2_PIN_MASK (1 << TP2_PIN)     

#define SDA_STATUS_REG GPIO_STATUS_REG
#define SCL_STATUS_REG GPIO_STATUS1_REG

#define SDA_STATUS_W1TC_REG GPIO_STATUS_W1TC_REG
#define SCL_STATUS_W1TC_REG GPIO_STATUS1_W1TC_REG

#define SDA_PIN_REG     GPIO_PIN0_REG+(SDA_PIN*4) 
#define SCL_PIN_REG     GPIO_PIN0_REG+(SCL_PIN*4) 

#define SDA_IN_REG      GPIO_IN_REG
#define SCL_IN_REG      GPIO_IN1_REG

#define GPIO_INTR_POSEDGE  1
#define GPIO_INTR_NEGEDGE  2

#define I2C_ADDR	0x5A

i2c_data_t i2c_data; 

inline void IRAM_ATTR restart() 
{
	i2c_data.I2Cstate = 0;
	// Pin-Config change to falling edge
	GPIO.pin[SDA_PIN].int_type = GPIO_INTR_NEGEDGE;

}

inline void IRAM_ATTR not_valid() 
{
	i2c_data.DataAvailable = 0;
	restart();
}

inline void IRAM_ATTR disable_i2c() 
{
	i2c_data.I2Cstate = 0;
	GPIO.pin[SDA_PIN].int_type = 0;
	GPIO.pin[SCL_PIN].int_type = 0;
}

void IRAM_ATTR sda_event()
{
 	if ((READ_PERI_REG (SCL_IN_REG) & SCL_PIN_MASK) != 0) {
    	// Start or Stop occurs only if SCL = 1     
		// TP2 = 1 
    	if (i2c_data.I2Cstate == 0) {
	    	WRITE_PERI_REG (GPIO_OUT_W1TS_REG,TP2_PIN_MASK);
      		// Startbit detected
      		i2c_data.I2Cstate = 1;
			i2c_data.BitCount = 0;
			i2c_data.ByteCount = 0;

			// Pin-Config change to rising edge
      		GPIO.pin[SDA_PIN].int_type = GPIO_INTR_POSEDGE;

		} else {
			// Stopbit detected, 
			if (i2c_data.I2CBuffer[0] == I2C_ADDR) {
				// i2c wr-frame
				if (i2c_data.ByteCount == 2) {
					// save i2c data-addr
					i2c_data.data_addr = i2c_data.I2CBuffer[1];
					i2c_data.DataAvailable = 1; // 1 = data-addr valif
					restart();
				} else {
					not_valid();
				}

			} else {
				// i2c rd-frame
				if ((i2c_data.I2CBuffer[0] == I2C_ADDR+1) 
				 && (i2c_data.ByteCount == 9) 
				 && (i2c_data.DataAvailable == 1)) {
				    // new data available
			    	WRITE_PERI_REG (GPIO_OUT_W1TS_REG,TP2_PIN_MASK);
					i2c_data.DataAvailable = 2;
					disable_i2c();
				} else {
					not_valid();
				}
			}
    	}
  	}
}

void IRAM_ATTR scl_event()
{
   if (i2c_data.I2Cstate == 1) {
		// read bit only if startbit detected
        WRITE_PERI_REG (GPIO_OUT_W1TS_REG,TP1_PIN_MASK);
		i2c_data.DataByte <<= 1;
		if ((READ_PERI_REG (SDA_IN_REG) & SDA_PIN_MASK) != 0) {
			i2c_data.DataByte += 1;
		}
		++i2c_data.BitCount;
		if (i2c_data.BitCount == 8) {
			if (i2c_data.ByteCount < MAX_I2CBYTE) {
				i2c_data.I2CBuffer[i2c_data.ByteCount] = i2c_data.DataByte;
				++i2c_data.ByteCount;

			} else {
				not_valid();
			}
		} else { // bitcount != 8
			if (i2c_data.BitCount == 9) {
				i2c_data.BitCount = 0;
			}
		}
    }
}


void IRAM_ATTR hlintC_main () 
{
	if ((READ_PERI_REG (SDA_STATUS_REG) & SDA_PIN_MASK) != 0) {
		WRITE_PERI_REG(SDA_STATUS_W1TC_REG,SDA_PIN_MASK);
		sda_event();
	}
	if ((READ_PERI_REG (SCL_STATUS_REG) & SCL_PIN_MASK) != 0) {
		WRITE_PERI_REG(SCL_STATUS_W1TC_REG,SCL_PIN_MASK);
		scl_event();
	} 
    WRITE_PERI_REG (GPIO_OUT_W1TC_REG,TP1_PIN_MASK);	
  	WRITE_PERI_REG (GPIO_OUT_W1TC_REG,TP2_PIN_MASK);

}
