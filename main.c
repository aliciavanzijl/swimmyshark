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

int hook [11][2] = {
	{3, 0}, {4, 0},
	{2, -1}, {5, -1},
	{2, -2}, {5, -2},
	{5, -3},
	{4, -4}, {4, -5}, {4, -6}, {4, -7}
};

int fish [14][2] = {
	{3, -1},
	{2, -2}, {3, -2}, {4, -2}, {6, -2},
	{1, -3}, {3, -3}, {4, -3}, {5, -3},
	{2, -4}, {3, -4}, {4, -4}, {6, -4},
	{4, -5}
};

int rock [23][2] = {
	{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0},
	{1, -1}, {2, -1}, {3, -1}, {4, -1}, {5, -1}, {6, -1},
	{2, -2}, {3, -2}, {4, -2}, {5, -2},
	{3, -3}, {4, -3},
	{3, -4}, {4, -4},
	{4, -5}
};

int net[127][2] = {
	{0, 0}, {12, 0},
	{1, -1}, {13, -1},
	{2, -2}, {3, -2}, {4, -2}, {5, -2}, {6, -2}, {7, -2}, {8, -2}, {9, -2}, {10, -2}, {11, -2}, {12, -2}, {13, -2}, {14, -2},
	{2, -3}, {5, -3}, {8, -3}, {11, -3}, {14, -3},
	{2, -4}, {3, -4}, {4, -4}, {5, -4}, {6, -4}, {7, -4}, {8, -4}, {9, -4}, {10, -4}, {11, -4}, {12, -4}, {13, -4}, {14, -4},
	{2, -5}, {5, -5}, {8, -5}, {11, -5}, {14, -5},
	{2, -6}, {3, -6}, {4, -6}, {5, -6}, {6, -6}, {7, -6}, {8, -6}, {9, -6}, {10, -6}, {11, -6}, {12, -6}, {13, -6}, {14, -6},
	{2, -7}, {5, -7}, {8, -7}, {11, -7}, {14, -7},
	{2, -8}, {3, -8}, {4, -8}, {5, -8}, {6, -8}, {7, -8}, {8, -8}, {9, -8}, {10, -8}, {11, -8}, {12, -8}, {13, -8}, {14, -8},
	{2, -9}, {5, -9}, {8, -9}, {11, -9}, {14, -9},
	{2, -10}, {3, -10}, {4, -10}, {5, -10}, {6, -10}, {7, -10}, {8, -10}, {9, -10}, {10, -10}, {11, -10}, {12, -10}, {13, -10}, {14, -10},
	{2, -11}, {5, -11}, {8, -11}, {11, -11}, {14, -11},
	{2, -12}, {3, -12}, {4, -12}, {5, -12}, {6, -12}, {7, -12}, {8, -12}, {9, -12}, {10, -12}, {11, -12}, {12, -12}, {13, -12}, {14, -12},
	{2, -13}, {5, -13}, {8, -13}, {11, -13}, {14, -13},
	{1, -14}, {2, -14}, {3, -14}, {4, -14}, {5, -14}, {6, -14}, {7, -14}, {8, -14}, {9, -14}, {10, -14}, {11, -14}, {12, -14}, {13, -14}, 
	{0, -15}, {12, -15}
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

// GLOBAL VARIABLES FOR GAME LOGIC
int spritex;
int spritey;
int spritedirection = 1;			// 1 for default, -1 to reverse
int spritefront;					// the front end of the sprite
int currentLevel;				// stores the current level
int levelPos;						// where in the larger levelBuffer the screen showing
int levelObstacles[5][2];			// stores 5 obstacles and x/y coordinates for each
int levelFish[3][4];				// stores 6 obstacles [0]x [1]y [2]active [3]y direction
int lifeCounter = 4;				// counter to change led display for lives
int livesDisplay = 0x1F;			// lives display value
int lives = 5;						// lives counter for logic
int score = 0;						// score counter

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
void isCollision ();					// Checks for collisions
void isCatch ();						// Checks for fish collisions aka catch
void loadGraphics();					// Puts graphics into buffer
void loadString();						// Puts string characters into text buffer
void updateLogic();						// Updates game data based on inputs
void loadLevel1();
void loadLevel2();
void loadLevel3();
void setFish();
void moveFish();
int powerOf (int exp, int base);		// simple function for calculating power


/* Set up ISR */
void user_isr()
{
	if (IFS(0) & 0x10000)
 	{
    	IFS(0) &= ~0x10000;
		tickClk();
  	}
	
	if (getClk() == 500) {
		moveFish();
		clearClk();
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
	*porte = 0x001f & livesDisplay;
	
	// Set fish
	setFish();
	
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
	spritefront = spritex + (6 * spritedirection);
	
	// Display Scroll
	if(spritefront >= 110 & (inputs & BTN_1)) {
		delayms(16);
		if(levelPos < 128) {
			levelPos++;
			spritex--;
		}
	}
	
	// COLLISION DETECTION
	isCollision();
	
	// FISH CATCH DETECTION
	isCatch();
	
	// DETECT GAME OVER
	if(lives == 0) {
		// end game logic
	}
	
	// DETECT HITING END OF LEVEL FLAG
	if(levelPos == 128 & spritefront >= 118) {
		delayms(128);
		spritex = 10;
		spritey = 16;
		currentLevel++;
		levelPos = 0;
		setFish();
	}
	
	// DETECT GAME COMPLETED
	
}

// COLLISION DETECTION FUNCTION
void isCollision () {
	int obj;
	for(obj = 0; obj < 5; obj++) {
		if ( ((spritey + 4) >= levelObstacles[obj][1] & (spritey + 4) <= (levelObstacles[obj][1] + 7) ) & (spritefront == (levelObstacles[obj][0] - levelPos) | spritefront == ((levelObstacles[obj][0] - levelPos)+8*spritedirection)) ) {
			// COLLISION DETECTED
			delayms(128);
			livesDisplay = livesDisplay - powerOf(lifeCounter, 2);
			spritex = spritex - 4*spritedirection;
			// Update LEDs to show lives
			// Port E 7-0 are the LEDs PORTE has address 0xbf886110
			volatile int* porte = (volatile int*) 0xbf886110;
			*porte = 0x001f & livesDisplay;
			// decrease lifeCounter
			lifeCounter--;
			// decrease life total
			lives--;
		}
	}
}

void isCatch() {
	
	int fishNumber;
	for(fishNumber = 0; fishNumber < 3; fishNumber++) {
		
		if ( (levelFish[fishNumber][2] == 1) & ((spritey + 4) >= levelFish[fishNumber][1] & (spritey + 4) <= (levelFish[fishNumber][1] + 8) ) & (spritefront == (levelFish[fishNumber][0] - levelPos) | spritefront == ((levelFish[fishNumber][0] - levelPos)+8*spritedirection)) ) {
			// CATCH DETECTED
			delayms(64);
			levelFish[fishNumber][2] = 0;	// set fish to inactive
			score++;						// increase score total
		}
	}
}


// Updates display with new graphics from displayBuffer
void loadGraphics() {
	
	clearBuffer(512, displayBuffer); 	//Clears the display buffer

	// Function to put player and scene graphics in buffer
	loadSprite(spritex, spritey, spritedirection, shark, displayBuffer);
	loadTiles(0, 2, 16, 128, lowerLine, displayBuffer); //load wave tiles
	loadFish (levelFish, fish, levelBuffer);
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
	
	// x,y,direction,name,buffer
	loadRock (70, 23, 1, rock, levelBuffer);
	loadRock (180, 23, 1, rock, levelBuffer);
	loadHook (100, 8, 1, hook, levelBuffer);
	loadNet (220, 16, 1, net, levelBuffer);
	
	// Set the x and y coordinates of obstacles for collision detection
	// 0 = x, 1 = y
	levelObstacles[0][0] = 70;
	levelObstacles[0][1] = 23;
	levelObstacles[1][0] = 100;
	levelObstacles[1][1] = 8;
	levelObstacles[2][0] = 180;
	levelObstacles[2][1] = 23;
	levelObstacles[3][0] = 220;
	levelObstacles[3][1] = 16;
	levelObstacles[4][0] = 0;
	levelObstacles[4][1] = 0;
	
	// Add end flag
	loadObject(246, 2, 8, 1, flag, levelBuffer);
		
}

void loadLevel2 () {
	
	clearBuffer(1024, levelBuffer); 	//Clears the level buffer
	
	loadTiles(0, 0, 32, 256, wave, levelBuffer); //load wave tiles
	
	// x,y,direction,name,buffer
	loadRock (50, 23, 1, rock, levelBuffer);
	loadRock (118, 23, 1, rock, levelBuffer);
	loadNet (140, 16, 1, net, levelBuffer);
	loadHook (200, 8, 1, hook, levelBuffer);
	
	// Set the x and y coordinates of obstacles for collision detection
	// 0 = x, 1 = y
	levelObstacles[0][0] = 50;
	levelObstacles[0][1] = 23;
	levelObstacles[1][0] = 118;
	levelObstacles[1][1] = 23;
	levelObstacles[2][0] = 140;
	levelObstacles[2][1] = 16;
	levelObstacles[3][0] = 200;
	levelObstacles[3][1] = 8;
	levelObstacles[4][0] = 0;
	levelObstacles[4][1] = 0;
	
	// Add end flag
	loadObject(240, 2, 8, 1, flag, levelBuffer);
	
}

void loadLevel3 () {
	
	clearBuffer(1024, levelBuffer); 	//Clears the level buffer
	
	loadTiles(0, 0, 32, 256, wave, levelBuffer); //load wave tiles
	
	// x,y,size,direction,name,buffer
	loadRock (50, 23, 1, rock, levelBuffer);
	loadRock (100, 23, 1, rock, levelBuffer);
	loadNet (130, 16, 1, net, levelBuffer);
	loadNet (146, 16, 1, net, levelBuffer);
	loadRock (200, 23, -1, rock, levelBuffer);
	
	// Set the x and y coordinates of obstacles for collision detection
	// 0 = x, 1 = y
	levelObstacles[0][0] = 50;
	levelObstacles[0][1] = 23;
	levelObstacles[1][0] = 100;
	levelObstacles[1][1] = 23;
	levelObstacles[2][0] = 130;
	levelObstacles[2][1] = 16;
	levelObstacles[3][0] = 146;
	levelObstacles[3][1] = 16;
	levelObstacles[4][0] = 200;
	levelObstacles[4][1] = 23;
	
	// Add end flag
	loadObject(240, 2, 8, 1, flag, levelBuffer);
	
}

void setFish() {
	
	if (currentLevel == 1) {
		levelFish[0][0] = 120;
		levelFish[0][1] = 18;
		levelFish[0][2] = 1;
		levelFish[0][3] = 1;
	
		levelFish[1][0] = 164;
		levelFish[1][1] = 20;
		levelFish[1][2] = 1;
		levelFish[1][3] = 1;
	
		levelFish[2][0] = 200;
		levelFish[2][1] = 14;
		levelFish[2][2] = 1;
		levelFish[2][3] = 1;
	} 
	else if (currentLevel == 2) {
		levelFish[0][0] = 120;
		levelFish[0][1] = 18;
		levelFish[0][2] = 1;
		levelFish[0][3] = 1;
	
		levelFish[1][0] = 200;
		levelFish[1][1] = 12;
		levelFish[1][2] = 1;
		levelFish[1][3] = 1;
	
		levelFish[2][0] = 250;
		levelFish[2][1] = 10;
		levelFish[2][2] = 1;
		levelFish[2][3] = 1;
	} 
	else if (currentLevel == 3) {
		levelFish[0][0] = 120;
		levelFish[0][1] = 18;
		levelFish[0][2] = 1;
		levelFish[0][3] = 2;
	
		levelFish[1][0] = 180;
		levelFish[1][1] = 14;
		levelFish[1][2] = 1;
		levelFish[1][3] = 1;
	
		levelFish[2][0] = 250;
		levelFish[2][1] = 18;
		levelFish[2][2] = 1;
		levelFish[2][3] = 2;
	}
}

void moveFish() {
	
	int fishNumber;
	for(fishNumber = 0; fishNumber < 3; fishNumber++) {
		// move in x
		if(levelFish[fishNumber][0] > -2) {
			levelFish[fishNumber][0]--;	// move all fish in the x-axis by -1
		} else if (levelFish[fishNumber][0] == -2) {
			levelFish[fishNumber][2] = 0;
		}
		
		// move in y
		if(levelFish[fishNumber][1] == 8) {
			levelFish[fishNumber][3] *= -1;
		}
		if(levelFish[fishNumber][1] == 20) {
			levelFish[fishNumber][3] *= -1;
		}
		levelFish[fishNumber][1] += levelFish[fishNumber][3];	// move fish in y-axis
	}
	
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


