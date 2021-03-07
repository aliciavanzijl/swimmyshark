#include <pic32mx.h>
#include <stdint.h>
#include "display.h"
#include "timers.h"
#include "io.h"
#include "loadgraphics.h"
#include "graphics.h"


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

int gameFinished = 0;				// game finished flag

// Score Variables
int score = 0;						// score counter
int playTimeCounter = 0;			// time counter
int finalScore = 0;					// final score holder
int newInitial[3];					// array to hold new initials (temp)
int highScores [3][4];				// hold scores [0]score [1-3]initials

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
void titleScreen ();
void mainMenu();
void gameLoop();
void isCollision ();					// Checks for collisions
void isCatch ();						// Checks for fish collisions aka catch

void loadGraphics();					// Puts graphics into buffer
void displayMenuPage();					// Puts screens/menu into buffer
void displayTitlePage();
void displayNextLevelPage();
void displayGameOver();
void displayGameWin();
void showHighScores();

void stringToBuffer(int line, char *s);	// Puts string characters into text buffer

void display_string(int line, char *s); // puts string into text buffer

void highScoreChecker();
void getInitials();
void sortHighScores();

void updateLogic();						// Updates game data based on inputs
void loadLevel1();						// Loads level objects
void loadLevel2();
void loadLevel3();
void setFish();							// sets fish stats per level
void moveFish();						// fish movement function
int powerOf (int exp, int base);		// simple function for calculating power


/* Set up ISR */
void user_isr()
{
	if (IFS(0) & 0x10000)
 	{
    	IFS(0) &= ~0x10000;
		tickClk();
  	}
	
	// Fish movement on clock ticks
	if (getClk() == 500) {
		moveFish();
		playTimeCounter++;
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

/* MAIN FUNCTION */
int main() {
	
	start();			// calls start function to init display, timers, & IO
	while(1) {
		titleScreen();	// displays the title screen
	}
	
	return 0;
	
}

/* DISPLAYS TITLE & CREDITS */
void titleScreen () {
	
	while(1) {
		
		inputHandler();				// poll inputs
		
		displayTitlePage();			// Puts tile graphics on the display
		
		if(inputs & BTN_1) {
			delayms(16);
			mainMenu();				// open menu
		}
	}
	return;
}

/* DISPLAYS MAIN MENU */
void mainMenu() {
	
	while(1) {
		
		inputHandler();			// poll inputs

		displayMenuPage();		// puts menu text on display
		
		// Start Game
		if(inputs & BTN_2) {
			delayms(16);
			gameLoop();			// start game
		}
		if(inputs & BTN_4) {
			delayms(16);
			showHighScores();	// show high scores
		}
		
	}
	return;
}

/* MAIN GAME LOOP */
void gameLoop () {
	
	gameFinished = 0;		// flag to check if game finished
	
	/* initialise some values for start of the game */
	spritex = 10;			// starting x position
	spritey = 20;			// starting y position
	currentLevel = 1;		
	
	score = 0;				// starting score 0
	levelPos = 0;			// position in the level (to scroll screen)
	
	lives = 5;				// starting lives 5
	lifeCounter = 4;		// life counter (for descreasing lives)
	livesDisplay = 0x1F;	// set LEDs to show 5 lights
	
	/* Update LEDs to show lives
	** Port E 7-0 are the LEDs PORTE has address 0xbf886110 */
	volatile int* porte = (volatile int*) 0xbf886110;
	*porte = 0x001f & livesDisplay;
	
	setFish();				// set fish for the level
	
	while(1) {
		
		if(gameFinished == 1) {
			return;
		}
		
		inputHandler();						// polls inputs
	
		updateLogic();						// game logic - interactive stuff here
		
		if (gameFinished == 1) {
			break;
		}
		
		if(currentLevel == 1) {				// checks current level and load obstacles
			loadLevel1(levelBuffer);
		} else if (currentLevel == 2) {
			loadLevel2(levelBuffer);
		} else if (currentLevel == 3) {
			loadLevel3(levelBuffer);
		}
		
		delayms(16);						// slow it down a bit so doesn't flicker
		loadGraphics();						// loads all graphics to the buffer
	}
	
	return;
	
}

/* VERY IMPORTANT INPUT HANDLER - GETS IO DATA */
void inputHandler() {
	
	inputs = getInputs();            		//get inputs from board - sws & btns
	
}

/* UPDATES GAME LOGIC PER INPUTS ETC */
void updateLogic() {
	
	/* MOVES SHARK PER BUTTONS */
	if(inputs & BTN_1) {					// move in x direction - right
		if(spritedirection == 1) {			// if already facing right, move
			if(spritex < 120) {
				spritex++;
			}
		} else {
			spritedirection = 1;			// if not facing right, flip
		}
	}
	
	if(inputs & BTN_4) {					// move in x direction - left
		if(spritedirection == -1) {			// if already facing left, move
			if(spritex > 8) {
				spritex--;
			}
		} else {
			spritedirection = -1;			// if not facing left, flip
		}
	}
	
	if(inputs & BTN_3) {					// move in y direction down
		if(spritey < 23) {					// limited by max down of display - UI
			delayms(16);
			spritey++;
		}
	}
	if(inputs & BTN_2) {					// move in y direction up
		if(spritey > 8) {					// limited by max up of display - waves
			delayms(16);
			spritey--;
		}
	}
	
	/* Update sprite front for collision detection
	** x is at back of sprite so add 7 x direction to get the front of the sprite */
	spritefront = spritex + (7 * spritedirection);		
	
	// Level Scroll
	// Once sprite front hits 100 and the player is still trying to move right
	if(spritefront >= 100 & (inputs & BTN_1)) {
		delayms(16);
		if(levelPos < 128) {
			levelPos++;				// scroll the level right
			spritex--;				// keep the sprite in place
		}
	}
	
	// COLLISION DETECTION
	isCollision();
	
	// FISH CATCH DETECTION
	isCatch();
	
	/* DETECT HITING END OF LEVEL FLAG
	** levelPos 128 means end of scroll, 
	** spritefront reaching anywhere in the y of the flag is a valid win */
	if(levelPos == 128 & spritefront >= 118) {
		
		if (currentLevel != 3) {		// if game not over, show Next Level on display
			
			displayNextLevelPage();
			delayms(800);				// delay so it stays up for a short while
			
		}
		
		spritex = 10;			// reinitialise shark position
		spritey = 20;			// reinitialise shark position
		currentLevel++;			// change to next level
		levelPos = 0;			// reset screen scroll position
		setFish();				// set fish for new level
	}
	
	// DETECT GAME COMPLETED
	if(currentLevel > 3) {

		displayGameWin();		// show You Win! on the screen
		delayms(700);			// delay so it stays up for a short while
		highScoreChecker(score);// check for high score
		gameFinished = 1;		// exit game loop by setting flag
	}
	
	// DETECT GAME OVER
	if(lives == 0) {

		displayGameOver();
		gameFinished = 1;		// exit game loop by setting flag
		delayms(700);			// delay so it stays up for a short while
	}
	
}

/* COLLISION DETECTION FUNCTION
** Goes through all 5 level obstacles, checks if shark overlaps obstacle position data stored in array*/
void isCollision () {
	int obj;
	for(obj = 0; obj < 5; obj++) {
		if ( ((spritey + 4) >= levelObstacles[obj][1] & (spritey + 4) <= (levelObstacles[obj][1] + 7) ) & (spritefront == (levelObstacles[obj][0] - levelPos) | spritefront == ((levelObstacles[obj][0] - levelPos)+8*spritedirection)) ) {
			
			// COLLISION DETECTED
			delayms(128);
			/* decrease life display LEDs
			** e.g. to get 0x1F to change to Ox0F, need to minus 16, which is 4^2
			** Then minus 8 = 2^3, then 4 = 2^2, then 2 = 2^1, then 1 = 2^0
			** so lifeCounter holds the decreasing exponents */
			livesDisplay = livesDisplay - powerOf(lifeCounter, 2); 
			
			// push shark back a bit from obstacle
			spritex = spritex - 6*spritedirection; 
			
			// Update LEDs to show lives
			// Port E 7-0 are the LEDs PORTE has address 0xbf886110
			volatile int* porte = (volatile int*) 0xbf886110;
			*porte = 0x001f & livesDisplay;
			
			lifeCounter--;						// decrease lifeCounter
			lives--;							// decrease life total
			
		}
	}
}

/* FISH CATCH DETECTION FUNCTION
** Goes through the 3 fish per level, checks if fish is active and shark overlaps
** Sets fish to inactive so that it is removed from the display
** increases score */
void isCatch() {
	
	int fishNumber;
	for(fishNumber = 0; fishNumber < 3; fishNumber++) {
		
		if ( (levelFish[fishNumber][2] == 1) & ((spritey + 4) >= levelFish[fishNumber][1] & (spritey + 4) <= (levelFish[fishNumber][1] + 8) ) & (spritefront == (levelFish[fishNumber][0] - levelPos) | spritefront == ((levelFish[fishNumber][0] - levelPos)+8*spritedirection)) ) {
			// CATCH DETECTED
			delayms(64);
			levelFish[fishNumber][2] = 0;	// set fish to inactive
			score += 2;						// increase score total
		}
	}
}


// Updates display with new graphics from displayBuffer
void loadGraphics() {
	
	clearBuffer(512, displayBuffer); 	//Clears the display buffer

	// Function to put player and scene graphics in buffer
	loadSprite(spritex, spritey, spritedirection, shark, displayBuffer);	// shark
	loadTiles(0, 2, 16, 128, lowerLine, displayBuffer); 					// waves
	loadFish (levelFish, fish, levelBuffer);								// fish
	loadLevel(levelPos, levelBuffer, displayBuffer);						// level obstacles flag etc
	loadUI(4, 3, score, lives, scoreLabel, livesLabel, displayBuffer);		// UI
	
	displayUpdate(displayBuffer); //displays new buffer
}

/* THE FOLLOWING UPDATE DISPLAY WITH SCREENS - NOT GAME PLAY */

// Main Menu Display
void displayMenuPage() {
	
	clearBuffer(512, displayBuffer); 	// Clears the display buffer
	
	display_string(0, "Press:");
	display_string(1, "BTN2 - play");
	display_string(2, "BTN4 - highscores");
	display_string(3, "                 ");
	
	display_update(textBuffer);

}

// Title Page
void displayTitlePage() {
	
	clearBuffer(512, displayBuffer); 	//Clears the display buffer

	loadScreen (36, 1, 59, title, displayBuffer);
	loadScreen (27, 2, 77, credits, displayBuffer);
	loadScreen (42, 3, 45, pressTo, displayBuffer);
		
	displayUpdate(displayBuffer); //displays new buffer
	
}

// Next Level
void displayNextLevelPage() {
	
	clearBuffer(512, displayBuffer); 	//Clears the display buffer
	
	loadScreen (42, 1, 45, nextLevelDis, displayBuffer);
	
	displayUpdate(displayBuffer); //displays new buffer
	
}

// Game Over
void displayGameOver() {
	
	clearBuffer(512, displayBuffer); 	//Clears the display buffer
	
	loadScreen (43, 1, 43, gameOverDis, displayBuffer);
	
	displayUpdate(displayBuffer); //displays new buffer
	
}

// Game Win
void displayGameWin() {
	
	clearBuffer(512, displayBuffer); 	//Clears the display buffer
	loadScreen (49, 1, 30, gameWonDis, displayBuffer);
	displayUpdate(displayBuffer); //displays new buffer
	
}

/* END OF SCREEN DISPLAY FUNCTIONS */

/* HIGH SCORE FUNCTIONS */

// DISPLAYS HIGH SCORES
void showHighScores() {
	
	while (1) {
		
		inputHandler();						// Polls inputs 		
		
		clearBuffer(512, displayBuffer); 	// Clears the display buffer
	
		int offset;
		int i;
	
		// highest
		loadCharacter (44, 1, highScores[2][1], font, displayBuffer);
		loadCharacter (54, 1, highScores[2][2], font, displayBuffer);
		loadCharacter (64, 1, highScores[2][3], font, displayBuffer);
	
		offset = 128*1 + 64 + 10;
	
		// Insert Score Number
		if (highScores[2][0] >= 0) {
		
			int holder = highScores[2][0];
			int ones = 0;
			int tens = 0;
			int hundreds = 0;
		
			// Checks for 100s digits
			for (i = 0; i < 10; i++) 
			{
				if (holder >= 100) 
				{
					hundreds++;
					holder -= 100;
				}
			}
			// Checks for 10s digits
			for (i = 0; i < 10; i++) 
			{
				if (holder >= 10) 
				{
					tens++;
					holder -= 10;
				}
			}
			// Checks for 1s digits
			for (i = 0; i < 10; i++) 
			{
				if (holder >= 1) 
				{
					ones++;
					holder -= 1;
				}
			}
		
			for (i = 0; i < 3; i++) 
			{
				displayBuffer[offset+i] |= numbersScore[(hundreds)*3 + i];
			}
			offset += 4;
		
			for (i = 0; i < 3; i++) 
			{
				displayBuffer[offset+i] |= numbersScore[(tens)*3 + i];
			}
			offset += 4;
		
			for (i = 0; i < 3; i++) 
			{
				displayBuffer[offset+i] |= numbersScore[(ones)*3 + i];
			}
			offset += 4;
		}
	
		// med
		loadCharacter (44, 2, highScores[1][1], font, displayBuffer);
		loadCharacter (54, 2, highScores[1][2], font, displayBuffer);
		loadCharacter (64, 2, highScores[1][3], font, displayBuffer);
	
		offset = 128*2 + 64 + 10;
	
		// show score Number
		if (highScores[1][0] >= 0) {
		
			int holder = highScores[1][0];
			int ones = 0;
			int tens = 0;
			int hundreds = 0;
		
			// Checks for 100s digits
			for (i = 0; i < 10; i++) 
			{
				if (holder >= 100) 
				{
					hundreds++;
					holder -= 100;
				}
			}
			// Checks for 10s digits
			for (i = 0; i < 10; i++) 
			{
				if (holder >= 10) 
				{
					tens++;
					holder -= 10;
				}
			}
			// Checks for 1s digits
			for (i = 0; i < 10; i++) 
			{
				if (holder >= 1) 
				{
					ones++;
					holder -= 1;
				}
			}
		
			for (i = 0; i < 3; i++) 
			{
				displayBuffer[offset+i] |= numbersScore[(hundreds)*3 + i];
			}
			offset += 4;
		
			for (i = 0; i < 3; i++) 
			{
				displayBuffer[offset+i] |= numbersScore[(tens)*3 + i];
			}
			offset += 4;
		
			for (i = 0; i < 3; i++) 
			{
				displayBuffer[offset+i] |= numbersScore[(ones)*3 + i];
			}
			offset += 4;
		}
	
		// lowest
		loadCharacter (44, 3, highScores[0][1], font, displayBuffer);
		loadCharacter (54, 3, highScores[0][2], font, displayBuffer);
		loadCharacter (64, 3, highScores[0][3], font, displayBuffer);
	
		offset = 128*3 + 64 + 10;
	
		// Insert Score Number
		if (highScores[0][0] >= 0) {
		
			int holder = highScores[0][0];
			int ones = 0;
			int tens = 0;
			int hundreds = 0;
		
			// Checks for 100s digits
			for (i = 0; i < 10; i++) 
			{
				if (holder >= 100) 
				{
					hundreds++;
					holder -= 100;
				}
			}
			// Checks for 10s digits
			for (i = 0; i < 10; i++) 
			{
				if (holder >= 10) 
				{
					tens++;
					holder -= 10;
				}
			}
			// Checks for 1s digits
			for (i = 0; i < 10; i++) 
			{
				if (holder >= 1) 
				{
					ones++;
					holder -= 1;
				}
			}
		
			for (i = 0; i < 3; i++) 
			{
				displayBuffer[offset+i] |= numbersScore[(hundreds)*3 + i];
			}
			offset += 4;
		
			for (i = 0; i < 3; i++) 
			{
				displayBuffer[offset+i] |= numbersScore[(tens)*3 + i];
			}
			offset += 4;
		
			for (i = 0; i < 3; i++) 
			{
				displayBuffer[offset+i] |= numbersScore[(ones)*3 + i];
			}
			offset += 4;
		}
	
		displayUpdate(displayBuffer); //displays new buffer
		
		
		if(inputs & BTN_1) {
			delayms(16);
			mainMenu();				// open menu
		}
	}
	
	
}

/* CHECKS FOR NEW HIGH SCORE 
** AND SETS NEW DATA IN ARRAY FOR DISPLAY */
void highScoreChecker(int score) {
	
	// calculate final score - multiplied by 10 because looks cooler!
	// and can also add in speed bonus/deduction later
	finalScore = (score * 10);
	
	int i;
	int isHigh = 0;
	
	// Check if the new score is high
	for(i = 0; i < 3; i++) {
		if(finalScore > highScores[i][0]) {
			isHigh = 1;
		}
	}
	
	if(isHigh == 0) {
		mainMenu();				// returns player to main menu
	}
	
	// if a new high score
	if(isHigh == 1) {
		
		getInitials();		// gets the iniials input from player
		
		// find minimum in current high score array
		int min = 0;
		for (i = 1; i < 3; i++) {
			if(highScores[i][0] < highScores[min][0]){
				min = i;
			}
		}
		
		// Replace minimum with new score and initials
		highScores[min][0] = finalScore;
		highScores[min][1] = newInitial[0];
		highScores[min][2] = newInitial[1];
		highScores[min][3] = newInitial[2];
		
	}

	// Sort highScores into descending order
	sortHighScores();
	
	gameFinished = 1;		// exit game loop (added in case of bug)
	mainMenu();				// returns player to main menu
	
}

/* GETS INITIALS INPUT FROM PLAYER */
void getInitials () {
	
	// Function to get intitials
	clearBuffer(512, displayBuffer); 	// Clears the display buffer
	
	display_string(1, "New High Score!");
	display_string(2, "Enter Initials!");
	display_update(textBuffer);
	delayms(900);
	
	//Initialise display	
	int initialOne = 65;
	int initialTwo = 65;
	int initialThree = 65;
	int currentInitial = 1;
	
	clearBuffer(512, displayBuffer); 	// Clears the display buffer
	
	// PUT INSTRUCTIONS ON SCREEN
	loadScreen (20, 0, 48, btn4select, displayBuffer);
	loadScreen (20, 1, 38, btn3next, displayBuffer);
	loadScreen (20, 2, 41, btn1save, displayBuffer);
	
	// SHOW CHARACTER SELECTION ON SCREEN
	loadCharacter (44, 3, initialOne, font, displayBuffer);
	loadCharacter (54, 3, initialTwo, font, displayBuffer);
	loadCharacter (64, 3, initialThree, font, displayBuffer);
	
	displayUpdate(displayBuffer); //displays new buffer
	
	// GET INPUT
	while(1) {
				
		inputHandler();			// poll inputs
		
		// LOGIC TIME
		
		// Change Initials
		if(inputs & BTN_4) {
			// Change Initial One
			if(currentInitial == 1) {
				if(initialOne < 90) {
					initialOne++;
					delayms(200);
				} else {
					initialOne = 65;
					delayms(200);
				}
			}
			// Change Initial Two
			if(currentInitial == 2) {
				if(initialTwo < 90) {
					initialTwo++;
					delayms(200);
				} else {
					initialTwo = 65;
					delayms(200);
				}
			}
			// Change Initial Three
			if(currentInitial == 3) {
				if(initialThree < 90) {
					initialThree++;
					delayms(200);
				} else {
					initialThree = 65;
					delayms(200);
				}
			}
		}
		
		// Switch Initial
		if(inputs & BTN_3) {
			if(currentInitial < 3) {
				currentInitial++;
				delayms(200);
			} else {
				currentInitial = 1;
				delayms(200);
			}
		}
		
		// Save Initials
		if(inputs & BTN_1) {
			
			newInitial[0] = initialOne;
			newInitial[1] = initialTwo;
			newInitial[2] = initialThree;
			
			delayms(16);
			break;
		}
		
		delayms(16);		// slow it down a bit so graphics doesn't jump
		
		clearBuffer(512, displayBuffer); 	// Clears the display buffer
		
		// CONTINUE TO SHOW INSTRUCTIONS
		loadScreen (20, 0, 48, btn4select, displayBuffer);
		loadScreen (20, 1, 38, btn3next, displayBuffer);
		loadScreen (20, 2, 41, btn1save, displayBuffer);
		
		// SHOW CURRENT CHARACTER SELECTIONS
		loadCharacter (44, 3, initialOne, font, displayBuffer);
		loadCharacter (54, 3, initialTwo, font, displayBuffer);
		loadCharacter (64, 3, initialThree, font, displayBuffer);
		
		displayUpdate(displayBuffer); 		//displays new buffer
		
	}
	
	return;
	
}

/* SORTS HIGH SCORE ARRAY INTO ORDER 
** LOWEST = 0
*/
void sortHighScores() {
	
	// holder
	int holder[1][4];
	int i;
	int j;
	int min;
	
	// find minimum in current high score array
	for (i = 0; i < 3; i++) {
		
		min = i;
		
		for (j = i+1; j < 3; j++) {
			if(highScores[j][0] < highScores[min][0]){
				min = j;
			}
		}
		
		// SWAP INTO PLACE
		// min into temp
		holder[0][0] = highScores[min][0];
		holder[0][1] = highScores[min][1];
		holder[0][2] = highScores[min][2];
		holder[0][3] = highScores[min][3];
		
		// put i position into min space
		highScores[min][0] = highScores[i][0];
		highScores[min][1] = highScores[i][1];
		highScores[min][2] = highScores[i][2];
		highScores[min][3] = highScores[i][3];
		
		// put min values into i position
		highScores[i][0] = holder[0][0];
		highScores[i][1] = holder[0][1];
		highScores[i][2] = holder[0][2];
		highScores[i][3] = holder[0][3];
		
	}
	
}

/* FUNCTIONS COPIED FROM LAB FILES FOR STRING DISPLAY */

// PUTS STRINGS INTO TEXT BUFFER
void display_string(int line, char *s) {
	int i;
	if(line < 0 || line >= 4)
		return;
	if(!s)
		return;
	
	for(i = 0; i < 16; i++)
		if(*s) {
			textBuffer[line][i] = *s;
			s++;
		} else
			textBuffer[line][i] = ' ';
}


/* LEVELS & FISH CODE */

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
	
	/* FISH DATA:
	** [0] starting x position
	** [1] starting y position
	** [2] active status - 1 is active
	** [3] y axis movement speed
	*/
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

/* FISH MOVEMENT DATA - UPDATED ON TIMER */
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

// Little Helper Math Function
// Used for updating LED lights to show lives
int powerOf (int exp, int base) {
	
	int result = 1;
	
    while (exp != 0) {
        result *= base;
        exp--;
    }
	
	return result;
	
}


