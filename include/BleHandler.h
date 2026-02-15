#pragma once
#include <Arduino.h>
#include "ElejoyCharacteristic.h"
class csBleHandler
{
public:
    csBleHandler();
    void setup(const uint8_t * data, int size);
    void Execute();

private:

    void SendData();

    const uint8_t * BLEdata;
    int BLEsize;
    BLEDevice HandyDevice;
    BLEService elejoyService;
    ElejoyCharacteristic elejoyCharacteristic;
    bool connected;
	uint32_t SendDelay;
};
