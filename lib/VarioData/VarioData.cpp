#include "VarioData.h"
#include "VarioDataTwoWireScheduler.h"
#include <HardwareConfig.h>
#include <DebugConfig.h>

#ifdef AGL_DEBUG
#define ARDUINOTRACE_ENABLE 1
#else
#define ARDUINOTRACE_ENABLE 0
#endif

#define ARDUINOTRACE_SERIAL SerialPort
#include <ArduinoTrace.h>

#include <VarioLog.h>

//****************************************************************************************************************************
VarioData::VarioData()
//****************************************************************************************************************************
{
}

void VarioData::init()
{
    kalmanVert.init(firstAlti,
                    0.0,
                    POSITION_MEASURE_STANDARD_DEVIATION,
                    ACCELERATION_MEASURE_STANDARD_DEVIATION,
                    millis());
}


void VarioData::getData()
{
    dataClass.getData(tmpAlti, tmpTemp, tmpAccel);

#ifdef DATA_DEBUG
    SerialPort.print("Alti : ");
    SerialPort.println(tmpAlti);
    SerialPort.print("Temperature : ");
    SerialPort.println(tmpTemp);
    SerialPort.print("Accel : ");
    SerialPort.println(tmpAccel);
#endif //DATA_DEBUG

    kalmanVert.update(tmpAlti,
                      tmpAccel,
                      millis());
}

double VarioData::getVelocity()
{
    return kalmanVert.getVelocity();
}

double VarioData::getCalibratedPosition(){
     return kalmanVert.getCalibratedPosition();
}