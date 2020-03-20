#include "VarioDataTwoWireScheduler.h"
#include <HardwareConfig.h>
#include <Arduino.h>

VarioDataTwoWireScheduler::VarioDataTwoWireScheduler()
{
}

void VarioDataTwoWireScheduler::getData(double tmpAlti, double tmpTemp, double tmpAccel)
{
#ifdef HAVE_ACCELEROMETER
    if (twScheduler.havePressure() && twScheduler.haveAccel())
    {
        compteurErrorMPU = 0;
        twScheduler.getTempAlti(tmpTemp, tmpAlti);
        tmpAccel = twScheduler.getAccel(NULL);
    }
#else
    if (twScheduler.havePressure())
    {
#ifdef MS5611_DEBUG
//    SerialPort.println("havePressure");
#endif //MS5611_DEBUG

        double tmpAlti, tmpTemp;
        twScheduler.getTempAlti(tmpTemp, tmpAlti);
        tmpAccel = 0.0;
    }
#endif