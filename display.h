#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

void displayUpdate(uint8_t * byteBuffer);
void displayInit();
void clearBuffer(int size, uint8_t * byteBuffer);
void clearBufferRow(int row, uint8_t * byteBuffer);

#endif