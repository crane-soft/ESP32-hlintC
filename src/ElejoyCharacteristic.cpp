#include "ElejoyCharacteristic.h"


const uint8_t FormatUint16[] = {
  0x04, 		// Format = 6 = "unsigned 16-bit integer"
  0x00, 		// Exponent = 0
  0x00,0x27, 	// Unit = 0x2700 = "unitless" (low byte)
  0x01, 		// Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00,0x00 	// Description = 0 = "unknown" (low byte)
};

ElejoyCharacteristic::ElejoyCharacteristic() :
	BLECharacteristic("ELEJOY_CHARACTERISTIC_UUID",  BLERead | BLENotify, 32),
	Descriptor2901 ("2901","I2C-Data"),
	Descriptor2904 ("2904",FormatUint16, sizeof (FormatUint16))

{
	addDescriptor(Descriptor2901);
	addDescriptor(Descriptor2904);
}
