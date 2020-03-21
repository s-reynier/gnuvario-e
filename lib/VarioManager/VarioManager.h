#include <VarioVersion.h>

#include <HardwareConfig.h>
#include <VarioAlim/VarioAlim.h>
#include <VarioSpeaker/VarioSpeaker.h>
#include <VarioImu/VarioImu.h>

class VarioManager
{
private:
    VarioAlim *varioAlim;
    VarioSpeaker *varioSpeaker;
    VarioImu *varioImu;

public:
    VarioManager();
    void init();
    void initAlim();
    void initSpeaker();
    void initImu();
    double getAlti();
    void initKalman(double startp, double starta, double sigmap, double sigmaa, unsigned long timestamp);
};
