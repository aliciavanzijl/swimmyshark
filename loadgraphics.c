#include <stdint.h>
#include <pic32mx.h>
#include "display.h"

#define DISPLAY_VDD PORTFbits.RF6
#define DISPLAY_VBATT PORTFbits.RF5
#define DISPLAY_COMMAND_DATA PORTFbits.RF4
#define DISPLAY_RESET PORTGbits.RG9


#define DISPLAY_VDD_PORT PORTF
#define DISPLAY_VDD_MASK 0x40
#define DISPLAY_VBATT_PORT PORTF
#define DISPLAY_VBATT_MASK 0x20
#define DISPLAY_COMMAND_DATA_PORT PORTF
#define DISPLAY_COMMAND_DATA_MASK 0x10
#define DISPLAY_RESET_PORT PORTG
#define DISPLAY_RESET_MASK 0x200

/* PROTOTYPING */
void setBit(uint8_t* byte, uint8_t b);

// FONT

static const uint8_t const font[] = {
	0, 0, 0, 0, 0, 0, 0, 0,				// [0]
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,				// [8]
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,				// [16]
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,				// [32] - do not use
	0, 0, 0, 94, 0, 0, 0, 0,			// [33] = 0x21 = !
	0, 0, 4, 3, 4, 3, 0, 0,				// [34] = 0x22 = "
	0, 36, 126, 36, 36, 126, 36, 0,		// [33] = 0x23 = #
	0, 36, 74, 255, 82, 36, 0, 0,		// [34] = 0x24 = $
	0, 70, 38, 16, 8, 100, 98, 0,		// [35] = 0x25 = %
	0, 52, 74, 74, 52, 32, 80, 0,		// [36] = 0x26 = &
	0, 0, 0, 4, 3, 0, 0, 0,				// [37] = 0x27 = '
	0, 0, 0, 126, 129, 0, 0, 0,			// [38] = 0x28 = (
	0, 0, 0, 129, 126, 0, 0, 0,			// [39] = 0x29 = )
	0, 42, 28, 62, 28, 42, 0, 0,		// [40] = 0x2A = *
	0, 8, 8, 62, 8, 8, 0, 0,			// [41]
	0, 0, 0, 128, 96, 0, 0, 0,			// [42]
	0, 8, 8, 8, 8, 8, 0, 0,				// [43]
	0, 0, 0, 0, 96, 0, 0, 0,			// [44]
	0, 64, 32, 16, 8, 4, 2, 0,			// [45]
	0, 62, 65, 73, 65, 62, 0, 0,		// [46]
	0, 0, 66, 127, 64, 0, 0, 0,			// [47]
	0, 0, 98, 81, 73, 70, 0, 0,			// [48] = 0x30 = 0
	0, 0, 34, 73, 73, 54, 0, 0,			// 1
	0, 0, 14, 8, 127, 8, 0, 0,			// 2
	0, 0, 35, 69, 69, 57, 0, 0,			// 3
	0, 0, 62, 73, 73, 50, 0, 0,			// 4
	0, 0, 1, 97, 25, 7, 0, 0,			// 5
	0, 0, 54, 73, 73, 54, 0, 0,			// 6
	0, 0, 6, 9, 9, 126, 0, 0,			// 7
	0, 0, 0, 102, 0, 0, 0, 0,			// 8
	0, 0, 128, 102, 0, 0, 0, 0,			// 9
	0, 0, 8, 20, 34, 65, 0, 0,			
	0, 0, 20, 20, 20, 20, 0, 0,
	0, 0, 65, 34, 20, 8, 0, 0,
	0, 2, 1, 81, 9, 6, 0, 0,
	0, 28, 34, 89, 89, 82, 12, 0,
	0, 0, 126, 9, 9, 126, 0, 0,
	0, 0, 127, 73, 73, 54, 0, 0,
	0, 0, 62, 65, 65, 34, 0, 0,			// [65] = 0x41 = A
	0, 0, 127, 65, 65, 62, 0, 0,		// B
	0, 0, 127, 73, 73, 65, 0, 0,		// C
	0, 0, 127, 9, 9, 1, 0, 0,			// D
	0, 0, 62, 65, 81, 50, 0, 0,
	0, 0, 127, 8, 8, 127, 0, 0,
	0, 0, 65, 127, 65, 0, 0, 0,
	0, 0, 32, 64, 64, 63, 0, 0,
	0, 0, 127, 8, 20, 99, 0, 0,
	0, 0, 127, 64, 64, 64, 0, 0,
	0, 127, 2, 4, 2, 127, 0, 0,
	0, 127, 6, 8, 48, 127, 0, 0,
	0, 0, 62, 65, 65, 62, 0, 0,
	0, 0, 127, 9, 9, 6, 0, 0,
	0, 0, 62, 65, 97, 126, 64, 0,
	0, 0, 127, 9, 9, 118, 0, 0,
	0, 0, 38, 73, 73, 50, 0, 0,
	0, 1, 1, 127, 1, 1, 0, 0,
	0, 0, 63, 64, 64, 63, 0, 0,
	0, 31, 32, 64, 32, 31, 0, 0,
	0, 63, 64, 48, 64, 63, 0, 0,
	0, 0, 119, 8, 8, 119, 0, 0,
	0, 3, 4, 120, 4, 3, 0, 0,
	0, 0, 113, 73, 73, 71, 0, 0,
	0, 0, 127, 65, 65, 0, 0, 0,
	0, 2, 4, 8, 16, 32, 64, 0,
	0, 0, 0, 65, 65, 127, 0, 0,
	0, 4, 2, 1, 2, 4, 0, 0,
	0, 64, 64, 64, 64, 64, 64, 0,
	0, 0, 1, 2, 4, 0, 0, 0,
	0, 0, 48, 72, 40, 120, 0, 0,
	0, 0, 127, 72, 72, 48, 0, 0,
	0, 0, 48, 72, 72, 0, 0, 0,
	0, 0, 48, 72, 72, 127, 0, 0,
	0, 0, 48, 88, 88, 16, 0, 0,
	0, 0, 126, 9, 1, 2, 0, 0,
	0, 0, 80, 152, 152, 112, 0, 0,
	0, 0, 127, 8, 8, 112, 0, 0,
	0, 0, 0, 122, 0, 0, 0, 0,
	0, 0, 64, 128, 128, 122, 0, 0,
	0, 0, 127, 16, 40, 72, 0, 0,
	0, 0, 0, 127, 0, 0, 0, 0,
	0, 120, 8, 16, 8, 112, 0, 0,
	0, 0, 120, 8, 8, 112, 0, 0,
	0, 0, 48, 72, 72, 48, 0, 0,
	0, 0, 248, 40, 40, 16, 0, 0,
	0, 0, 16, 40, 40, 248, 0, 0,
	0, 0, 112, 8, 8, 16, 0, 0,
	0, 0, 72, 84, 84, 36, 0, 0,
	0, 0, 8, 60, 72, 32, 0, 0,
	0, 0, 56, 64, 32, 120, 0, 0,
	0, 0, 56, 64, 56, 0, 0, 0,
	0, 56, 64, 32, 64, 56, 0, 0,
	0, 0, 72, 48, 48, 72, 0, 0,
	0, 0, 24, 160, 160, 120, 0, 0,
	0, 0, 100, 84, 84, 76, 0, 0,
	0, 0, 8, 28, 34, 65, 0, 0,
	0, 0, 0, 126, 0, 0, 0, 0,
	0, 0, 65, 34, 28, 8, 0, 0,
	0, 0, 4, 2, 4, 2, 0, 0,
	0, 120, 68, 66, 68, 120, 0, 0,
};

uint8_t numbers[]  = {
	0xF8, 0x88, 0xF8, 	//0
	0x08, 0xF8, 0x00, 	//1
    0xE8, 0xA8, 0xB8, 	//2
    0x88, 0xA8, 0xF8, 	//3
	0x38, 0x20, 0xF8, 	//4
	0xB8, 0xA8, 0xE8, 	//5
	0xF8, 0xA8, 0xE8, 	//6
	0x08, 0xE8, 0x18, 	//7
	0xF8, 0xA8, 0xF8, 	//8
	0xB8, 0xA8, 0xF8 	//9
};


/* Functions */

/* LOADS SHARK INTO THE DISPLAY BUFFER 
** USES ADVANCED POSITIONING AND LOADS PIXEL BY PIXEL
*/
void loadSprite (int x, int y, int direction, int data[23][2], uint8_t * displayBuffer) {
	
	int size = 23;
	
	int valueX;
	int valueY;
	int B, b;
	int i;
	
	for(i = 0; i < size; i++) {
		
		// takes x coordinate of pixel & offset of direction + starting x
		valueX = (data[i][0] * direction) + x; 
		
		// takes y coordinate of pixel & offset of starting y.
		valueY = data[i][1] + y;
		
		// B is set to the correct row and x value in dislay buffer
		B = (valueY / 8) * 128 + valueX;
		// b gives the offset within the byte to put the pixel in the correct place
		b = valueY % 8;
		
		// set the pixel as 1
		setBit(&displayBuffer[B], b);
		
	}
}

/* LOADS WAVE TILES INTO THE ANY BUFFER 
** USES SIMPLE POSITIONING (Y=0-3)
*/
void loadTiles (int x, int y, int tiles, int bufferSize, const uint8_t *data, uint8_t * buffer) {
	
	int size = 8;
	int i;
	int j;
	int pos = bufferSize*y + x;
	
	for (i = 0; i < tiles; i++) {
		for(j = 0; j < size; j++) {
			buffer[pos+j] |= data[j];
		}
		pos = pos + 8;
	}
}

/* LOADS OBJECTS INTO THE LEVEL BUFFER 
** USES SIMPLE POSITIONING (Y=0-3)
** DIRECTION CAN BE MODIFIED
*/
void loadObject (int x, int y, int size, int direction, const uint8_t *data, uint8_t *buffer) {
	
	int i;
	int pos = 256*y + x;
	
	for(i = 0; i < size; i++) {
		buffer[pos+(i*direction)] |= data[i];	
	}
	
}

/* LOADS CHARACTERS FROM FONT INTO THE DISPLAY BUFFER 
** USES SIMPLE POSITIONING (Y=0-3)
*/
void loadCharacter (int x, int line, int charNumber, const uint8_t *font, uint8_t *buffer) {
	
	int i;
	int pos = 128*line + x;
	
	for(i = 0; i < 8; i++) {
		buffer[pos+i] |= font[charNumber*8+i];
	}
	
}

/* LOADS LABELS (IMAGES) AND NUMBERS INTO THE DISPLAY BUFFER 
** USES SIMPLE POSITIONING (Y=0-3)
*/
void loadUI (int x, int y, int score, int lives, const uint8_t *scoreLabel, const uint8_t *livesLabel, uint8_t *displayBuffer) {
	
	// Add Score Label
	int i;
	int pos = 128*y + x;
	
	for (i = 0; i < 24; i++) {
		displayBuffer[pos+i] |= scoreLabel[i];
	}
	
	int offset = pos + 28;
	
	// Insert Score Number
	if (score >= 0) {
		
		int ones = 0;
		int tens = 0;
		int hundreds = 0;
		
		// Checks for 100s digits
		for (i = 0; i < 10; i++) 
		{
			if (score >= 100) 
			{
				hundreds++;
				score -= 100;
			}
		}
		// Checks for 10s digits
		for (i = 0; i < 10; i++) 
		{
			if (score >= 10) 
			{
				tens++;
				score -= 10;
			}
		}
		// Checks for 1s digits
		for (i = 0; i < 10; i++) 
		{
			if (score >= 1) 
			{
				ones++;
				score -= 1;
			}
		}
		
		for (i = 0; i < 3; i++) 
		{
			displayBuffer[offset+i] |= numbers[(hundreds)*3 + i];
		}
		offset += 4;
		
		for (i = 0; i < 3; i++) 
		{
			displayBuffer[offset+i] |= numbers[(tens)*3 + i];
		}
		offset += 4;
		
		for (i = 0; i < 3; i++) 
		{
			displayBuffer[offset+i] |= numbers[(ones)*3 + i];
		}
		offset += 4;
		
	}
	
	// Add Lives Label
	int offsetLives = 80;
	// puts lives label and arrow on display
	for (i = 0; i < 32; i++) {
		displayBuffer[pos+i+offsetLives] |= livesLabel[i];
	}
	// puts lives number on screen
	for (i = 0; i < 3; i++) {
		displayBuffer[offsetLives+pos+34+i] |= numbers[(lives)*3 + i];
	}
	
	
}

/* LOADS ROCK INTO THE LEVEL BUFFER 
** USES ADVANCED POSITIONING AND LOADS PIXEL BY PIXEL
*/
void loadRock (int x, int y, int direction, int data[23][2], uint8_t * levelBuffer) {
	
	int size = 23;
	
	int valueX;
	int valueY;
	int B, b;
	int i;
	
	for(i = 0; i < size; i++) {
		
		valueX = (data[i][0] * direction) + x;
		valueY = data[i][1] + y;
		
		B = (valueY / 8) * 256 + valueX;
		b = valueY % 8;
		
		setBit(&levelBuffer[B], b);
	}
}

/* LOADS HOOK INTO THE LEVEL BUFFER 
** USES ADVANCED POSITIONING AND LOADS PIXEL BY PIXEL
*/
void loadHook (int x, int y, int direction, int data[11][2], uint8_t * levelBuffer) {
	
	int size = 11;
	
	int valueX;
	int valueY;
	int B, b;
	int i;
	
	for(i = 0; i < size; i++) {
		
		valueX = (data[i][0] * direction) + x;
		valueY = data[i][1] + y;
		
		B = (valueY / 8) * 256 + valueX;
		b = valueY % 8;
		
		setBit(&levelBuffer[B], b);
	}
}

/* LOADS NET INTO THE LEVEL BUFFER 
** USES ADVANCED POSITIONING AND LOADS PIXEL BY PIXEL
*/
void loadNet (int x, int y, int direction, int data[127][2], uint8_t * levelBuffer) {
	
	int size = 127;
	
	int valueX;
	int valueY;
	int B, b;
	int i;
	
	for(i = 0; i < size; i++) {
		
		valueX = (data[i][0] * direction) + x;
		valueY = data[i][1] + y;
		
		B = (valueY / 8) * 256 + valueX;
		b = valueY % 8;
		
		setBit(&levelBuffer[B], b);
	}
}

/* LOADS FISH INTO THE LEVEL BUFFER 
** USES ADVANCED POSITIONING AND LOADS PIXEL BY PIXEL
*/
void loadFish (int levelFish[3][4], int data[14][2], uint8_t * levelBuffer) {
	
	int size = 14;
	
	int valueX;
	int valueY;
	int B, b;
	int i;
	
	int fishNumber;
	
	for(fishNumber = 0; fishNumber < 3; fishNumber++) {
		
		if (levelFish[fishNumber][2] == 1) {	// IF FISH ACTIVE, SHOW
			
			for(i = 0; i < size; i++) {
		
				valueX = (data[i][0]) + levelFish[fishNumber][0]; 	// [0] x position
				valueY = data[i][1] + levelFish[fishNumber][1];		// [1] y position
		
				B = (valueY / 8) * 256 + valueX;
				b = valueY % 8;
		
				setBit(&levelBuffer[B], b);
			}
		}
	}
}

/* LOADS LEVEL BUFFER (FROM POSITION -> 128 pixels) INTO THE DISPLAY BUFFER 
** USES SIMPLE POSITIONING (Y=0-3)
** USES LEVELPOS AS AN OFFSET ON THE LEVEL BUFFER
*/
void loadLevel (int levelPos, uint8_t *levelBuffer, uint8_t *displayBuffer) {
	
	int yPos;
	int xPos;
	
	for (yPos = 0; yPos < 3; yPos++) {
		
		for (xPos = 0; xPos < 128; xPos++) {
			
			displayBuffer[yPos*128+xPos] |= levelBuffer[yPos*256+xPos+levelPos];
		}
	}
	
}

/* LOADS AN IMAGES (FOR SCREENS/MENUS) INTO THE DISPLAY BUFFER 
** USES SIMPLE POSITIONING (Y=0-3)
*/
void loadScreen (int x, int y, int size, const uint8_t *data, uint8_t *buffer) {
	
	int i;
	int pos = 128*y + x;
	
	for(i = 0; i < size; i++) {
		buffer[pos+i] |= data[i];	
	}
	
}

/* Text Display Functionality
** from labs
*/

uint8_t spi_send_recv(uint8_t data) {
	while(!(SPI2STAT & 0x08));
	SPI2BUF = data;
	while(!(SPI2STAT & 0x01));
	return SPI2BUF;
}

void display_update(char textBuffer[4][16]) {
	int i, j, k;
	int c;
	for(i = 0; i < 4; i++) {
		DISPLAY_COMMAND_DATA_PORT &= ~DISPLAY_COMMAND_DATA_MASK;
		spi_send_recv(0x22);
		spi_send_recv(i);
		
		spi_send_recv(0x0);
		spi_send_recv(0x10);
		
		DISPLAY_COMMAND_DATA_PORT |= DISPLAY_COMMAND_DATA_MASK;
		
		for(j = 0; j < 16; j++) {
			c = textBuffer[i][j];
			if(c & 0x80)
				continue;
			
			for(k = 0; k < 8; k++)
				spi_send_recv(font[c*8 + k]);
		}
	}
}

/* SETS A BIT TO THE DISPLAY BUFFER
** b is the offset on the coordinate
*/
void setBit(uint8_t* byte, uint8_t b)
{
	*byte |= ((uint8_t) 1 << b);
}



