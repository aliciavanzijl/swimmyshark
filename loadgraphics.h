#ifndef LOADGRAPHICS_H
#define LOADGRAPHICS_H

#include <stdint.h>


//void loadSprite (int x, int y, int direction, const uint8_t *data, uint8_t * displayBuffer);

void loadSprite (int x, int y, int direction, const uint8_t * data[23][2], uint8_t * displayBuffer);

void loadTiles (int x, int y, int tiles, int bufferSize, const uint8_t *data, uint8_t * buffer);

void loadObject (int x, int y, int size, int direction, const uint8_t *data, uint8_t * buffer);

void loadLevel (int levelPos, uint8_t *levelBuffer, uint8_t *displayBuffer);

void setBit(uint8_t* byte, uint8_t b);

#endif