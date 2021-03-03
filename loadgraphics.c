#include <stdint.h>
#include <pic32mx.h>
#include "display.h"

//uint8_t levelBuffer[1024];	// Level buffer
/*
const uint8_t sharkA [23][2] = {
	{3, -6},
	{3, -5}, {4, -5},
	{0, -4}, {2, -4}, {3, -4}, {4, -4}, {5, -4}, {6, -4}, {7, -4},
	{1, -3}, {2, -3}, {3, -3}, {4, -3}, {5, -3}, {7, -3},
	{0, -2}, {2, -2}, {3, -2}, {4, -2}, {5, -2}, {6, -2},
	{4, -1}
};*/

/* PROTOTYPING */
void setBit(uint8_t* byte, uint8_t b);

/* Functions */
/*
void loadSprite (int x, int y, int direction, const uint8_t * data[23][2], uint8_t * displayBuffer) {
	
	
	//int size = 8;
	//int i;
	//int pos = 128*y + x;
	
	//for(i = 0; i < size; i++) {
		//displayBuffer[pos+(i*direction)] |= data[i];
	//}
	
	//int size = sizeof(data) / sizeof(data[0]);
	int size = 23;
	
	uint8_t valueX;
	uint8_t valueY;
	int B, b;
	int i;
	
	for(i = 0; i < size; i++) {
		
		valueX = (data[i][0] + x) * direction;
		valueY = data[i][1] + y;
		
		B = (valueY / 8) * 128 + valueX;
		b = valueY % 8;
		
		setBit(&displayBuffer[B], b);
	}
} */ 

void testSprite (int x, int y, int direction, int data[23][2], uint8_t * displayBuffer) {
	
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


