
#include "VarioManager.h"
#include <HardwareConfig.h>
#include <Arduino.h>
#include <DebugConfig.h>

VarioManager::VarioManager()
{
    varioAlim = new VarioAlim;
    varioSpeaker = new VarioSpeaker;
    varioImu = new VarioImu;
}

void VarioManager::init()
{
#ifdef PROG_DEBUG
    ///  while (!SerialPort) { ;}
    char tmpbuffer[100];
    sprintf(tmpbuffer, "GNUVARIO compiled on %s", __DATE__); // at %s", __DATE__, __TIME__);
    SerialPort.println(tmpbuffer);
    sprintf(tmpbuffer, "VERSION %i.%i - %s", VERSION, SUB_VERSION, DEVNAME);
    SerialPort.println(tmpbuffer);
    if (BETA_CODE > 0)
    {
        SerialPort.print("Beta ");
        SerialPort.println(BETA_CODE);
    }
    SerialPort.flush();
#endif //PRO_DEBBUG

#if defined(ESP32)
    if (BETA_CODE > 0)
    {
        ESP_LOGI(TAG, "GnuVario-E version %d.%d Beta %d.", VERSION, SUB_VERSION, BETA_CODE);
    }
    else
    {
        ESP_LOGI(TAG, "GnuVario-E version %d.%d.", VERSION, SUB_VERSION);
        //  ESP_LOGE(TAG, "Failed to initialize the card (%d). Make sure SD card lines have pull-up resistors in place.", ret);
    }
#endif //ESP32
}

void VarioManager::initAlim()
{
    this->varioAlim->init();
}

void VarioManager::initSpeaker()
{
    this->varioSpeaker->init();
}

void VarioManager::initImu(){
    this->varioImu->init();
}