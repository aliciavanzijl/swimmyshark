#include <stdint.h>
#include <pic32mx.h>

void initIO() {
	
	// This init Port E bits 0-7 (8LSB) are outputs (0)
	// TRISE has address 0xbf886100
	// These bits are connected to 8 green LEDs on the Basic I/O Shield
	volatile int * trise = (volatile int*) 0xbf886100;
	*trise = *trise & 0xff00;
	//*trise =xxxxxxxx.....xxxxx & 1111 1111 0000 0000
	
	// Init Port D bits 11-5 as inputs (1)
	// Using definition in pic32mx.h
	TRISD = 0x0FE0;		// Switches 1-4 and Buttons 2-4
	TRISF = 0x0002;		// Button 1
	
}

// Function to return the status of the toggle-swiches
// SW4-SW1 is bits 11-8 of Port D
int getsw(void) {
	
	int sw = PORTD >> 8; 	// takes status from Port D [11:8] & right-shift 8
	sw &= 0xf;				// mask to select only the 4 LSB & set all others to 0
	
	return sw;
	
}

// Function to return the status of the push buttons
// BTN4-BTN2 is bits 7-5 of Port D
// BTN1 is bit 1 of Port F
int getbtns(void) {
	
	int btn = PORTD >> 4; 	// takes status from Port D [7:5] & right-shift 4
	int btn1 = PORTF >> 1;	// takes status from Port F [1:0] & right-shift 1
	btn &= 0xE;				// mask to select
	btn1 &= 0x1;			// mask to select
	btn |= btn1;			// add in btn 1 data
	
	return btn;				// returns 4 LSB containing button data
	
}

//returns switches and buttons as one set of 8 bits
uint8_t getInputs()
{
    return (getsw() << 4) | getbtns();
}