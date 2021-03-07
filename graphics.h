#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <pic32mx.h>

extern char textbuffer[4][16];

/* Declare bitmap array containing font */
extern const uint8_t const font[128*8];

/* Declare bitmap array containing icon */
extern const uint8_t const icon[128];

/* Declare text buffer for display output */
extern char textbuffer[4][16];


/* SPRITES */

extern int shark [23][2];

extern int hook [11][2];

extern int fish [14][2];

extern int rock [23][2];

extern int net[127][2];


extern const uint8_t flag[8];

extern const uint8_t wave[8];

extern const uint8_t lowerLine[8];

extern const uint8_t scoreLabel[24];

extern const uint8_t livesLabel[32];

extern const uint8_t title[59];

extern const uint8_t credits[77];

extern const uint8_t pressTo[45];

extern const uint8_t nextLevelDis[45];

extern const uint8_t gameOverDis[43];

extern const uint8_t gameWonDis[30];

extern const uint8_t btn4select[48];

extern const uint8_t btn3next[38];

extern const uint8_t btn1save[41];

extern const uint8_t const font[128*8];

extern uint8_t numbersScore[30];


#endif