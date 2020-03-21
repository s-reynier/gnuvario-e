
#include <HardwareConfig.h>
#include <kalmanvert.h>

#ifdef TWOWIRESCHEDULER
#include <VarioImu/VarioImuTwoWire/VarioImuTwoWire.h>
#else
#include <VarioImu/VarioImuStd/VarioImuStd.h>
#endif

class VarioImu
{
private:
#ifdef TWOWIRESCHEDULER
    VarioImuTwoWire *imu;
#else
    VarioImuStd *imu;
#endif
    Kalmanvert kalmanVert;
    const float POSITION_MEASURE_STANDARD_DEVIATION = 0.1;

#ifdef HAVE_ACCELEROMETER
    const float ACCELERATION_MEASURE_STANDARD_DEVIATION = 0.3;
#else
    const float ACCELERATION_MEASURE_STANDARD_DEVIATION = 0.6;
#endif //HAVE_ACCELEROMETER
    double tmpAlti;
    double tmpTemp;
    double tmpAccel;

public:
    VarioImu();
    void init();
    double getAlti();
    
};