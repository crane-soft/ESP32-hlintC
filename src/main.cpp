#include <Arduino.h>
#include "BleHandler.h"
#include "elejoy.h"

int delTimer = 0;
int led_state = 1;
const byte led =LED_BUILTIN;
TickType_t xLastWakeTime;

csBleHandler BleHandler;

void setup() 
{
	Serial.begin(115200);
	pinMode(led,OUTPUT);
	BleHandler.setup ((uint8_t*)&elejoyData, sizeof(elejoyData));
	delay (2000);
  	Serial.println("XIAO Started");  
	sniffer_setup();
	xLastWakeTime = xTaskGetTickCount();
}

void loop() {
	vTaskDelayUntil( &xLastWakeTime, 1 );
	if (++ delTimer == 500) {
		delTimer = 0;
		digitalWrite(led, led_state);
		led_state ^=1;
	    if (led_state == 1) {
	      sniffer_print();
		}
	}
 
	scan_i2c_data();
 	BleHandler.Execute();

}

