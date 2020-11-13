/*********************************************************************************/
/*                                                                               */
/*                           Libraries SdCardHal                                 */
/*                                                                               */
/*  version    Date     Description                                              */
/*    1.0    19/04/19                                                            */
/*    1.1    26/11/19   Ajout SDFat                                              */
/*                                                                               */
/*********************************************************************************/
#ifndef sdcardHAL_h
#define sdcardHAL_h

#include <Arduino.h>

#include <HardwareConfig.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"

class SdCardHAL
{ //public SDClass {

public:
	boolean begin(void);
	boolean end(void);
	static void deleteRecursive(String path);
};

#define SDHAL_SD SD

#define SDCARD_STATE_INITIAL 0
#define SDCARD_STATE_INITIALIZED 1
#define SDCARD_STATE_READY 2
#define SDCARD_STATE_ERROR -1

extern SdCardHAL SDHAL;

#endif
