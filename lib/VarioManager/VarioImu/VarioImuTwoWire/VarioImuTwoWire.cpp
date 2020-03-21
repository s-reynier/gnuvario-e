#include "VarioImuTwoWire.h"
#include <HardwareConfig.h>
#include <Arduino.h>

#ifdef TWOWIRESCHEDULER
#ifdef HAVE_BMP280
Bmp280 TWScheduler::bmp280;
#else
Ms5611 TWScheduler::ms5611;
#endif

#ifdef HAVE_ACCELEROMETER
Vertaccel TWScheduler::vertaccel;
#endif //HAVE_ACCELEROMETER

VarioImuTwoWire::VarioImuTwoWire()
{
}

void VarioImuTwoWire::init()
{
    /**************************/
    /* init Two Wires devices */
    /**************************/
    //!!!
#ifdef HAVE_ACCELEROMETER
    intTW.begin();
    twScheduler.init();
    //  vertaccel.init();

#endif //HAVE_ACCELEROMETER
}

double VarioImuTwoWire::getAlti()
{
    return twScheduler.getAlti();
}
#endif