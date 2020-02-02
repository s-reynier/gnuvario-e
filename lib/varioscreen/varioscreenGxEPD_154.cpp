/* varioscreenGxEPD_154 -- 
 *
 * Copyright 2019 Jean-philippe GOI
 * 
 * This file is part of GnuVario-E.
 *
 * ToneHAL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ToneHAL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/* 
 *********************************************************************************
 *********************************************************************************
 *                                                                               *
 *                           VarioScreenGxEPD_154                                *
 *                               Ecran 1.54''                                    *
 *                                                                               *
 *  version    Date     Description                                              *
 *    1.0    24/09/19                                                            *
 *    1.0.1  28/09/19   Modification des noms des librairies                     *
 *    1.0.2  09/10/19   Ajout showtitle                                          *
 *    1.0.3	 13/10/19		Ajout Wind																							 *	
 *    1.0.4  07/11/19   Modif updateScreen																			 *
 *    1.0.5  11/01/20   Modif ScreenViewPage																		 *
 *                      Modif effacement de zone +1 à gauche et +3 pour vitesse  *
 *    1.0.6  17/01/20   Desactivation effacement ligne 1427                      *
 *    1.0.7  20/01/20   Modif ScreenViewReboot																	 *
 *                                                                               *
 *********************************************************************************/
 
 /*
 *********************************************************************************
 *                    conversion image to cpp code                               *
 *                                                                               *
 *      https://javl.github.io/image2cpp/                                        *
 *                                                                               *
 *********************************************************************************/

#include <HardwareConfig.h>
#include <DebugConfig.h>

#if (VARIOSCREEN_SIZE == 154)

#include <varioscreenObjects_154.h>

#include <varioscreenGxEPD_154.h>
#include <Arduino.h>

#if defined(ESP32)
static const char* TAG = "VarioScreen";
#include "esp_log.h"
#endif //ESP32

//#include <avr\dtostrf.h>
#include <stdlib.h>

/* http://javl.github.io/image2cpp/ */

#include <VarioButton.h>

#include <imglib/gridicons_sync.h>
#include <varioscreenIcone_154.h>

#include <VarioSettings.h>
#include <toneHAL.h>

#include <Utility.h>

#include <SysCall.h>

#ifdef __AVR__
  #include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
  #include <pgmspace.h>
#endif

// Many (but maybe not all) non-AVR board installs define macros
// for compatibility with existing PROGMEM-reading AVR code.
// Do our own checks and defines here for good measure...

#ifndef pgm_read_byte
 #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif
#ifndef pgm_read_word
 #define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif
#ifndef pgm_read_dword
 #define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#endif

// Pointers are a peculiar case...typically 16-bit on AVR boards,
// 32 bits elsewhere.  Try to accommodate both...

#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
 #define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))
#else
 #define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif

volatile uint8_t stateDisplay;
volatile unsigned long oldtime;
volatile uint8_t led1stat = 0; 
volatile uint8_t stateMulti = 0;

#define VARIOSCREEN_DOT_WIDTH 6
#define VARIOSCREEN_DIGIT_WIDTH 11

#define ColorScreen    GxEPD_WHITE
#define ColorText      GxEPD_BLACK

#define VARIOSCREEN_TENSION_ANCHOR_X 140
#define VARIOSCREEN_TENSION_ANCHOR_Y 170

#define VARIOSCREEN_AUTONOMIE_ANCHOR_X 90
#define VARIOSCREEN_AUTONOMIE_ANCHOR_Y 235

/*****************************************/
/* screen objets Page 0                  */
/*****************************************/

#define VARIOSCREEN_ALTI_ANCHOR_X 110
#define VARIOSCREEN_ALTI_ANCHOR_Y 80
#define VARIOSCREEN_ALTI_UNIT_ANCHOR_X    120
#define VARIOSCREEN_VARIO_ANCHOR_X 90
//#define VARIOSCREEN_VARIO_ANCHOR_X 5
#define VARIOSCREEN_VARIO_ANCHOR_Y 135
#define VARIOSCREEN_VARIO_UNIT_ANCHOR_X   100
#define VARIOSCREEN_VARIO_UNIT_ANCHOR_Y   110
#define VARIOSCREEN_SPEED_ANCHOR_X 49
#define VARIOSCREEN_SPEED_ANCHOR_Y 190
#define VARIOSCREEN_SPEED_UNIT_ANCHOR_X 52
#define VARIOSCREEN_SPEED_UNIT_ANCHOR_Y 165
#define VARIOSCREEN_GR_ANCHOR_X 130
#define VARIOSCREEN_GR_ANCHOR_Y 135
#define VARIOSCREEN_INFO_ANCHOR_X 17
#define VARIOSCREEN_INFO_ANCHOR_Y 0
#define VARIOSCREEN_VOL_ANCHOR_X 44
#define VARIOSCREEN_VOL_ANCHOR_Y 0
#define VARIOSCREEN_RECCORD_ANCHOR_X 84
#define VARIOSCREEN_RECCORD_ANCHOR_Y 0
#define VARIOSCREEN_BAT_ANCHOR_X 124
#define VARIOSCREEN_BAT_ANCHOR_Y 0
#define VARIOSCREEN_SAT_ANCHOR_X 164
#define VARIOSCREEN_SAT_ANCHOR_Y 0
#define VARIOSCREEN_SAT_FIX_ANCHOR_X 176
#define VARIOSCREEN_SAT_FIX_ANCHOR_Y 32
#define VARIOSCREEN_TIME_ANCHOR_X 198
#define VARIOSCREEN_TIME_ANCHOR_Y 190
#define VARIOSCREEN_ELAPSED_TIME_ANCHOR_X 198
#define VARIOSCREEN_ELAPSED_TIME_ANCHOR_Y 190
#define VARIOSCREEN_BT_ANCHOR_X 0
#define VARIOSCREEN_BT_ANCHOR_Y 4
#define VARIOSCREEN_TREND_ANCHOR_X 157 //120
#define VARIOSCREEN_TREND_ANCHOR_Y 57  //111
#define VARIOSCREEN_WIND_ANCHOR_X 200
#define VARIOSCREEN_WIND_ANCHOR_Y 33

/*****************************************/
/* screen objets Page 1                  */
/*****************************************/
#define VARIOSCREEN_TEMP_ANCHOR_X 90
#define VARIOSCREEN_TEMP_ANCHOR_Y 190
#define VARIOSCREEN_TEMP_UNIT_ANCHOR_X 160

/*****************************************/
/* screen objets Page 10 - Calibrate GPS */
/*****************************************/
#define VARIOSCREEN_GPS_PERIOD_ANCHOR_X 190
#define VARIOSCREEN_GPS_PERIOD_ANCHOR_Y 80
#define VARIOSCREEN_GPS_MEAN_PERIOD_ANCHOR_X 190
#define VARIOSCREEN_GPS_MEAN_PERIOD_ANCHOR_Y 120
#define VARIOSCREEN_GPS_DURATION_ANCHOR_X 190
#define VARIOSCREEN_GPS_DURATION_ANCHOR_Y 160

VarioScreen screen;

/**********/
/* screen */
/**********/

//****************************************************************************************************************************
//****************************************************************************************************************************
//								VARIOSCREEN
//****************************************************************************************************************************
//****************************************************************************************************************************

//****************************************************************************************************************************
VarioScreen::~VarioScreen() {
//****************************************************************************************************************************
//	free(tensionDigit);
	free(displayList);
	free(schedulerScreen);
}

#define ITEMS_IN_ARRAY(array)   (sizeof(array) / sizeof(*array))

//****************************************************************************************************************************
void VarioScreen::init(void)
//****************************************************************************************************************************
{
#ifdef SCREEN_DEBUG
	SerialPort.println("init");	
#endif //SCREEN_DEBUG

  display.init(0);   ///115200);  pour affichage debug Screen GxEPD2
	
//  setRotation(2);

#ifdef SCREEN_DEBUG
	SerialPort.println("fillScreen");	
#endif //SCREEN_DEBUG

  display.setFullWindow();

  display.fillScreen(GxEPD_WHITE);
	
#ifdef SCREEN_DEBUG
	SerialPort.println("setTextColor");	
#endif //SCREEN_DEBUG
	
  display.setTextColor(GxEPD_BLACK);
}
	
//****************************************************************************************************************************
void VarioScreen::createScreenObjects(void)
//****************************************************************************************************************************
{
	
#ifdef SCREEN_DEBUG
	SerialPort.println("Create screen objects");	
#endif //SCREEN_DEBUG
	
	/* création des champs d'affichage */
	
	
//	ScreenDigit tensionDigit(TENSION_DISPLAY_POSX /*VARIOSCREEN_TENSION_ANCHOR_X*/, VARIOSCREEN_TENSION_ANCHOR_Y, 5, 2, false, false, ALIGNRIGHT);

/*	tensionDigit = new ScreenDigit(VARIOSCREEN_TENSION_ANCHOR_X, VARIOSCREEN_TENSION_ANCHOR_Y, 5, 2, false, false, ALIGNRIGHT);	
	tempratureDigit = new ScreenDigit(VARIOSCREEN_TENSION_ANCHOR_X, VARIOSCREEN_TENSION_ANCHOR_Y, 5, 2, false, false, ALIGNRIGHT);*/
	
	createScreenObjectsPage0();
	createScreenObjectsPage1();
//	createScreenObjectsPage10();
	
//	displayList  = new ScreenSchedulerObject[3];
	MaxObjectList = 0;
	
#ifdef SCREEN_DEBUG
	SerialPort.println("schedulerScreen : createObjectDisplay");	
#endif //SCREEN_DEBUG
	
	createScreenObjectsDisplayPage0();
	createScreenObjectsDisplayPage1();
//	createScreenObjectsDisplayPage10();
}	
	
//****************************************************************************************************************************
void VarioScreen::createScreenObjectsPage0(void) {
//****************************************************************************************************************************
	altiDigit = new ScreenDigit(VARIOSCREEN_ALTI_ANCHOR_X, VARIOSCREEN_ALTI_ANCHOR_Y, 4, 0, false, false, ALIGNRIGHT, true, DISPLAY_OBJECT_ALTI);
	munit = new MUnit(VARIOSCREEN_ALTI_UNIT_ANCHOR_X, VARIOSCREEN_ALTI_ANCHOR_Y);
	varioDigit = new ScreenDigit(VARIOSCREEN_VARIO_ANCHOR_X, VARIOSCREEN_VARIO_ANCHOR_Y, 4, 1, true, false,  ALIGNRIGHT, true, DISPLAY_OBJECT_VARIO);
//	varioDigit = new ScreenDigit(VARIOSCREEN_VARIO_ANCHOR_X, VARIOSCREEN_VARIO_ANCHOR_Y, 4, 1, true, false,  ALIGNLEFT, true, DISPLAY_OBJECT_VARIO);
	msunit = new MSUnit(VARIOSCREEN_VARIO_UNIT_ANCHOR_X, VARIOSCREEN_VARIO_UNIT_ANCHOR_Y);
	kmhunit = new KMHUnit(VARIOSCREEN_SPEED_UNIT_ANCHOR_X, VARIOSCREEN_SPEED_UNIT_ANCHOR_Y);
	speedDigit = new ScreenDigit(VARIOSCREEN_SPEED_ANCHOR_X, VARIOSCREEN_SPEED_ANCHOR_Y, 2, 0, false, false, ALIGNRIGHT, true, DISPLAY_OBJECT_SPEED);
	ratioDigit = new ScreenDigit(VARIOSCREEN_GR_ANCHOR_X, VARIOSCREEN_GR_ANCHOR_Y, 2, 0, false, true, ALIGNLEFT, true, DISPLAY_OBJECT_RATIO);
	trendDigit = new ScreenDigit(VARIOSCREEN_GR_ANCHOR_X, VARIOSCREEN_GR_ANCHOR_Y, 3, 1, false, true, ALIGNLEFT, true, DISPLAY_OBJECT_TREND);

	infoLevel = new INFOLevel(VARIOSCREEN_INFO_ANCHOR_X, VARIOSCREEN_INFO_ANCHOR_Y);
	volLevel = new VOLLevel(VARIOSCREEN_VOL_ANCHOR_X, VARIOSCREEN_VOL_ANCHOR_Y);
	recordIndicator = new RECORDIndicator(VARIOSCREEN_RECCORD_ANCHOR_X, VARIOSCREEN_RECCORD_ANCHOR_Y);
	trendLevel = new TRENDLevel(VARIOSCREEN_TREND_ANCHOR_X, VARIOSCREEN_TREND_ANCHOR_Y);

	batLevel = new BATLevel(VARIOSCREEN_BAT_ANCHOR_X, VARIOSCREEN_BAT_ANCHOR_Y, VOLTAGE_DIVISOR_VALUE, VOLTAGE_DIVISOR_REF_VOLTAGE);

	satLevel = new SATLevel(VARIOSCREEN_SAT_ANCHOR_X, VARIOSCREEN_SAT_ANCHOR_Y);

	timeMDigit = new ScreenDigit (VARIOSCREEN_TIME_ANCHOR_X-55, VARIOSCREEN_TIME_ANCHOR_Y, 2, 0, false, true, ALIGNLEFT, false, DISPLAY_OBJECT_DURATION);
	timeHDigit = new ScreenDigit (VARIOSCREEN_TIME_ANCHOR_X-73, VARIOSCREEN_TIME_ANCHOR_Y, 2, 0, false, true, ALIGNRIGHT, true, DISPLAY_OBJECT_TIME);

	screenTime = new ScreenTime(VARIOSCREEN_TIME_ANCHOR_X, VARIOSCREEN_TIME_ANCHOR_Y, *timeHDigit, *timeMDigit,false);
	screenElapsedTime = new ScreenElapsedTime(VARIOSCREEN_ELAPSED_TIME_ANCHOR_X, VARIOSCREEN_ELAPSED_TIME_ANCHOR_Y, *timeHDigit, *timeMDigit);

	fixgpsinfo = new FIXGPSInfo(VARIOSCREEN_SAT_FIX_ANCHOR_X, VARIOSCREEN_SAT_FIX_ANCHOR_Y);
	btinfo = new BTInfo(VARIOSCREEN_BT_ANCHOR_X, VARIOSCREEN_BT_ANCHOR_Y);
	
	/*
  display.drawLine(130, 26, 295, 26, GxEPD_BLACK);
  display.drawLine(130, 26, 130, 0, GxEPD_BLACK);
  display.drawLine(295, 26, 295, 0, GxEPD_BLACK);
  display.drawLine(190, 26, 190, 127, GxEPD_BLACK);
  display.drawCircle(245, 80, 40, GxEPD_BLACK);*/
	
/*	bgline1 = new BGLine(130, 26, 295, 26);
	bgline2 = new BGLine(130, 26, 130, 0);
	bgline3 = new BGLine(295, 26, 295, 0);
	bgline4 = new BGLine(190, 26, 190, 127);	
	
	bgcircle = new BGCircle(245, 80, 40);*/

//	wind = new WIND(VARIOSCREEN_WIND_ANCHOR_X, VARIOSCREEN_WIND_ANCHOR_Y);
}
	
//****************************************************************************************************************************
void VarioScreen::createScreenObjectsPage10(void) {
//****************************************************************************************************************************
	gpsPeriodDigit 			= new ScreenDigit(VARIOSCREEN_GPS_PERIOD_ANCHOR_X, VARIOSCREEN_GPS_PERIOD_ANCHOR_Y, 6, 1, false, false, ALIGNRIGHT, true);
	gpsMeanPeriodDigit 	= new ScreenDigit(VARIOSCREEN_GPS_MEAN_PERIOD_ANCHOR_X, VARIOSCREEN_GPS_MEAN_PERIOD_ANCHOR_Y, 6, 1, false, false,  ALIGNRIGHT, true);
	gpsDurationDigit	 	= new ScreenDigit(VARIOSCREEN_GPS_DURATION_ANCHOR_X, VARIOSCREEN_GPS_DURATION_ANCHOR_Y, 6, 1, false, false,  ALIGNRIGHT, true);
}

//****************************************************************************************************************************
void VarioScreen::createScreenObjectsPage1(void) {
//****************************************************************************************************************************	
	tempDigit 					= new ScreenDigit(VARIOSCREEN_TEMP_ANCHOR_X, VARIOSCREEN_TEMP_ANCHOR_Y, 2, 0, false, false, ALIGNLEFT, false, DISPLAY_OBJECT_TEMPERATURE);
	tunit 							= new TUnit(VARIOSCREEN_TEMP_UNIT_ANCHOR_X, VARIOSCREEN_TEMP_ANCHOR_Y);
}
	
//****************************************************************************************************************************
void VarioScreen::createScreenObjectsDisplayPage0(void) {
//****************************************************************************************************************************
//	CreateObjectDisplay(DISPLAY_OBJECT_TENSION, tensionDigit, 0, 0, true); 
//	CreateObjectDisplay(DISPLAY_OBJECT_TEMPRATURE, tempratureDigit, 0, 2, true); 
		CreateObjectDisplay(DISPLAY_OBJECT_ALTI							, altiDigit					, 0, 0, true); 
		CreateObjectDisplay(DISPLAY_OBJECT_MUNIT						, munit							, 0, 0, true); 
		CreateObjectDisplay(DISPLAY_OBJECT_VARIO						, varioDigit				, 0, 0, true); 
		CreateObjectDisplay(DISPLAY_OBJECT_MSUNIT						, msunit						, 0, 0, true); 
		CreateObjectDisplay(DISPLAY_OBJECT_KMHUNIT					, kmhunit						, 0, 0, true); 
		CreateObjectDisplay(DISPLAY_OBJECT_SPEED						, speedDigit				, 0, 0, true); 
	
#ifdef SCREEN_DEBUG
		SerialPort.print("RATIO_CLIMB_RATE : ");	
		SerialPort.println(GnuSettings.RATIO_CLIMB_RATE);	
#endif //SCREEN_DEBUG
	
    if (GnuSettings.RATIO_CLIMB_RATE == 1) {	
		  CreateObjectDisplay(DISPLAY_OBJECT_RATIO					, ratioDigit				, 0, 0, true); 
		} else if (GnuSettings.RATIO_CLIMB_RATE == 2) {	
		  CreateObjectDisplay(DISPLAY_OBJECT_TREND					, trendDigit				, 0, 0, true); 
		} else {
		  CreateObjectDisplay(DISPLAY_OBJECT_RATIO					, ratioDigit				, 0, 1, true); 
		  CreateObjectDisplay(DISPLAY_OBJECT_TREND					, trendDigit				, 0, 2, true); 			
    }
		CreateObjectDisplay(DISPLAY_OBJECT_INFOLEVEL				, infoLevel					, 0, 0, true); 
		CreateObjectDisplay(DISPLAY_OBJECT_VOLLEVEL					, volLevel					, 0, 0, true); 
		CreateObjectDisplay(DISPLAY_OBJECT_RECORDIND				, recordIndicator		, 0, 0, true); 
		CreateObjectDisplay(DISPLAY_OBJECT_TRENDLEVEL				, trendLevel				, 0, 0, true); 
		CreateObjectDisplay(DISPLAY_OBJECT_BATLEVEL					, batLevel					, 0, 0, true); 
		CreateObjectDisplay(DISPLAY_OBJECT_SATLEVEL					, satLevel					, 0, 0, true); 
		CreateObjectDisplay(DISPLAY_OBJECT_SCREENTIME				, screenTime				, 0, 1, true); 
		CreateObjectDisplay(DISPLAY_OBJECT_SCREENELAPSEDTIME, screenElapsedTime	, 0, 2, true); 		
		CreateObjectDisplay(DISPLAY_OBJECT_FIXGPSINFO				, fixgpsinfo				, 0, 0, true); 
		CreateObjectDisplay(DISPLAY_OBJECT_BTINFO						, btinfo						, 0, 0, true); 
		
		CreateObjectDisplay(DISPLAY_OBJECT_BTINFO						, btinfo						, 0, 0, true); 

/*		CreateObjectDisplay(DISPLAY_OBJECT_LINE	          	, bgline1           , 0, 0, true);
		CreateObjectDisplay(DISPLAY_OBJECT_LINE	          	, bgline2           , 0, 0, true);
		CreateObjectDisplay(DISPLAY_OBJECT_LINE	          	, bgline3           , 0, 0, true);
		CreateObjectDisplay(DISPLAY_OBJECT_LINE	          	, bgline4           , 0, 0, true);

		CreateObjectDisplay(DISPLAY_OBJECT_CIRCLE         	, bgcircle          , 0, 0, true);		*/		
		
//		CreateObjectDisplay(DISPLAY_OBJECT_WIND         		, wind          		, 0, 0, true);				
}
	
//****************************************************************************************************************************
void VarioScreen::createScreenObjectsDisplayPage10(void) {
//****************************************************************************************************************************
		CreateObjectDisplay(DISPLAY_OBJECT_GPS_PERIOD				, gpsPeriodDigit		, 10, 0, true); 
		CreateObjectDisplay(DISPLAY_OBJECT_GPS_MEAN_PERIOD	, gpsMeanPeriodDigit, 10, 0, true); 
		CreateObjectDisplay(DISPLAY_OBJECT_GPS_DURATION			, gpsDurationDigit	, 10, 0, true); 

		CreateObjectDisplay(DISPLAY_OBJECT_VOLLEVEL					, volLevel					, 10, 0, true); 
		CreateObjectDisplay(DISPLAY_OBJECT_BATLEVEL					, batLevel					, 10, 0, true); 
		CreateObjectDisplay(DISPLAY_OBJECT_SATLEVEL					, satLevel					, 10, 0, true); 
}	

//****************************************************************************************************************************
void VarioScreen::createScreenObjectsDisplayPage1(void) {
//****************************************************************************************************************************
		CreateObjectDisplay(DISPLAY_OBJECT_TEMPERATURE			, tempDigit		, 1, 0, true); 
		CreateObjectDisplay(DISPLAY_OBJECT_TUNIT						, tunit				, 1, 0, true); 
}	

//****************************************************************************************************************************
void VarioScreen::begin(void)
//****************************************************************************************************************************
{
	
#ifdef SCREEN_DEBUG
	SerialPort.println("schedulerScreen : create");	
		
	SerialPort.print("begin - objectCount : ");	
	SerialPort.println(ITEMS_IN_ARRAY(displayList));	
	SerialPort.print("begin - MaxobjectCount : ");	
	SerialPort.println(MaxObjectList);	

//	SerialPort.println(sizeof(displayList);	
#endif //SCREEN_DEBUG
	
//	schedulerScreen = new ScreenScheduler(displayList, ITEMS_IN_ARRAY(displayList) -1, 0, 0);   //ITEMS_IN_ARRAY(displayList), 0, 0);
	schedulerScreen = new ScreenScheduler(displayList, MaxObjectList -1, 0, 1);   //ITEMS_IN_ARRAY(displayList), 0, 0);

  schedulerScreen->setPage(0,false);
  stateDisplay = STATE_OK;
	
//  display.update();  

/*  updateWindow(0, 0, display.width(), display.height(), false);  
  while (GetState() != STATE_OK) {
    updateWindow(0, 0, display.width(), display.height(), false);
  }*/
}

//****************************************************************************************************************************
void VarioScreen::CreateObjectDisplay(int8_t ObjectDisplayTypeID, VarioScreenObject* object, int8_t page, int8_t multiDisplayID, boolean actif) { 
//****************************************************************************************************************************

  MaxObjectList++;
	
#ifdef SCREEN_DEBUG
	SerialPort.print("MaxObjectList : ");	
	SerialPort.println(MaxObjectList);	
#endif //SCREEN_DEBUG
	
//  displayList = (ScreenSchedulerObject*) realloc (displayList, MaxObjectList * sizeof(ScreenSchedulerObject ) );
	
	displayList[MaxObjectList-1].object				 				= object;
	displayList[MaxObjectList-1].page   							= page;
	displayList[MaxObjectList-1].multiDisplayID 			= multiDisplayID;
	displayList[MaxObjectList-1].ObjectDisplayTypeID	= ObjectDisplayTypeID;
	displayList[MaxObjectList-1].actif  							= actif;
	
	
}

TaskHandle_t taskDisplay;

//****************************************************************************************************************************
void genericTask( void * parameter ){
//****************************************************************************************************************************
  stateDisplay = STATE_BUSY;
#ifdef SCREEN_DEBUG
    SerialPort.print("Created task: Executing on core ");
    SerialPort.println(xPortGetCoreID());
#endif //SCREEN_DEBUG

	display.display(true); // partial update
	display.epd2.powerOff();
/*	for(int i=0;i<200;i++) {
		delay(1);
// Passes control to other tasks when called
		SysCall::yield();
	}*/
//	display.
  stateDisplay = STATE_OK;
  vTaskDelete(taskDisplay);
}

//****************************************************************************************************************************
void VarioScreen::updateScreen (void)
//****************************************************************************************************************************
{
#ifdef SCREEN_DEBUG2
	SerialPort.println("screen update");	
#endif //SCREEN_DEBUG
	
  if (stateDisplay != STATE_OK) {
#ifdef SCREEN_DEBUG2
		SerialPort.println("Task en cours");	
#endif //SCREEN_DEBUG

		if (millis() - timerShow > 1500) {
			stateDisplay = STATE_OK;
			vTaskDelete(taskDisplay);
//			display.powerOff();
			display.epd2.reset();
#ifdef SCREEN_DEBUG2
			SerialPort.println("Task reset");	
#endif //SCREEN_DEBUG

		}
	  return;
	}
	
	timerShow = millis();
	display.setFullWindow();
#ifdef SCREEN_DEBUG
	SerialPort.println("screen update : setFullWindows");	
#endif //SCREEN_DEBUG
	
//	xTaskCreate(
//							genericTask,       // Task function. 
//							"genericTask",     // String with name of task. 
//							10000,             // Stack size in words. 
//							NULL,              // Parameter passed as input of the task 
//							2,                 // Priority of the task. 
//							NULL);             // Task handle. 
	
	xTaskCreatePinnedToCore(
							genericTask,       // Task function. 
							"TaskDisplay",     // String with name of task. 
							10000,             // Stack size in words. 
							NULL,              // Parameter passed as input of the task 
							2,                 // Priority of the task.
							&taskDisplay,			 // Task handle
							1);             	 // pin task to core 1	
							
//	display.display(true); // partial update
#ifdef SCREEN_DEBUG
	SerialPort.println("screen update : create task");	
#endif //SCREEN_DEBUG

//	display.updateWindow(0, 0, display.width(), display.height(), false);*/
	//display.display(true); // partial update

}

//****************************************************************************************************************************
void VarioScreen::updateScreenNB (void)
//****************************************************************************************************************************
{
#ifdef SCREEN_DEBUG2
	SerialPort.println("screen updateNB");	
#endif //SCREEN_DEBUG
	
	display.display(true); // partial update

}

//****************************************************************************************************************************
void VarioScreen::updateData(int8_t ObjectDisplayTypeID, double data) {
//****************************************************************************************************************************
	
	#ifdef SCREEN_DEBUG
	SerialPort.println("updateData");	
#endif //SCREEN_DEBUG

	for(int i=0;i<=sizeof(displayList)/sizeof(ScreenSchedulerObject);i++) {
		
#ifdef SCREEN_DEBUG
		SerialPort.print("i = ");	
		SerialPort.println(i);	
		SerialPort.print("ObjectDisplayTypeID = ");	
		SerialPort.println(displayList[i].ObjectDisplayTypeID);			
#endif //SCREEN_DEBUG
		
		if (displayList[i].ObjectDisplayTypeID	== ObjectDisplayTypeID) {
	//		displayList[i].object->setValue(data);
			
#ifdef SCREEN_DEBUG
		  SerialPort.print("i = ");	
		  SerialPort.println(i);	
		  SerialPort.print("data = ");	
		  SerialPort.println(data);	
#endif //SCREEN_DEBUG
		
			
		}
	}
}

//****************************************************************************************************************************
void VarioScreen::clearScreen() {
//****************************************************************************************************************************
  display.clearScreen();
}

//****************************************************************************************************************************
void VarioScreen::ScreenViewInit(uint8_t Version, uint8_t Sub_Version, String Author, uint8_t Beta_Code)
//****************************************************************************************************************************
{
  char tmpbuffer[100];
	
  display.setFullWindow();
  display.firstPage();
  do
  {
// 	  display.fillScreen(ColorScreen);
//		display.clearScreen(ColorScreen);

		display.drawBitmap(0, 10, logo_gnuvario, 102, 74, ColorText); //94

		display.setFont(&FreeSansBold12pt7b);
		display.setTextColor(ColorText);
		display.setTextSize(1);

		display.setCursor(100, 30);
		display.println("Version");
		if (Beta_Code > 0) {
			sprintf(tmpbuffer," Beta %01d", Beta_Code);
  		display.setCursor(105, 50);
		  display.print(tmpbuffer);
		}
		sprintf(tmpbuffer,"%02d.%02d-", Version, Sub_Version);
		display.setCursor(105, 70);
		display.print(tmpbuffer);
		display.print(Author);
		sprintf(tmpbuffer,"%s", __DATE__);
		display.setCursor(25, 110);
		display.print(tmpbuffer);

		display.setFont(&FreeSansBold12pt7b);
		display.setTextSize(1);
		display.setCursor(20, VARIOSCREEN_TENSION_ANCHOR_Y);
		display.print("GNUVARIO-E");
  }
  while (display.nextPage());
	
//	display.powerOff();
	
	unsigned long TmplastDisplayTimestamp = millis();
	int compteur = 0;
	while (compteur < 3) {
		ButtonScheduleur.update();
		if (ButtonScheduleur.Get_StatePage() == STATE_PAGE_CALIBRATION) break;
		
		if( millis() - TmplastDisplayTimestamp > 1000 ) {

			TmplastDisplayTimestamp = millis();
			compteur++;
		
		  display.fillRect(19, 170, 180, -30, GxEPD_WHITE);

		  if ((compteur % 2) == 0) {
				display.setCursor(20, VARIOSCREEN_TENSION_ANCHOR_Y);
				display.print("GNUVARIO-E");
#ifdef SCREEN_DEBUG
				SerialPort.println("Gnuvario-E");	
#endif //SCREEN_DEBUG

			} else {
#ifdef SCREEN_DEBUG
				SerialPort.println("Startup");	
#endif //SCREEN_DEBUG

			}
			updateScreen ();
		}
	}
	
// 	display.fillScreen(ColorScreen);
	
//  display..update();
/*		display..updateWindow(0, 0, display.width(), display.height(), false);
  while (display..GetState() != STATE_OK) {
    display..updateWindow(0, 0, display.width(), display.height(), false);
  }*/	
}

//****************************************************************************************************************************
void VarioScreen::ScreenViewPage(int8_t page, boolean clear, boolean refresh)
//****************************************************************************************************************************
{
	
#ifdef SCREEN_DEBUG
	SerialPort.println("fillScreen");	
#endif //SCREEN_DEBUG

//setPage(int8_t page, boolean force = false);

  if (clear) {
	  display.setFullWindow();	
		display.clearScreen(ColorScreen);
		display.fillScreen(ColorScreen);
	}
	
#ifdef SCREEN_DEBUG
	SerialPort.println("setTextColor");	
#endif //SCREEN_DEBUG
	display.clearScreen(ColorScreen);
  display.fillScreen(ColorScreen);
  display.setTextColor(GxEPD_BLACK);//display.setTextColor(GxEPD_BLACK);
	
#ifdef SCREEN_DEBUG
	SerialPort.println("update");	
#endif //SCREEN_DEBUG	

	if (refresh) {
		altiDigit->update(true);
		varioDigit->update(true);
		speedDigit->update(true);
		ratioDigit->update(true);
		trendDigit->update(true);
		infoLevel->update(true);
		volLevel->update(true);
	
		recordIndicator->update(true);
		recordIndicator->update(true);
		trendLevel->update(true);
		batLevel->update(true);
		satLevel->update(true);
		screenTime->update(true);
		screenElapsedTime->update(true);

		fixgpsinfo->update(true);
		btinfo->update(true);
	}
	else {		
		altiDigit->setValue(9999);
		varioDigit->setValue(0.0);
		speedDigit->setValue(0);
		ratioDigit->setValue(0);
		trendDigit->setValue(0);
		infoLevel->set(INFO_NONE);
		volLevel->setVolume(10);
	
		recordIndicator->setActifSCAN();
		recordIndicator->stateRECORD();
		trendLevel->stateTREND(0);
		batLevel->setVoltage(0);
		satLevel->setSatelliteCount(0);

		int8_t tmptime[] = {0,0,0};
		screenTime->setTime(tmptime);

		screenElapsedTime->setBaseTime(screenTime->getTime());
		screenElapsedTime->setCurrentTime(screenTime->getTime());

		fixgpsinfo->unsetFixGps();
		btinfo->unsetBT();
	}
		
	munit->toDisplay();
	msunit->toDisplay();
	kmhunit->toDisplay();
}	

/*void VarioScreen::clearScreen(void) 
{
  eraseDisplay();	
}*/

/*void VarioScreen::beginClear(void) {
  clearingStep = 0;
}

bool VarioScreen::clearStep(void) {

  /* check if clear is needed *
  if( clearingStep == LCDHEIGHT ) {
    return false;
  }

  /* clear one line *

  /* next *
  clearingStep++;
  return true;
}*/


//****************************************************************************************************************************
void VarioScreen::ScreenViewStat(void)
//****************************************************************************************************************************
{
	
  display.setFullWindow();
  display.clearScreen(ColorScreen);
  display.fillScreen(ColorScreen);
  display.firstPage();
  do
  {
// 	  display.fillScreen(ColorScreen);
//		display.clearScreen(ColorScreen);

		ScreenViewStatPage(0);
  }
  while (display.nextPage());
	
//	display.powerOff();
	
/*	unsigned long TmplastDisplayTimestamp = millis();
	int compteur = 0;
	while (compteur < 3) {
		if( millis() - TmplastDisplayTimestamp > 1000 ) {

			TmplastDisplayTimestamp = millis();
			compteur++;
		
		}
	}

/*  altiDigit.setValue(flystat.GetAlti());
  altiDigit.update();
  altiDigit.display();
  varioDigit.setValue(flystat.GetVarioMin());
  varioDigit.update();
  varioDigit.display();
  double tmpspeed = flystat.GetSpeed();
  if (tmpspeed > 99) tmpspeed = 99;
  speedDigit.setValue(tmpspeed);
  speedDigit.update();
  speedDigit.display();
  kmhunit.update();
  kmhunit.display();
  msunit.update();
  msunit.display();
  munit.update();  
  munit.display();*/
}

//****************************************************************************************************************************
void VarioScreen::ScreenViewStatPage(int PageStat)
//****************************************************************************************************************************
{

  char tmpbuffer[100];

	display.setFont(&FreeSansBold12pt7b);
	display.setTextColor(ColorText);
	display.setTextSize(1);

	display.setCursor(20, 30);
	display.print("STATISTIQUE");

	uint8_t tmpDate[3];
	int8_t  tmpTime[3];
	flystat.GetDate(tmpDate);
		
#ifdef SCREEN_DEBUG
    for (int i=0; i< 3; i++) {
		  SerialPort.print(tmpDate[i]);
      SerialPort.print(" ");
    }
    SerialPort.println("");		
#endif //SCREEN_DEBUG
		
		sprintf(tmpbuffer,"Date : %02d.%02d.%02d", tmpDate[0],tmpDate[1],tmpDate[2]);
		display.setCursor(1, 60);
		display.print(tmpbuffer);

#ifdef SCREEN_DEBUG
 		  SerialPort.println(tmpbuffer);
#endif //SCREEN_DEBUG

		flystat.GetTime(tmpTime);
		sprintf(tmpbuffer,"heure : %02d:%02d",tmpTime[2],tmpTime[1]); 
		display.setCursor(1, 80);
		display.print(tmpbuffer);

#ifdef SCREEN_DEBUG
 		  SerialPort.println(tmpbuffer);
#endif //SCREEN_DEBUG

		flystat.GetDuration(tmpTime);	
		sprintf(tmpbuffer,"duree : %02d:%02d",tmpTime[2],tmpTime[1]); 
		display.setCursor(1, 100);
		display.print(tmpbuffer);

#ifdef SCREEN_DEBUG
 		  SerialPort.println(tmpbuffer);
#endif //SCREEN_DEBUG

    double tmpAlti = flystat.GetAlti();
		sprintf(tmpbuffer,"Alti Max : %.0f",tmpAlti); 
		display.setCursor(1, 120);
		display.print(tmpbuffer);
	 
   double tmpVarioMin = flystat.GetVarioMin();
	 sprintf(tmpbuffer,"Vario Min : %.02f",tmpVarioMin); 
	 display.setCursor(1, 140);
	 display.print(tmpbuffer);

   double tmpVarioMax = flystat.GetVarioMax();
	 sprintf(tmpbuffer,"Vario Max : %.02f",tmpVarioMax); 
	 display.setCursor(1, 160);
	 display.print(tmpbuffer);
	 
   double tmpSpeed = flystat.GetSpeed();
	 sprintf(tmpbuffer,"Vitesse : %.0f",tmpSpeed); //%02d.%02d.%02d", tmpDate[0],tmpDate[1],tmpDate[2]);
	 display.setCursor(1, 180);
	 display.print(tmpbuffer);
}

//****************************************************************************************************************************
void VarioScreen::ScreenViewWifi(String SSID, String IP)
//****************************************************************************************************************************
{
  char tmpbuffer[100];
	
	if ((SSID == "") && (IP == "")) {
		display.setFullWindow();
		display.firstPage();
		do
		{

			display.setFont(&FreeSansBold12pt7b);
			display.setTextColor(ColorText);
			display.setTextSize(2);

			display.setCursor(40, 40);
			display.print("WIFI");

			display.setTextSize(1);
			display.setCursor(5, 80);
			display.print("Connection ...");
		}
		while (display.nextPage());
		
#ifdef SCREEN_DEBUG
		SerialPort.println("ScreenViewWifi : Connecting");	
#endif //SCREEN_DEBUG


	} else if ((SSID != "") && (IP != "")) {
		display.setCursor(5, 120);
		display.print("Connection a ");

		display.setCursor(5, 140);
		display.print(SSID);

		display.setCursor(5, 160);
		display.print(IP);
		
#ifdef SCREEN_DEBUG
		SerialPort.println("ScreenViewWifi : Connected");	
#endif //SCREEN_DEBUG

		updateScreen ();
	} else {
		display.setCursor(5, 180);
		display.print("START");
		
#ifdef SCREEN_DEBUG
		SerialPort.println("ScreenViewWifi : start");	
#endif //SCREEN_DEBUG

		updateScreen ();
		
	}
}

//****************************************************************************************************************************
void VarioScreen::ScreenViewReboot(String message)
//****************************************************************************************************************************
{
  char tmpbuffer[100];
	
  display.setFullWindow();
  display.firstPage();
  do
  {
// 	  display.fillScreen(ColorScreen);
//		display.clearScreen(ColorScreen);

		display.drawBitmap(0, 10, logo_gnuvario, 102, 74, ColorText); //94

		display.setFont(&FreeSansBold12pt7b);
		display.setTextColor(ColorText);
		display.setTextSize(1);

		display.setCursor(95, 80);
		display.print("Vario-E");

		display.setFont(&FreeSansBold12pt7b);
		display.setTextSize(1);
		display.setCursor(20, 140);
		if (message == "")
			display.print("Redemarrage");
		else 
			display.print(message);
			
 		display.setCursor(40, 170);
		display.print("en cours");
  }
  while (display.nextPage());
}

//****************************************************************************************************************************
void VarioScreen::ScreenViewMessage(String message, int delai)
//****************************************************************************************************************************
{
  char tmpbuffer[100];
	
  display.setFullWindow();
  display.firstPage();
  do
  {
// 	  display.fillScreen(ColorScreen);
//		display.clearScreen(ColorScreen);

		display.drawBitmap(0, 10, logo_gnuvario, 102, 74, ColorText); //94

		display.setFont(&FreeSansBold12pt7b);
		display.setTextColor(ColorText);
		display.setTextSize(1);

/*		display.setCursor(100, 30);
		display.println("Version");
		if (Beta_Code > 0) {
			sprintf(tmpbuffer," Beta %01d", Beta_Code);
  		display.setCursor(105, 50);
		  display.print(tmpbuffer);
		}
		sprintf(tmpbuffer,"%02d.%02d-", Version, Sub_Version);
		display.setCursor(105, 70);
		display.print(tmpbuffer);
		display.print(Author);
		sprintf(tmpbuffer,"%s", __DATE__);
		display.setCursor(25, 110);
		display.print(tmpbuffer);*/

		display.setCursor(20, 110);
		display.print("GnueVario-E");

		display.setFont(&FreeSansBold12pt7b);
		display.setTextSize(1);
		
		
		int16_t tbx, tby; uint16_t tbw, tbh; // boundary box window
		display.getTextBounds(message, 0, 0, &tbx, &tby, &tbw, &tbh); // it works for origin 0, 0, fortunately (negative tby!)
  // center bounding box by transposition of origin:
		uint16_t x = ((display.width() - tbw) / 2) - tbx;
		uint16_t y = ((display.height() - tbh) / 2) - tby;
    display.setCursor(x, VARIOSCREEN_TENSION_ANCHOR_Y); // set the postition to start printing text
		display.print(message);
  }
  while (display.nextPage());
	
//	display.powerOff();
	
	delay(500);
	unsigned long TmplastDisplayTimestamp = millis();
	int compteur = 0;
	
	int16_t tbx, tby; uint16_t tbw, tbh; // boundary box window
	display.getTextBounds(message, 0, 0, &tbx, &tby, &tbw, &tbh); // it works for origin 0, 0, fortunately (negative tby!)
// center bounding box by transposition of origin:
	uint16_t x = ((display.width() - tbw) / 2) - tbx;
	uint16_t y = ((display.height() - tbh) / 2) - tby;
	
	while (compteur < delai) {
		ButtonScheduleur.update();
		
		if( millis() - TmplastDisplayTimestamp > 1000 ) {

			TmplastDisplayTimestamp = millis();
			compteur++;
		
		  display.fillRect(19, 170, 180, -30, GxEPD_WHITE);

		  if ((compteur % 2) == 0) {
				display.setCursor(x, VARIOSCREEN_TENSION_ANCHOR_Y);
				display.print(message);
#ifdef SCREEN_DEBUG
				SerialPort.println(message);	
#endif //SCREEN_DEBUG

			} else {
#ifdef SCREEN_DEBUG
//				SerialPort.println("");	
#endif //SCREEN_DEBUG

			}
			updateScreen ();
		}
	}
}

const unsigned char volume75_1_icons[] = { 
// '750px-Speaker_Icon_1', 75x75px
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff,
0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x3f, 0xff, 0xff,
0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff,
0xe0, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0x80, 0x3f, 0xff, 0xff, 0xff, 0xe0,
0xff, 0xff, 0xff, 0xff, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x3f,
0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff,
0xff, 0xf8, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x3f, 0xff, 0xff,
0xff, 0xe0, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0x80,
0x00, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xe0,
0xff, 0xff, 0xfe, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x3e,
0x7f, 0xff, 0xff, 0xe0, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x3f, 0xff, 0xff, 0xe0, 0xf0, 0x00,
0x00, 0x00, 0x00, 0x38, 0x1f, 0xff, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x1f, 0xff,
0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x0f, 0xff, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00,
0x00, 0x3e, 0x0f, 0xff, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x07, 0xff, 0xff, 0xe0,
0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x07, 0xff, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f,
0x07, 0xff, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x07, 0xff, 0xff, 0xe0, 0xe0, 0x00,
0x00, 0x00, 0x00, 0x3f, 0x03, 0xff, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x03, 0xff,
0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x83, 0xff, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00,
0x00, 0x3f, 0x83, 0xff, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x83, 0xff, 0xff, 0xe0,
0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x03, 0xff, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f,
0x03, 0xff, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x07, 0xff, 0xff, 0xe0, 0xe0, 0x00,
0x00, 0x00, 0x00, 0x3f, 0x07, 0xff, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x07, 0xff,
0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x07, 0xff, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00,
0x00, 0x3e, 0x0f, 0xff, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x0f, 0xff, 0xff, 0xe0,
0xe0, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x1f, 0xff, 0xff, 0xe0, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x3c,
0x1f, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x3c, 0x3f, 0xff, 0xff, 0xe0, 0xff, 0xff,
0xfc, 0x00, 0x00, 0x3e, 0x7f, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x3f, 0xff, 0xff,
0xff, 0xe0, 0xff, 0xff, 0xff, 0x80, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xc0,
0x00, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xe0,
0xff, 0xff, 0xff, 0xf0, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x3f,
0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff,
0xff, 0xff, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0x80, 0x3f, 0xff, 0xff,
0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff,
0xe0, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x3f, 0xff, 0xff, 0xff, 0xe0,
0xff, 0xff, 0xff, 0xff, 0xf8, 0x7f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0
};

const unsigned char volume75_2_icons[] = { 
// '750px-Speaker_Icon_2', 75x75px
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff,
0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x3f, 0xff, 0xff,
0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff,
0xe0, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0x80, 0x3f, 0xff, 0xff, 0xff, 0xe0,
0xff, 0xff, 0xff, 0xff, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x3f,
0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x3f, 0xfc, 0x7f, 0xff, 0xe0, 0xff, 0xff,
0xff, 0xf8, 0x00, 0x3f, 0xf8, 0x3f, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x3f, 0xf0, 0x3f,
0xff, 0xe0, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x3f, 0xf8, 0x1f, 0xff, 0xe0, 0xff, 0xff, 0xff, 0x80,
0x00, 0x3f, 0xf8, 0x1f, 0xff, 0xe0, 0xff, 0xff, 0xff, 0x00, 0x00, 0x3f, 0xfc, 0x0f, 0xff, 0xe0,
0xff, 0xff, 0xfe, 0x00, 0x00, 0x3f, 0xfe, 0x0f, 0xff, 0xe0, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x3e,
0x7e, 0x07, 0xff, 0xe0, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x3f, 0x07, 0xff, 0xe0, 0xf0, 0x00,
0x00, 0x00, 0x00, 0x38, 0x1f, 0x03, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x0f, 0x03,
0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x0f, 0x83, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00,
0x00, 0x3e, 0x0f, 0x81, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x07, 0x81, 0xff, 0xe0,
0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x07, 0xc1, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f,
0x07, 0xc1, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x07, 0xc1, 0xff, 0xe0, 0xe0, 0x00,
0x00, 0x00, 0x00, 0x3f, 0x03, 0xc1, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x03, 0xc0,
0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x83, 0xc0, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00,
0x00, 0x3f, 0x83, 0xc0, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x83, 0xc0, 0xff, 0xe0,
0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x03, 0xc0, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f,
0x03, 0xc1, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x07, 0xc1, 0xff, 0xe0, 0xe0, 0x00,
0x00, 0x00, 0x00, 0x3f, 0x07, 0xc1, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x07, 0xc1,
0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x07, 0x81, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00,
0x00, 0x3e, 0x0f, 0x81, 0xff, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x0f, 0x83, 0xff, 0xe0,
0xe0, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x1f, 0x03, 0xff, 0xe0, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x3c,
0x1f, 0x03, 0xff, 0xe0, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x3c, 0x3f, 0x07, 0xff, 0xe0, 0xff, 0xff,
0xfc, 0x00, 0x00, 0x3e, 0x7e, 0x07, 0xff, 0xe0, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x3f, 0xfe, 0x0f,
0xff, 0xe0, 0xff, 0xff, 0xff, 0x80, 0x00, 0x3f, 0xfc, 0x0f, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xc0,
0x00, 0x3f, 0xf8, 0x1f, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x3f, 0xf8, 0x1f, 0xff, 0xe0,
0xff, 0xff, 0xff, 0xf0, 0x00, 0x3f, 0xf8, 0x3f, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x3f,
0xf8, 0x7f, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x3f, 0xfc, 0xff, 0xff, 0xe0, 0xff, 0xff,
0xff, 0xff, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0x80, 0x3f, 0xff, 0xff,
0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff,
0xe0, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x3f, 0xff, 0xff, 0xff, 0xe0,
0xff, 0xff, 0xff, 0xff, 0xf8, 0x7f, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0
};

const unsigned char volume75_3_icons[] = { 
 // 'basic1-095_volume_loud-32'
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x03, 0x80, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x0f,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x1f, 0xc0, 0x00, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xc0, 0x00, 0x0f, 0xc0, 0x00,
0x00, 0x00, 0x00, 0x00, 0xff, 0xc0, 0x00, 0x0f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xc0,
0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xc0, 0x03, 0x83, 0xf0, 0x00, 0x00, 0x00,
0x00, 0x07, 0xff, 0xc0, 0x07, 0xc3, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xc0, 0x0f, 0xc1,
0xf8, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xc0, 0x07, 0xe1, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x7f,
0xff, 0xc0, 0x07, 0xe0, 0xfc, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xc0, 0x03, 0xf0, 0xfc, 0x00,
0x00, 0x00, 0x01, 0xff, 0xff, 0xc0, 0x01, 0xf0, 0x7c, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xc1,
0x81, 0xf8, 0x7e, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xc3, 0xc0, 0xf8, 0x3e, 0x00, 0x0f, 0xff,
0xff, 0xff, 0xff, 0xc7, 0xe0, 0xfc, 0x3e, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xc3, 0xf0, 0xfc,
0x3f, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xc3, 0xf0, 0x7c, 0x3f, 0x00, 0x1f, 0xff, 0xff, 0xff,
0xff, 0xc1, 0xf0, 0x7e, 0x1f, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xc1, 0xf8, 0x7e, 0x1f, 0x00,
0x1f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0xf8, 0x3e, 0x1f, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xc0,
0xf8, 0x3e, 0x1f, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0xf8, 0x3e, 0x1f, 0x80, 0x1f, 0xff,
0xff, 0xff, 0xff, 0xc0, 0xfc, 0x3e, 0x1f, 0x80, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0xfc, 0x3f,
0x1f, 0x80, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x7c, 0x3f, 0x1f, 0x80, 0x1f, 0xff, 0xff, 0xff,
0xff, 0xc0, 0x7c, 0x3f, 0x1f, 0x80, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x7c, 0x3f, 0x1f, 0x80,
0x1f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0xfc, 0x3f, 0x1f, 0x80, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xc0,
0xfc, 0x3e, 0x1f, 0x80, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0xf8, 0x3e, 0x1f, 0x80, 0x1f, 0xff,
0xff, 0xff, 0xff, 0xc0, 0xf8, 0x3e, 0x1f, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xc1, 0xf8, 0x3e,
0x1f, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xc1, 0xf8, 0x7e, 0x1f, 0x00, 0x1f, 0xff, 0xff, 0xff,
0xff, 0xc1, 0xf0, 0x7e, 0x1f, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xc3, 0xf0, 0x7c, 0x3f, 0x00,
0x1f, 0xff, 0xff, 0xff, 0xff, 0xc3, 0xe0, 0xfc, 0x3f, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xc3,
0xe0, 0xfc, 0x3e, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc3, 0xc0, 0xf8, 0x7e, 0x00, 0x00, 0x00,
0x03, 0xff, 0xff, 0xc1, 0x81, 0xf8, 0x7e, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xc0, 0x01, 0xf0,
0x7c, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xc0, 0x03, 0xf0, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x3f,
0xff, 0xc0, 0x07, 0xe0, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xc0, 0x07, 0xe1, 0xf8, 0x00,
0x00, 0x00, 0x00, 0x0f, 0xff, 0xc0, 0x07, 0xc1, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xc0,
0x07, 0x83, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xc0, 0x03, 0x03, 0xf0, 0x00, 0x00, 0x00,
0x00, 0x00, 0xff, 0xc0, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xc0, 0x00, 0x0f,
0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xc0, 0x00, 0x1f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
0x1f, 0xc0, 0x00, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0x1f, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char volume75_4_icons[] = { 
/// '750px-Speaker_Icon_4', 76x76px
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff,
0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff,
0xf8, 0x1f, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x1f, 0xff, 0xff, 0xff, 0xf0,
0xff, 0xff, 0xff, 0xff, 0xe0, 0x1f, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x80, 0x1f,
0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff,
0xff, 0xfe, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x1f, 0xff, 0xff,
0xff, 0xf0, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xf0,
0x00, 0x1f, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xf0,
0xff, 0xff, 0xff, 0x80, 0x00, 0x1f, 0xbf, 0xff, 0xfd, 0xf0, 0xff, 0xff, 0xff, 0x00, 0x00, 0x1f,
0x0f, 0xff, 0xf0, 0xf0, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x1e, 0x07, 0xff, 0xe0, 0x70, 0xfc, 0x00,
0x00, 0x00, 0x00, 0x1e, 0x07, 0xff, 0xe0, 0x70, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x03, 0xff,
0xc0, 0xf0, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x81, 0xff, 0x81, 0xf0, 0xf8, 0x00, 0x00, 0x00,
0x00, 0x1f, 0x80, 0xff, 0x01, 0xf0, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xc0, 0x7e, 0x03, 0xf0,
0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xe0, 0x3c, 0x07, 0xf0, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f,
0xf0, 0x1c, 0x0f, 0xf0, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x18, 0x1f, 0xf0, 0xf8, 0x00,
0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 0x3f, 0xf0, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfe, 0x00,
0x7f, 0xf0, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfe, 0x00, 0x7f, 0xf0, 0xf8, 0x00, 0x00, 0x00,
0x00, 0x1f, 0xff, 0x00, 0xff, 0xf0, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0x00, 0xff, 0xf0,
0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0x00, 0xff, 0xf0, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f,
0xfe, 0x00, 0x7f, 0xf0, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 0x3f, 0xf0, 0xf8, 0x00,
0x00, 0x00, 0x00, 0x1f, 0xf8, 0x10, 0x1f, 0xf0, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf0, 0x18,
0x0f, 0xf0, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xe0, 0x3c, 0x07, 0xf0, 0xf8, 0x00, 0x00, 0x00,
0x00, 0x1f, 0xc0, 0x7e, 0x03, 0xf0, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xc0, 0xff, 0x03, 0xf0,
0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x81, 0xff, 0x81, 0xf0, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f,
0x03, 0xff, 0xc0, 0xf0, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x07, 0xff, 0xc0, 0x70, 0xff, 0xff,
0xfc, 0x00, 0x00, 0x1e, 0x07, 0xff, 0xe0, 0x70, 0xff, 0xff, 0xff, 0x00, 0x00, 0x1e, 0x0f, 0xff,
0xf0, 0xf0, 0xff, 0xff, 0xff, 0x80, 0x00, 0x1f, 0x9f, 0xff, 0xf9, 0xf0, 0xff, 0xff, 0xff, 0xc0,
0x00, 0x1f, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xf0,
0xff, 0xff, 0xff, 0xf0, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x1f,
0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff,
0xff, 0xff, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x80, 0x1f, 0xff, 0xff,
0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x1f, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff,
0xe0, 0x1f, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x1f, 0xff, 0xff, 0xff, 0xf0,
0xff, 0xff, 0xff, 0xff, 0xf8, 0x1f, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x3f,
0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0
};


//****************************************************************************************************************************
void VarioScreen::ScreenViewSound(int volume)
//****************************************************************************************************************************
{
	
/*  display.setFullWindow();
  display.firstPage();
  do
  {*/
// 	  display.fillScreen(ColorScreen);
//		display.clearScreen(ColorScreen);

#ifdef SCREEN_DEBUG
		SerialPort.println("Show : VolLevel");
#endif //SCREEN_DEBUG

#ifdef SCREEN_DEBUG
		SerialPort.print("Volume : ");
		SerialPort.println(volume);
#endif //SCREEN_DEBUG
  
		display.fillRect(60, 30, 60+75, 30+75, GxEPD_WHITE);
		
		if (volume == 0) display.drawInvertedBitmap(60, 30, volume75_4_icons, 75, 75, GxEPD_BLACK);
		else if (volume < 5) display.drawInvertedBitmap(60, 30, volume75_1_icons, 75, 75, GxEPD_BLACK);
		else if (volume < 8) display.drawInvertedBitmap(60, 30, volume75_2_icons, 75, 75, GxEPD_BLACK);
		else  display.drawBitmap(60, 30, volume75_3_icons, 75, 75, GxEPD_BLACK);

		display.fillRect(20, 117, 145, 55, GxEPD_WHITE);

		display.setFont(&FreeSansBold12pt7b);
		display.setTextColor(ColorText);
		display.setTextSize(2);

		if (volume == 10) display.setCursor(65, 160);
		else 							display.setCursor(65+12, 160);
		display.print(volume);
		
		if (ButtonScheduleur.Get_StatePage() == STATE_PAGE_CONFIG_SOUND) {
			display.drawRect(55, 120, 80, 50, GxEPD_BLACK);
			display.drawRect(54, 119, 80, 50, GxEPD_BLACK);
			display.drawRect(53, 118, 80, 50, GxEPD_BLACK);
			
			display.fillTriangle(140,130,160,145,140,160,GxEPD_BLACK);
			display.fillTriangle(45,130,25,145,45,160,GxEPD_BLACK);
		}
		
 /* }
  while (display.nextPage());*/
}

/************************/
/* The screen scheduler */
/************************/

//****************************************************************************************************************************
//****************************************************************************************************************************
//									SCREENSCHEDULER
//****************************************************************************************************************************
//****************************************************************************************************************************

//#define TIMER_DISPLAY

#ifdef TIMER_DISPLAY

hw_timer_t * timerScreenScheduler = NULL;
portMUX_TYPE timerMuxScreenScheduler = portMUX_INITIALIZER_UNLOCKED;

//****************************************************************************************************************************
void IRAM_ATTR onTimerScreenScheduler() {
//****************************************************************************************************************************
  portENTER_CRITICAL_ISR(&timerMuxScreenScheduler);

	if (millis() - oldtime >= GnuSettings.VARIOMETER_MULTIDISPLAY_DURATION)
	{
		oldtime  = millis();
		stateMulti = 1 - stateMulti;

#ifdef SCREEN_DEBUG
		led1stat = 1 - led1stat;
//		digitalWrite(pinLED, led1stat);   // turn the LED on or off
#endif //SCREEN_DEBUG
	}

//  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMuxScreenScheduler);
}

#endif //TIMER_DISPLAY

//****************************************************************************************************************************
ScreenScheduler::ScreenScheduler(ScreenSchedulerObject* displayList, uint8_t objectCount, int8_t startPage, int8_t endPage)
: displayList(displayList), objectCount(objectCount), pos(0), currentPage(startPage), endPage(endPage) {
//****************************************************************************************************************************
	
#ifdef SCREEN_DEBUG
/*	pinMode(pinLED, OUTPUT);
  digitalWrite(pinLED, LOW); */
	led1stat    = 0;  
#endif //SCREEN_DEBUG
	
	oldtime     = 0;
	stateMulti  = 0;

#ifdef TIMER_DISPLAY
	
	timerScreenScheduler = timerBegin(1, 80, true);
	timerAttachInterrupt(timerScreenScheduler, &onTimerScreenScheduler, true);
	timerAlarmWrite(timerScreenScheduler, 10000, true);  //100Hz
	timerAlarmEnable(timerScreenScheduler);
#endif //TIMER_DISPLAY
}
		 
//****************************************************************************************************************************
void ScreenScheduler::displayStep(void) {
//****************************************************************************************************************************

	if (currentPage == endPage+1) return;

#ifndef TIMER_DISPLAY

	if (millis() - oldtime >= GnuSettings.VARIOMETER_MULTIDISPLAY_DURATION)
	{
		oldtime  = millis();
		stateMulti = 1 - stateMulti;

#ifdef SCREEN_DEBUG
		led1stat = 1 - led1stat;
//		digitalWrite(pinLED, led1stat);   // turn the LED on or off
#endif //SCREEN_DEBUG
	}

#endif //TIMER_DISPLAY

  /* next try to find something to display */
  /* for the current page                  */
#ifdef SCREEN_DEBUG
  SerialPort.print("displaystep - objectCount  : ");
  SerialPort.println(objectCount);
#endif //SCREEN_DEBUG

  display.setFullWindow();
 	
	if (millis() - oldtimeAllDisplay >= 30000)	{
		oldtimeAllDisplay  = millis();	
		ShowDisplayAll = true;
//		display.fillRect(0, 0, display.width(), display.height(), GxEPD_WHITE);
		
#ifdef SCREEN_DEBUG
		SerialPort.println("displaystep - showDisplayAll");
#endif //SCREEN_DEBUG
		
	}

	uint8_t n = 0;
	while( n <= objectCount ) {
/*   if( displayList[pos].page == currentPage && displayList[pos].actif == true && displayList[pos].object->update() ) {
	return;
}*/
#ifdef SCREEN_DEBUG
  SerialPort.print("displaylist number : ");
  SerialPort.println(n);
  SerialPort.print("displaylist / Page : ");
//  SerialPort.println(displayList[pos].page);
  SerialPort.print(displayList[n].page);
  SerialPort.print(" - Pageactive : ");
  SerialPort.println(currentPage);
	
#endif //SCREEN_DEBUG

/*	if( displayList[pos].page == currentPage && displayList[pos].actif == true) {
		ScreenDigit* tmppointeur = (ScreenDigit*)displayList[pos].object;
		tmppointeur->update(); 
	}*/

  currentMultiDisplay = 1 + stateMulti;
	
#ifdef SCREEN_DEBUG
  SerialPort.print("Current Multidisplay : ");
  SerialPort.println(currentMultiDisplay);
#endif //SCREEN_DEBUG

	if (ShowDisplayAll && displayList[n].page == currentPage && displayList[n].actif == true ) {
		displayList[n].object->reset();
		displayList[n].object->update(true);
#ifdef SCREEN_DEBUG
		SerialPort.println("displaystep - reset");
#endif //SCREEN_DEBUG
  }
	else if ( displayList[n].page == currentPage && displayList[n].actif == true && (displayList[n].multiDisplayID == 0 || displayList[n].multiDisplayID == currentMultiDisplay))	displayList[n].object->update();

/* next */
		pos++;
		if( pos == objectCount) {
			pos = 0;
		}
		n++;
	}

  if (ShowDisplayAll == true) {
		ShowDisplayAll = false;
//		display.fillRect(0, 0, display.width(), display.height(), GxEPD_WHITE);
		//.clearScreen();
	}
	
#ifdef SCREEN_DEBUG
  SerialPort.println("displayStep : Display");
#endif //SCREEN_DEBUG

/*  display.setCursor(10, 150);
  display.print("HelloWorld");*/

//    display.displayWindow(box_x, box_y, box_w, box_h);

//  display.display(true); // partial update
	
}

//****************************************************************************************************************************
void ScreenScheduler::displayAll(void) {
//****************************************************************************************************************************
  IsDisplayAll = false;
}

//****************************************************************************************************************************
int8_t ScreenScheduler::getPage(void) {
//****************************************************************************************************************************

  return currentPage;
}

//****************************************************************************************************************************
int8_t ScreenScheduler::getMaxPage(void) {
//****************************************************************************************************************************

  return endPage;
}

bool displayStat = true;

//****************************************************************************************************************************
void ScreenScheduler::setPage(int8_t page, boolean forceUpdate)  {
//****************************************************************************************************************************

  /* check if page change is needed */
  if ((forceUpdate == false) && ( page == currentPage )) return;

  /* set the new page */
  currentPage = page;

  /* screen need to by cleared */
//  display.clearScreen();
 // display.eraseDisplay();
  display.fillRect(0, 0, display.width(), display.height(), GxEPD_WHITE);

	if (currentPage == endPage+1) {
		displayStat = true;
		screen.ScreenViewSound(toneHAL.getVolume());
	} else if ((currentPage == endPage+2) && (displayStat)) {
		displayStat = false;
		screen.ScreenViewStatPage(0);
		screen.updateScreen ();

	} else {
		/* all the page object need to be redisplayed */
		/* but no problem to reset all the objects */
		displayStat = true;
		for(uint8_t i = 0; i<objectCount; i++) {
			displayList[i].object->reset();
		}
	}
}

//****************************************************************************************************************************
void ScreenScheduler::nextPage(void) {
//****************************************************************************************************************************
  
  uint8_t newPage = currentPage + 1;
  if( newPage > endPage + 2 ) {
    newPage = 0;
  }

  setPage(newPage);
}
 
//****************************************************************************************************************************
void ScreenScheduler::previousPage(void) {
//****************************************************************************************************************************
  
  int8_t newPage = currentPage - 1;
	
  if( newPage < 0 ) {
    newPage = endPage+2;
  }

  setPage(newPage);
} 

//****************************************************************************************************************************
void ScreenScheduler::enableShow(void) {
//****************************************************************************************************************************
	oldtime = millis();
#ifdef TIMER_DISPLAY
  timerAlarmEnable(timerScreenScheduler);
#endif //TIMER_DISPLAY
} 

void ScreenScheduler::disableShow(void) {
#ifdef TIMER_DISPLAY
  timerAlarmDisable(timerScreenScheduler);
#endif //TIMERDISPLAY
} 

//****************************************************************************************************************************
//****************************************************************************************************************************
//							MULTIDISPLAY
//****************************************************************************************************************************
//****************************************************************************************************************************

//****************************************************************************************************************************
void MultiDisplay::displayStep(void) {
//****************************************************************************************************************************

#ifdef SCREEN_DEBUG
	SerialPort.println("display step - Multidisplay");	
#endif //SCREEN_DEBUG

  if (lastFreqUpdate == 0) lastFreqUpdate = millis();
  unsigned long FreqDuration = millis() - lastFreqUpdate;
  if( FreqDuration > 1000 ) {
    lastFreqUpdate = millis();

    for(uint8_t i = 0; i<multiObjectCount; i++) {
      multiDisplayList[i].countTime--;
      if (multiDisplayList[i].countTime <= 0) {
         multiDisplayList[i].countTime = multiDisplayList[i].seconde;
         multiDisplayList[i].oldDisplayActif = multiDisplayList[i].displayActif;
         multiDisplayList[i].displayActif++;
         if (multiDisplayList[i].displayActif > multiObjectCount) multiDisplayList[i].displayActif = 0;
		 
#ifdef SCREEN_DEBUG
		 SerialPort.print("old display  : ");
         SerialPort.println(objectCount);
#endif //SCREEN_DEBUG
		 
         displayList[multiDisplayList[i].objectList[multiDisplayList[i].oldDisplayActif]].actif = false;       
         displayList[multiDisplayList[i].objectList[multiDisplayList[i].displayActif]].actif = true;  
 /*        displayList[8].actif = false;       
         displayList[9].actif = true;  */
		 
			}		 
		}
	}
}

#endif //VARIOSCREEN_SIZE