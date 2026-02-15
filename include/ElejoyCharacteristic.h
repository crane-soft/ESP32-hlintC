#pragma once
#include <ArduinoBLE.h>


// https://www.youtube.com/watch?v=XXs1b-iohg4
// https://www.uuidgenerator.net/

#define ELEJOY_SERVICE_UUID			"bf94"
#define ELEJOY_CHARACTERISTIC_UUID	"98e6"

class ElejoyCharacteristic : public BLECharacteristic 
{
public:
	ElejoyCharacteristic();
private:
	BLEDescriptor Descriptor2901; 
	BLEDescriptor Descriptor2904;
};
