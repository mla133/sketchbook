/*********************************************************
 * TvText    whmp 2015 11 16  
 *
 * an example for MRETV
 *
 * This sketch will let your Arduino act as an external video card for text
 * only display. Being able to cut and paste to another display is handy
 * for programming.
 *
 * You can also plug the Arduino into your Big screen to leave a text message
 * that are hard to miss. A schedule or menu or leader board on a bigscreen
 * at your charity event. Put your Arduino to work!
 *
 * About Extension defines.
 *
 * Defining these in the MRETV library folder allows for extra optimisation and
 * improved performance. All examples are predefined in Arduino\libraries\MRETV\MRETVextensions.h
 *
 * for MasteRasterExtendedTransmissionVision 2
 **********************************************************/


#include <MrETV.h>
#ifdef default_extensions // make sure the extensions are set correctly
#define kludge me
#else
//#error Please define 'default_extensions' in arduino\libraries\MRETV\MRETV.extensions , See comments at start of sketch
#endif

#include "textfont.h"  // readable font

#ifdef __AVR_ATmega168__
#define asciibufsize 22*40
#else 
#define asciibufsize 25*46
#endif

uchar  asciibuf[asciibufsize];
void setup(){
#ifdef __AVR_ATmega168__
  Screen.begin(textfont,asciibuf, 22*8,40,34,52,1);
#else 
  Screen.begin(textfont,asciibuf, 25*8,46,22,20,1);
#endif
  Video.readS(); //Response from keyboard init should be 0xAA, read it here so it does not print
}

void loop() {
  uchar chin;
  while(1)if((chin=Video.readS()))Screen.render(chin);
  // render is the basic scrolling text terminal service and in not a tiny function.
  // I also hope the Arduino community will port some of their fine work into MRETV
  // this render NEEDS a rewrite
}



