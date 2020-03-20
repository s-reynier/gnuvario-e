
#include <HardwareConfig.h>

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

public:
    VarioImu();
    void init();
};