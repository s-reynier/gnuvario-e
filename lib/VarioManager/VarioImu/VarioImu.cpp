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

double VarioImu::getAlti(){
    return this->imu->getAlti();
}

void VarioImu::initKalman(double startp, double starta, double sigmap, double sigmaa, unsigned long timestamp){

}