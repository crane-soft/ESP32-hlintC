#include <ArduinoBLE.h>
#include <soc/periph_defs.h>
#include "i2cSniffer.h"
#include "elejoy.h"

#define INTR_NUM 31
#define INTR_FLAG	ESP_INTR_FLAG_LEVEL5
#define CPU_CORE 1


void config_int_pin (int pinNo, gpio_int_type_t int_type)
{
  gpio_config_t io_conf;
  io_conf.intr_type = int_type;
  io_conf.pin_bit_mask = 1ULL << pinNo;	        // bit mask of the pin(s)
  io_conf.mode = GPIO_MODE_INPUT;                       // set as input mode
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;         // disable pull-down mode
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;	            // enable pull-up mode
  gpio_config(&io_conf);
}

void restart_i2c_intr()
{
	i2c_data.I2Cstate = 0;
	i2c_data.DataAvailable = 0;
	ESP_INTR_DISABLE(INTR_NUM); 
	gpio_set_intr_type ((gpio_num_t)SCL_PIN,GPIO_INTR_POSEDGE);
	gpio_set_intr_type ((gpio_num_t)SDA_PIN,GPIO_INTR_NEGEDGE);
  	ESP_INTR_ENABLE(INTR_NUM); 
}

void sniffer_setup()
{
	i2c_data.I2Cstate = 0;
	i2c_data.DataAvailable = 0;

	pinMode(TP1_PIN,OUTPUT);
	pinMode(TP2_PIN,OUTPUT);
  
	config_int_pin (SCL_PIN,GPIO_INTR_POSEDGE);
  	config_int_pin (SDA_PIN,GPIO_INTR_NEGEDGE);

  	ESP_INTR_DISABLE(INTR_NUM);                           // disable interrupt 
  	intr_matrix_set(CPU_CORE, ETS_GPIO_INTR_SOURCE, INTR_NUM);
  	ESP_INTR_ENABLE(INTR_NUM);                            // enable interrupt 
}
  
uint16_t bigEndian (uint8_t * data) 
{
	return ((uint16_t)data[0] << 8) | data[1];
}

eElejoy_t elejoyData;
int test_cnt;
void scan_i2c_data()
{
    if (i2c_data.DataAvailable == 2) {
		++test_cnt;
        if (i2c_data.data_addr == 0) {
		    digitalWrite (TP2_PIN,1);
			elejoyData.inpVoltage = bigEndian(&i2c_data.I2CBuffer[1]);
			elejoyData.inpCurrent = bigEndian(&i2c_data.I2CBuffer[3]);
			elejoyData.outVoltage = bigEndian(&i2c_data.I2CBuffer[5]);
			elejoyData.outCurrent = bigEndian(&i2c_data.I2CBuffer[7]);
		} else if (i2c_data.data_addr == 0x30) {
			elejoyData.temperature = bigEndian(&i2c_data.I2CBuffer[1]);
  		}
		i2c_data.DataAvailable = 0;
		restart_i2c_intr();
	    digitalWrite (TP2_PIN,0);

	}
}

void sniffer_print() 
{
	Serial.print (test_cnt);
	Serial.print (",");
	Serial.print (elejoyData.inpVoltage);
	Serial.print (",");
	Serial.print (elejoyData.outVoltage);
	Serial.print (",");
	Serial.println(elejoyData.temperature);
}
