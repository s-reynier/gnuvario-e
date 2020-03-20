#include "VarioImu.h"
#include <HardwareConfig.h>
#include <Arduino.h>

VarioImu::VarioImu()
{
#ifdef TWOWIRESCHEDULER
    imu = new VarioImuTwoWire;
#else
    imu = new VarioImuStd;
#endif
}

void VarioImu::init()
{
    this->imu->init();
}