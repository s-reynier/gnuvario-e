#include <HardwareConfig.h>
#include <kalmanvert.h>
#include "VarioDataTwoWireScheduler.h"
#include <IntTW.h>
#include <ms5611TW.h>
#include <vertaccel.h>
#include <LightInvensense.h>
#include <TwoWireScheduler.h>


/***************/
/* IMU objects */
/***************/

#ifdef HAVE_BMP280
Bmp280 TWScheduler::bmp280;
#else
Ms5611 TWScheduler::ms5611;
#endif
#ifdef HAVE_ACCELEROMETER
Vertaccel TWScheduler::vertaccel;
#endif //HAVE_ACCELEROMETER


class VarioData
{
public:
    VarioData();
    void init();
    void getData();
    double getVelocity();
    double getCalibratedPosition();

private:
    Kalmanvert kalmanVert;
    const float POSITION_MEASURE_STANDARD_DEVIATION = 0.1;

#ifdef HAVE_ACCELEROMETER
    const float ACCELERATION_MEASURE_STANDARD_DEVIATION = 0.3;
#else
    const float ACCELERATION_MEASURE_STANDARD_DEVIATION = 0.6;
#endif //HAVE_ACCELEROMETER

#ifdef TWOWIRESCHEDULER
    /* init kalman filter with 0.0 accel*/
    double firstAlti = twScheduler.getAlti();
    VarioDataTwoWireScheduler dataClass;
#else //TWOWIRESCHEDULER
    VarioDataNotTwoWireScheduler dataClass;
    double firstAlti = ms5611.readPressure();
#endif
    double tmpAlti;
    double tmpTemp;
    double tmpAccel;
};

//extern Kalmanvert kalmanvert;
extern VarioData varioData;
