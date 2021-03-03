#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <pic32mx.h>

void initIO();
uint8_t getInputs();
int getbtns();
int getsw();

#endif