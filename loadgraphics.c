#include <stdint.h>
#include <pic32mx.h>
#include "display.h"

/* PROTOTYPING */
void setBit(uint8_t* byte, uint8_t b);

/* Functions */

void loadSprite (int x, int y, int direction, int data[23][2], uint8_t * displayBuffer) {
	
	int size = 23;
	
	int valueX;
	int valueY;
	int B, b;
	int i;
	
	for(i = 0; i < size; i++) {
		
		valueX = (data[i][0] * direction) + x;
		valueY = data[i][1] + y;
		
		B = (valueY / 8) * 128 + valueX;
		b = valueY % 8;
		
		setBit(&displayBuffer[B], b);
	}
}

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

void loadObject (int x, int y, int size, int direction, const uint8_t *data, uint8_t *buffer) {
	
	int i;
	int pos = 256*y + x;
	
	for(i = 0; i < size; i++) {
		buffer[pos+(i*direction)] |= data[i];	
	}
	
}

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


void loadFish (int levelFish[3][4], int data[14][2], uint8_t * levelBuffer) {
	
	int size = 14;
	
	int valueX;
	int valueY;
	int B, b;
	int i;
	
	int fishNumber;
	
	for(fishNumber = 0; fishNumber < 3; fishNumber++) {
		
		if (levelFish[fishNumber][2] == 1) {
			
			for(i = 0; i < size; i++) {
		
				valueX = (data[i][0]) + levelFish[fishNumber][0];
				valueY = data[i][1] + levelFish[fishNumber][1];
		
				B = (valueY / 8) * 256 + valueX;
				b = valueY % 8;
		
				setBit(&levelBuffer[B], b);
			}
		}
	}
}


void loadLevel (int levelPos, uint8_t *levelBuffer, uint8_t *displayBuffer) {
	
	int yPos;
	int xPos;
	
	for (yPos = 0; yPos < 3; yPos++) {
		for (xPos = 0; xPos < 128; xPos++) {
			displayBuffer[yPos*128+xPos] |= levelBuffer[yPos*256+xPos+levelPos];
		}
	}
	
}

//Sets a bit to the display buffer
void setBit(uint8_t* byte, uint8_t b)
{
	*byte |= ((uint8_t) 1 << b);
}


