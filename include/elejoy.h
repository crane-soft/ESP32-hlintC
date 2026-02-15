#pragma once
#include <stdint.h>

typedef  struct {
      //uint16_t flags;
      uint16_t inpVoltage;
      uint16_t inpCurrent;
      uint16_t outVoltage;
      uint16_t outCurrent;
	  uint16_t temperature;
  } eElejoy_t;
 
 extern eElejoy_t elejoyData;

 void sniffer_setup();
 void scan_i2c_data();
 void sniffer_print();