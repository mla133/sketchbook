//<File !Start!>
// FILE: [SimpCount_GSLC.h]
// Created by GUIslice Builder version: [0.15.b005]
//
// GUIslice Builder Generated GUI Framework File
//
// For the latest guides, updates and support view:
// https://github.com/ImpulseAdventure/GUIslice
//
//<File !End!>

#ifndef _GUISLICE_GEN_H
#define _GUISLICE_GEN_H

// ------------------------------------------------
// Headers to include
// ------------------------------------------------
#include "GUIslice.h"
#include "GUIslice_drv.h"

// Include any extended elements
//<Includes !Start!>
//<Includes !End!>

// ------------------------------------------------
// Headers and Defines for fonts
// Note that font files are located within the Adafruit-GFX library folder:
// ------------------------------------------------
//<Fonts !Start!>
#if defined(DRV_DISP_TFT_ESPI)
  #error Project tab->Target Platform should be tft_espi
#endif
#include <Adafruit_GFX.h>
//<Fonts !End!>

// ------------------------------------------------
// Defines for resources
// ------------------------------------------------
//<Resources !Start!>
//<Resources !End!>

// ------------------------------------------------
// Enumerations for pages, elements, fonts, images
// ------------------------------------------------
//<Enum !Start!>
enum {E_PG_MAIN};
enum {E_BTN_DEC,E_BTN_INC,E_ELEM_TEXT1,E_ELEM_TEXT2,E_ELEM_TEXT3};
// Must use separate enum for fonts with MAX_FONT at end to use gslc_FontSet.
enum {E_AO_10X16PT7B,E_AO_5X8PT7B,MAX_FONT};
//<Enum !End!>

// ------------------------------------------------
// Instantiate the GUI
// ------------------------------------------------

// ------------------------------------------------
// Define the maximum number of elements and pages
// ------------------------------------------------
//<ElementDefines !Start!>
#define MAX_PAGE                1

#define MAX_ELEM_PG_MAIN 5 // # Elems total on page
#define MAX_ELEM_PG_MAIN_RAM MAX_ELEM_PG_MAIN // # Elems in RAM
//<ElementDefines !End!>

// ------------------------------------------------
// Create element storage
// ------------------------------------------------
gslc_tsGui                      m_gui;
gslc_tsDriver                   m_drv;
gslc_tsFont                     m_asFont[MAX_FONT];
gslc_tsPage                     m_asPage[MAX_PAGE];

//<GUI_Extra_Elements !Start!>
gslc_tsElem                     m_asPage1Elem[MAX_ELEM_PG_MAIN_RAM];
gslc_tsElemRef                  m_asPage1ElemRef[MAX_ELEM_PG_MAIN];

#define MAX_STR                 100

//<GUI_Extra_Elements !End!>

// ------------------------------------------------
// Program Globals
// ------------------------------------------------

// Element References for direct access
//<Extern_References !Start!>
extern gslc_tsElemRef* m_pElemCounter;
//<Extern_References !End!>

// Define debug message function
static int16_t DebugOut(char ch);

// ------------------------------------------------
// Callback Methods
// ------------------------------------------------
bool CbBtnCommon(void* pvGui,void *pvElemRef,gslc_teTouch eTouch,int16_t nX,int16_t nY);
bool CbCheckbox(void* pvGui, void* pvElemRef, int16_t nSelId, bool bState);
bool CbDrawScanner(void* pvGui,void* pvElemRef,gslc_teRedrawType eRedraw);
bool CbKeypad(void* pvGui, void *pvElemRef, int16_t nState, void* pvData);
bool CbListbox(void* pvGui, void* pvElemRef, int16_t nSelId);
bool CbSlidePos(void* pvGui,void* pvElemRef,int16_t nPos);
bool CbSpinner(void* pvGui, void *pvElemRef, int16_t nState, void* pvData);
bool CbTickScanner(void* pvGui,void* pvScope);

// ------------------------------------------------
// Create page elements
// ------------------------------------------------
void InitGUIslice_gen()
{
  gslc_tsElemRef* pElemRef = NULL;

  if (!gslc_Init(&m_gui,&m_drv,m_asPage,MAX_PAGE,m_asFont,MAX_FONT)) { return; }

  // ------------------------------------------------
  // Load Fonts
  // ------------------------------------------------
//<Load_Fonts !Start!>
    if (!gslc_FontSet(&m_gui,E_AO_10X16PT7B,GSLC_FONTREF_PTR,NULL,2)) { return; }
    if (!gslc_FontSet(&m_gui,E_AO_5X8PT7B,GSLC_FONTREF_PTR,NULL,1)) { return; }
//<Load_Fonts !End!>

//<InitGUI !Start!>
  gslc_PageAdd(&m_gui,E_PG_MAIN,m_asPage1Elem,MAX_ELEM_PG_MAIN_RAM,m_asPage1ElemRef,MAX_ELEM_PG_MAIN);

  // NOTE: The current page defaults to the first page added. Here we explicitly
  //       ensure that the main page is the correct page no matter the add order.
  gslc_SetPageCur(&m_gui,E_PG_MAIN);
  
  // Set Background to a flat color
  gslc_SetBkgndColor(&m_gui,GSLC_COL_BLACK);

  // -----------------------------------
  // PAGE: E_PG_MAIN
  
  
  // Create E_ELEM_TEXT1 text label
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT1,E_PG_MAIN,(gslc_tsRect){76,10,168,18},
    (char*)"Simple Counter",0,E_AO_10X16PT7B);
  
  // Create E_ELEM_TEXT2 text label
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT2,E_PG_MAIN,(gslc_tsRect){10,120,168,18},
    (char*)"Current Count:",0,E_AO_10X16PT7B);
  gslc_ElemSetTxtCol(&m_gui,pElemRef,GSLC_COL_BLUE_LT4);
  
  // Create E_ELEM_TEXT3 runtime modifiable text
  static char m_sDisplayText3[13] = "0";
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT3,E_PG_MAIN,(gslc_tsRect){170,140,144,18},
    (char*)m_sDisplayText3,13,E_AO_10X16PT7B);
  m_pElemCounter = pElemRef;
  
  // create E_BTN_INC button with text label
  pElemRef = gslc_ElemCreateBtnTxt(&m_gui,E_BTN_INC,E_PG_MAIN,
    (gslc_tsRect){0,160,80,20},(char*)"Increment",0,E_AO_5X8PT7B,&CbBtnCommon);
  
  // create E_BTN_DEC button with text label
  pElemRef = gslc_ElemCreateBtnTxt(&m_gui,E_BTN_DEC,E_PG_MAIN,
    (gslc_tsRect){0,190,80,20},(char*)"Decrement",0,E_AO_5X8PT7B,&CbBtnCommon);
//<InitGUI !End!>

//<Startup !Start!>
//<Startup !End!>

}

#endif // end _GUISLICE_GEN_H
