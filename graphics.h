#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <pic32mx.h>

/* Declare bitmap array containing font */
extern const uint8_t const font[128*8];

/* Declare bitmap array containing icon */
extern const uint8_t const icon[128];

/* Declare text buffer for display output */
extern char textbuffer[4][16];


#endif