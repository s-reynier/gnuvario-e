#include "VarioSpeaker.h"
#include <HardwareConfig.h>
#include <Arduino.h>

VarioSpeaker::VarioSpeaker() {}

void VarioSpeaker::init()
{
#if defined(HAVE_SPEAKER)
    toneHAL.init();
    beeper.setVolume(10);
    //beeper.generateTone(2000,300);
#endif
}
