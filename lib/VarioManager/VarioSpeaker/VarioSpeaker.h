
#include <HardwareConfig.h>

#ifdef HAVE_SPEAKER
#include <toneHAL.h>
#include <beeper.h>
#endif //HAVE_SPEAKER

class VarioSpeaker
{
public:
    VarioSpeaker();
    void init();
};