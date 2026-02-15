
#include "BleHandler.h"

csBleHandler::csBleHandler():  
    elejoyService (ELEJOY_SERVICE_UUID)
{
}

void csBleHandler::setup(const uint8_t * data, int size)
{
    BLEdata = data;
    BLEsize = size;
    if (BLE.begin()) {
        BLE.setDeviceName("elejoyMonitor");
        BLE.setLocalName("elejoyMonitor");
        BLE.setAdvertisedService(elejoyService); 

        elejoyService.addCharacteristic(elejoyCharacteristic); 
        BLE.addService(elejoyService);      
        BLE.advertise();
        connected = false;
    }
}

void  csBleHandler::Execute()
{
    if (connected == false) {
        HandyDevice = BLE.central();
        if (! HandyDevice) 
            return;
        connected = true;
		SendDelay = millis();;
    } else {
		if (HandyDevice.connected()) {            
			if (millis() > SendDelay) {
				SendData();
				SendDelay = millis() + 100;
			}
		} else {
			connected = false;
		}
	}
}

void csBleHandler::SendData()
{
    elejoyCharacteristic.writeValue(BLEdata, BLEsize);
}
