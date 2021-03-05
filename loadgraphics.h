#ifndef LOADGRAPHICS_H
#define LOADGRAPHICS_H

#include <stdint.h>

void loadSprite (int x, int y, int direction, int data[23][2], uint8_t * displayBuffer);

void loadTiles (int x, int y, int tiles, int bufferSize, const uint8_t *data, uint8_t * buffer);

void loadObject (int x, int y, int size, int direction, const uint8_t *data, uint8_t * buffer);

void loadUI (int x, int y, int score, int lives, const uint8_t *scoreLabel, const uint8_t *livesLabel, uint8_t *displayBuffer);

void loadRock (int x, int y, int direction, int data[23][2], uint8_t * levelBuffer);
void loadHook (int x, int y, int direction, int data[11][2], uint8_t * levelBuffer);
void loadNet (int x, int y, int direction, int data[127][2], uint8_t * levelBuffer);

void loadFish (int levelFish[3][4], int data[14][2], uint8_t * levelBuffer);

void loadLevel (int levelPos, uint8_t *levelBuffer, uint8_t *displayBuffer);

void setBit(uint8_t* byte, uint8_t b);

#endif