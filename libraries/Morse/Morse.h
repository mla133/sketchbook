/*
 	Morse.h - Library for flashing Morse code
	Created by:  Matt Allen, September 17, 2020
	Released into the public domain

	Based off tutorial found at:
	arduino.cc/en/Hacking/libraryTutorial
*/
#ifdef Morse_h
#define Morse_h

#include "Arduino.h"

class Morse
{
	public:
		Morse(int pin);
		void dot();
		void dash();
	private:
		int _pin;
};
#endif
