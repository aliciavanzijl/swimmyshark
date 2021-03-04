#include <pic32mx.h>
#include <stdint.h>
#include "display.h"
#include "timers.h"
#include "io.h"
#include "loadgraphics.h"

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 32
#define DISPLAY_PAGES 4
#define DISPLAY_SIZE DISPLAY_WIDTH * DISPLAY_PAGES //no. of bytes in display buffer

/* GLOBAL VARIABLES */
// Buffers for text & display
char textBuffer[4][16];       			// Text buffer
uint8_t displayBuffer[DISPLAY_SIZE];    // Display buffer
uint8_t levelBuffer[DISPLAY_SIZE*2];	// Level buffer

uint8_t inputs;							//Inputs as 8 LSB

/* SPRITES */

int shark [23][2] = {
	{3, -6},
	{3, -5}, {4, -5},
	{0, -4}, {2, -4}, {3, -4}, {4, -4}, {5, -4}, {6, -4}, {7, -4},
	{1, -3}, {2, -3}, {3, -3}, {4, -3}, {5, -3}, {7, -3},
	{0, -2}, {2, -2}, {3, -2}, {4, -2}, {5, -2}, {6, -2},
	{4, -1}
};

const uint8_t hook[] = {
0x00, 0x00, 0x60, 0x80, 0x8f, 0x70, 0x00, 0x00
};

const uint8_t fish[] = {
0x00, 0x28, 0x10, 0x38, 0x3c, 0x28, 0x10, 0x00
};

const uint8_t rock[] = {
0x80, 0xc0, 0xe0, 0xf8, 0xfe, 0xf0, 0xe0, 0x80
};

int rockTest [23][2] = {
	{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0},
	{1, -1}, {2, -1}, {3, -1}, {4, -1}, {5, -1}, {6, -1},
	{2, -2}, {3, -2}, {4, -2}, {5, -2},
	{3, -3}, {4, -3},
	{3, -4}, {4, -4},
	{4, -5}
};

const uint8_t net[] = {
0x81, 0x7e, 0x55, 0xd5, 0x7e, 0x55, 0xd5, 0x7e
};

const uint8_t seaweed[] = {
0x00, 0x10, 0x62, 0xf4, 0x5c, 0x60, 0x30, 0x00
};

const uint8_t flag[] = {
0x00, 0x00, 0xff, 0x12, 0x12, 0x14, 0x18, 0x10
};

const uint8_t wave[] = {
0x04, 0x04, 0x02, 0x01, 0x07, 0x05, 0x04, 0x04
};

const uint8_t lowerLine[] = {
0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
};


int spritex;
int spritey;
int spritedirection = 1;
int spritefront;
int currentLevel;
int levelPos;
int levelObstacles[5][2];
int lifeCounter = 4;
int lives = 0x1F;

//Interval between updates of the game logic. 160 -> 60Hz, 190 -> 52Hz
#define UPDATE_INTERVAL 190 		//clock measures s *10^-4, not -3 so 160 -> 16ms

//Input masks
#define SW_1  0x80
#define SW_2  0x40
#define SW_3  0x20
#define SW_4  0x10
#define BTN_4 0x8
#define BTN_3 0x4
#define BTN_2 0x2
#define BTN_1 0x1

// Function Prototypes
void inputHandler();              		// Polls inputs
void loadGraphics();					// Puts graphics into buffer
void loadString();						// Puts string characters into text buffer
void updateLogic();						// Updates game data based on inputs
void loadLevel1();
void loadLevel2();
void loadLevel3();
int powerOf (int exp, int base);


/* Set up ISR */
void user_isr()
{
	if (IFS(0) & 0x10000)
 	{
    	IFS(0) &= ~0x10000;
		tickClk();
  	}
}

/* INIT TIMERS, DISPLAY, & IO */
void start()
{
	initTimers();
	displayInit();
	initIO();
}

int main() {
	
	start();
	
	spritex = 10;
	spritey = 16;
	currentLevel = 1;
	levelPos = 0;
	// Update LEDs to show lives
	// Port E 7-0 are the LEDs PORTE has address 0xbf886110
	volatile int* porte = (volatile int*) 0xbf886110;
	*porte = 0x001f & lives;
	
	while(1) {
		
		inputHandler();
	
		updateLogic();
		
		if(currentLevel == 1) {
			loadLevel1(levelBuffer);
		} else if (currentLevel == 2) {
			loadLevel2(levelBuffer);
		} else if (currentLevel == 3) {
			loadLevel3(levelBuffer);
		}
		
		delayms(16);		// slow it down a bit so graphics doesn't jump
		loadGraphics();
	}
	return 0;
	
}

void inputHandler() {
	
	inputs = getInputs();            //get inputs from board
	
}

void updateLogic() {
	
	// insert functions to update the logic with buttons
	if(inputs & BTN_1) {
		if(spritedirection == 1) {
			if(spritex < 120) {
				spritex++;
			}
		} else {
			spritedirection = 1;
		}
	}
	if(inputs & BTN_4) {
		if(spritedirection == -1) {
			if(spritex > 8) {
				spritex--;
			}
		} else {
			spritedirection = -1;
		}
	}
	if(inputs & BTN_3) {
		if(spritey < 23) {
			delayms(16);
			spritey++;
		}
	}
	if(inputs & BTN_2) {
		if(spritey > 8) {
			delayms(16);
			spritey--;
		}
	}
	
	// Update sprite front for collision detection
	spritefront = spritex + (7 * spritedirection);
	
	// Display Scroll
	if(spritefront >= 110 & (inputs & BTN_1)) {
		delayms(16);
		if(levelPos < 128) {
			levelPos++;
			spritex--;
		}
	}
	
	// COLLISION DETECTION
	int obj;
	for(obj = 0; obj < 5; obj++) {
		if ( ((spritey + 4) >= levelObstacles[obj][1] & (spritey + 4) <= (levelObstacles[obj][1] + 7) ) & (spritefront == (levelObstacles[obj][0] - levelPos) | spritefront == ((levelObstacles[obj][0] - levelPos)+8*spritedirection)) ) {
			// COLLISION DETECTED
			delayms(128);
			lives = lives - powerOf(lifeCounter, 2);
			spritex = spritex - 4*spritedirection;
			// Update LEDs to show lives
			// Port E 7-0 are the LEDs PORTE has address 0xbf886110
			volatile int* porte = (volatile int*) 0xbf886110;
			*porte = 0x001f & lives;
			// decrease lifeCounter
			lifeCounter--;
		}
	}
	
	// DETECT GAME OVER
	
	// DETECT HITING END OF LEVEL FLAG
	if(levelPos == 128 & spritefront >= 118) {
		delayms(128);
		spritex = 10;
		spritey = 16;
		currentLevel++;
		levelPos = 0;
	}
	
	// DETECT GAME COMPLETED
	
}

// Updates display with new graphics from displayBuffer
void loadGraphics() {
	
	clearBuffer(512, displayBuffer); 	//Clears the display buffer

	// insert functions here to put player and scene graphics in buffer
	//loadSprite(spritex, spritey, spritedirection, sharkA, displayBuffer);
	loadSprite(spritex, spritey, spritedirection, shark, displayBuffer);
	loadTiles(0, 2, 16, 128, lowerLine, displayBuffer); //load wave tiles
	loadLevel(levelPos, levelBuffer, displayBuffer);
	// add function for loading UI at bottom to show level & lives

	displayUpdate(displayBuffer); //displays new buffer
}

// Function from Labs
void loadString(int line, char *s) {
	
	int i;
	if(line < 0 || line >= 4)
		return;
	if(!s)
		return;
	
	for(i = 0; i < 16; i++) {
		if(*s) {
			textBuffer[line][i] = *s;
			s++;
		} else {
			textBuffer[line][i] = ' ';
		}
	}
}

// Level Details & Loading
void loadLevel1 () {
	
	clearBuffer(1024, levelBuffer); 	//Clears the level buffer
	
	loadTiles(0, 0, 32, 256, wave, levelBuffer); //load wave tiles
	
	// x,y,size,direction,name,buffer
	//loadObject(70, 2, 8, -1, rock, levelBuffer);
	loadObject(100, 0, 8, 1, hook, levelBuffer); 
	//loadObject(180, 2, 8, 1, rock, levelBuffer);
	
	testObject (70, 23, 1, rockTest, levelBuffer);
	testObject (180, 23, 1, rockTest, levelBuffer);
	
	levelObstacles[0][0] = 70;
	levelObstacles[0][1] = 23;
	//levelObstacles[0][1] = 2;
	levelObstacles[1][0] = 100;
	levelObstacles[1][1] = 0;
	levelObstacles[2][0] = 180;
	levelObstacles[2][1] = 23;
	//levelObstacles[2][1] = 2;
	levelObstacles[3][0] = 0;
	levelObstacles[3][1] = 0;
	levelObstacles[4][0] = 0;
	levelObstacles[4][1] = 0;
	
	// Add end flag
	loadObject(246, 2, 8, 1, flag, levelBuffer);
	
}

void loadLevel2 () {
	
	clearBuffer(1024, levelBuffer); 	//Clears the level buffer
	
	loadTiles(0, 0, 32, 256, wave, levelBuffer); //load wave tiles
	
	// x,y,size,direction,name,buffer
	loadObject(50, 2, 8, 1, rock, levelBuffer);
	loadObject(118, 2, 8, 1, rock, levelBuffer);
	loadObject(120, 0, 8, 1, net, levelBuffer);
	loadObject(200, 2, 8, -1, rock, levelBuffer);
	
	levelObstacles[0][0] = 50;
	levelObstacles[0][1] = 2;
	levelObstacles[1][0] = 118;
	levelObstacles[1][1] = 2;
	levelObstacles[2][0] = 120;
	levelObstacles[2][1] = 0;
	levelObstacles[3][0] = 200;
	levelObstacles[3][1] = 2;
	levelObstacles[4][0] = 0;
	levelObstacles[4][1] = 0;
	
	// Add end flag
	loadObject(240, 2, 8, 1, flag, levelBuffer);
	
}

void loadLevel3 () {
	
	clearBuffer(1024, levelBuffer); 	//Clears the level buffer
	
	loadTiles(0, 0, 32, 256, wave, levelBuffer); //load wave tiles
	
	// x,y,size,direction,name,buffer
	loadObject(50, 2, 8, 1, rock, levelBuffer);
	loadObject(118, 2, 8, 1, rock, levelBuffer);
	loadObject(130, 0, 8, 1, net, levelBuffer);
	loadObject(130, 1, 8, 1, net, levelBuffer);
	loadObject(200, 2, 8, -1, rock, levelBuffer);
	
	levelObstacles[0][0] = 50;
	levelObstacles[0][1] = 2;
	levelObstacles[1][0] = 118;
	levelObstacles[1][1] = 2;
	levelObstacles[2][0] = 130;
	levelObstacles[2][1] = 0;
	levelObstacles[3][0] = 130;
	levelObstacles[3][1] = 1;
	levelObstacles[4][0] = 200;
	levelObstacles[4][1] = 2;
	
	// Add end flag
	loadObject(240, 2, 8, 1, flag, levelBuffer);
	
}

// Little Helper Math Functions
int powerOf (int exp, int base) {
	
	int result = 1;
	
    while (exp != 0) {
        result *= base;
        exp--;
    }
	
	return result;
	
}


