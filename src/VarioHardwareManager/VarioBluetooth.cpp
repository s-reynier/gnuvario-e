#include <Arduino.h>
#include <HardwareConfig.h>
#include <DebugConfig.h>

#include <VarioLog.h>

#ifndef ARDUINOTRACE_ENABLE
#ifdef HARDWARE_DEBUG
#define ARDUINOTRACE_ENABLE 1
#else
#define ARDUINOTRACE_ENABLE 0
#endif
#endif

#ifndef ARDUINOTRACE_SERIAL
#define ARDUINOTRACE_SERIAL SerialPort
#endif

#include <ArduinoTrace.h>

#ifdef HAVE_BLUETOOTH

#include "VarioBluetooth.h"

//#include "SimpleBLE.h"
#include <VarioSettings.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// ******************************
bool VarioBluetooth::init()
// ******************************
{
	if (GnuSettings.VARIOMETER_ENABLE_BT) {
		TRACE();
		return true;
	}
	else {
		TRACE();
		return false;
	}
}


// ******************************
bool VarioBluetooth::update(double velocity, double position, double calibratedPosition)
// ******************************
{
  // ********************
  // * update bluetooth *
  // ********************
#ifdef HAVE_GPS
  /* in priority send vario nmea sentence */
  if (bluetoothNMEA.available())
  {
    while (bluetoothNMEA.available())
    {
      serialNmea.write(bluetoothNMEA.get());
    }
    serialNmea.release();
		return true;
  }
#else //!HAVE_GPS
  /* check the last vario nmea sentence */
  if (millis() - lastVarioSentenceTimestamp > VARIOMETER_SENTENCE_DELAY)
  {
    lastVarioSentenceTimestamp = millis();
#ifdef VARIOMETER_BLUETOOTH_SEND_CALIBRATED_ALTITUDE
    bluetoothNMEA.begin(calibratedPosition, velocity);
#else
    bluetoothNMEA.begin(position, velocity);
#endif
    while (bluetoothNMEA.available())
    {
      serialNmea.write(bluetoothNMEA.get());
    }
	  return true;
  }
#endif //!HAVE_GPS
  return false;
}

#endif //HAVE_BLUETOOTH
