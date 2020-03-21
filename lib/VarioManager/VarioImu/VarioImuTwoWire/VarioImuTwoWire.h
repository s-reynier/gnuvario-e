#include <HardwareConfig.h>

#ifdef TWOWIRESCHEDULER

#include <IntTW.h>
#include <ms5611TW.h>
#include <vertaccel.h>
#include <LightInvensense.h>
#include <TwoWireScheduler.h>

class VarioImuTwoWire
{

public:
    VarioImuTwoWire();
    void init();
    double getAlti();
};

#endif