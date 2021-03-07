#include <pic32mx.h>
#include <stdint.h>
#include "display.h"
#include "timers.h"
#include "io.h"
#include "loadgraphics.h"


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

/* SPRITES */

int shark [23][2] = {
	{3, -6},
	{3, -5}, {4, -5},
	{0, -4}, {2, -4}, {3, -4}, {4, -4}, {5, -4}, {6, -4}, {7, -4},
	{1, -3}, {2, -3}, {3, -3}, {4, -3}, {5, -3}, {7, -3},
	{0, -2}, {2, -2}, {3, -2}, {4, -2}, {5, -2}, {6, -2},
	{4, -1}
};

int hook [11][2] = {
	{3, 0}, {4, 0},
	{2, -1}, {5, -1},
	{2, -2}, {5, -2},
	{5, -3},
	{4, -4}, {4, -5}, {4, -6}, {4, -7}
};

int fish [14][2] = {
	{3, -1},
	{2, -2}, {3, -2}, {4, -2}, {6, -2},
	{1, -3}, {3, -3}, {4, -3}, {5, -3},
	{2, -4}, {3, -4}, {4, -4}, {6, -4},
	{4, -5}
};

int rock [23][2] = {
	{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0},
	{1, -1}, {2, -1}, {3, -1}, {4, -1}, {5, -1}, {6, -1},
	{2, -2}, {3, -2}, {4, -2}, {5, -2},
	{3, -3}, {4, -3},
	{3, -4}, {4, -4},
	{4, -5}
};

int net[127][2] = {
	{0, 0}, {12, 0},
	{1, -1}, {13, -1},
	{2, -2}, {3, -2}, {4, -2}, {5, -2}, {6, -2}, {7, -2}, {8, -2}, {9, -2}, {10, -2}, {11, -2}, {12, -2}, {13, -2}, {14, -2},
	{2, -3}, {5, -3}, {8, -3}, {11, -3}, {14, -3},
	{2, -4}, {3, -4}, {4, -4}, {5, -4}, {6, -4}, {7, -4}, {8, -4}, {9, -4}, {10, -4}, {11, -4}, {12, -4}, {13, -4}, {14, -4},
	{2, -5}, {5, -5}, {8, -5}, {11, -5}, {14, -5},
	{2, -6}, {3, -6}, {4, -6}, {5, -6}, {6, -6}, {7, -6}, {8, -6}, {9, -6}, {10, -6}, {11, -6}, {12, -6}, {13, -6}, {14, -6},
	{2, -7}, {5, -7}, {8, -7}, {11, -7}, {14, -7},
	{2, -8}, {3, -8}, {4, -8}, {5, -8}, {6, -8}, {7, -8}, {8, -8}, {9, -8}, {10, -8}, {11, -8}, {12, -8}, {13, -8}, {14, -8},
	{2, -9}, {5, -9}, {8, -9}, {11, -9}, {14, -9},
	{2, -10}, {3, -10}, {4, -10}, {5, -10}, {6, -10}, {7, -10}, {8, -10}, {9, -10}, {10, -10}, {11, -10}, {12, -10}, {13, -10}, {14, -10},
	{2, -11}, {5, -11}, {8, -11}, {11, -11}, {14, -11},
	{2, -12}, {3, -12}, {4, -12}, {5, -12}, {6, -12}, {7, -12}, {8, -12}, {9, -12}, {10, -12}, {11, -12}, {12, -12}, {13, -12}, {14, -12},
	{2, -13}, {5, -13}, {8, -13}, {11, -13}, {14, -13},
	{1, -14}, {2, -14}, {3, -14}, {4, -14}, {5, -14}, {6, -14}, {7, -14}, {8, -14}, {9, -14}, {10, -14}, {11, -14}, {12, -14}, {13, -14}, 
	{0, -15}, {12, -15}
};


const uint8_t flag[] = {
0x00, 0x00, 0xff, 0x12, 0x12, 0x14, 0x18, 0x10
};

const uint8_t wave[] = {
0x04, 0x04, 0x02, 0x01, 0x07, 0x05, 0x04, 0x04
};

const uint8_t lowerLine[] = {
0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
};

const uint8_t scoreLabel[] = {
0x4c, 0x92, 0x92, 0x64, 0x00, 0x70, 0x88, 0x88, 0x00, 0x70, 0x88, 0x88, 
0x70, 0x00, 0xf8, 0x08, 0x08, 0x10, 0x00, 0x70, 0xa8, 0xa8, 0xa8, 0x30
};

const uint8_t livesLabel[] = {
0xfe, 0x80, 0x80, 0x80, 0x80, 0x00, 0xe8, 0x00, 0x38, 0x40, 0x80, 0x40, 
0x38, 0x00, 0x70, 0xa8, 0xa8, 0xa8, 0x30, 0x00, 0x90, 0xa8, 0xa8, 0x48, 
0x00, 0x10, 0x30, 0x7f, 0xff, 0x7f, 0x30, 0x10
};

const uint8_t title[] = {
0x24, 0x2a, 0x2a, 0x12, 0x00, 			// S
0x1e, 0x20, 0x38, 0x20, 0x1e, 0x00,		// W
0x22, 0x3e, 0x22, 0x00,					// I
0x3e, 0x02, 0x0c, 0x02, 0x3e, 0x00, 	// M
0x3e, 0x02, 0x0c, 0x02, 0x3e, 0x00, 	// M
0x0e, 0x38, 0x0e, 0x00, 				// Y
0x00, 0x00,								// space
0x24, 0x2a, 0x2a, 0x12, 0x00, 			// S
0x3e, 0x08, 0x08, 0x3e, 0x00, 			// H
0x3c, 0x0a, 0x0a, 0x3c, 0x00, 			// A
0x3c, 0x0a, 0x0a, 0x34, 0x00,			// R
0x3e, 0x08, 0x14, 0x22, 0x00,			// K
}; // size 59

const uint8_t credits[] = {
0x3e, 0x2a, 0x2a, 0x14, 0x00,			// B
0x0e, 0x38, 0x0e, 0x00, 				// Y
0x00, 0x00,								// space
0x3c, 0x0a, 0x0a, 0x3c, 0x00, 			// A
0x3e, 0x20, 0x20, 0x20, 0x00, 			// L
0x22, 0x3e, 0x22, 0x00,					// I
0x1c, 0x22, 0x22, 0x14, 0x00,			// C
0x22, 0x3e, 0x22, 0x00,					// I
0x3c, 0x0a, 0x0a, 0x3c, 0x00, 			// A
0x00, 0x00,								// space
0x0e, 0x10, 0x20, 0x10, 0x0e, 0x00,		// V
0x3c, 0x0a, 0x0a, 0x3c, 0x00, 			// A
0x3e, 0x08, 0x10, 0x3e, 0x00, 			// N
0x00, 0x00,								// space
0x22, 0x32, 0x2a, 0x26, 0x00,			// Z
0x22, 0x3e, 0x22, 0x00,					// I
0x12, 0x22, 0x3e, 0x00,					// J
0x3e, 0x20, 0x20, 0x20, 0x00, 			// L
}; // size 77

const uint8_t pressTo[] = {
0x3c, 0x0a, 0x0a, 0x04, 0x00, 			// P
0x3c, 0x0a, 0x0a, 0x34, 0x00,			// R
0x1c, 0x2a, 0x2a, 0x22, 0x00, 			// E
0x24, 0x2a, 0x2a, 0x12, 0x00, 			// S
0x24, 0x2a, 0x2a, 0x12, 0x00, 			// S
0x00, 0x00,								// space
0x3e, 0x2a, 0x2a, 0x14, 0x00,			// B
0x02, 0x3e, 0x02, 0x00,					// T
0x3e, 0x08, 0x10, 0x3e, 0x00, 			// N
0x24, 0x3e, 0x20, 0x00,					// 1
}; // size 45

const uint8_t nextLevelDis[] = {
0x3e, 0x08, 0x10, 0x3e, 0x00, 			// N
0x1c, 0x2a, 0x2a, 0x22, 0x00, 			// E
0x36, 0x08, 0x36, 0x00, 				// X
0x02, 0x3e, 0x02, 0x00,					// T
0x00, 0x00,								// space
0x3e, 0x20, 0x20, 0x20, 0x00, 			// L
0x1c, 0x2a, 0x2a, 0x22, 0x00, 			// E
0x0e, 0x10, 0x20, 0x10, 0x0e, 0x00,		// V
0x1c, 0x2a, 0x2a, 0x22, 0x00, 			// E
0x3e, 0x20, 0x20, 0x20					// L
}; // size 45

const uint8_t gameOverDis[] = {
0x1c, 0x22, 0x2a, 0x1a, 0x00, 			// G
0x3c, 0x0a, 0x0a, 0x3c, 0x00, 			// A
0x3e, 0x02, 0x0c, 0x02, 0x3e, 0x00, 	// M
0x1c, 0x2a, 0x2a, 0x22,	0x00,			// E
0x00, 0x00, 							// space
0x1c, 0x22, 0x22, 0x1c, 0x00,			// O
0x0e, 0x10, 0x20, 0x10, 0x0e, 0x00,		// V
0x1c, 0x2a, 0x2a, 0x22, 0x00, 			// E
0x3c, 0x0a, 0x0a, 0x34 					// R
}; // size 43

const uint8_t gameWonDis[] = {
0x0e, 0x38, 0x0e, 0x00, 				// Y
0x1c, 0x22, 0x22, 0x1c, 0x00,			// O
0x1e, 0x20, 0x20, 0x1e, 0x00, 			// U
0x00, 0x00, 							// space
0x1e, 0x20, 0x38, 0x20, 0x1e, 0x00,		// W
0x22, 0x3e, 0x22, 0x00,					// I
0x3e, 0x08, 0x10, 0x3e 					// N
}; // size 30

const uint8_t btn4select[] = {
0x3e, 0x2a, 0x2a, 0x14, 0x00,			// B
0x02, 0x3e, 0x02, 0x00,					// T
0x3e, 0x08, 0x10, 0x3e, 0x00, 			// N
0x38, 0x20, 0xF8, 0x00,					// 4
0x00, 0x00, 							// space
0x24, 0x2a, 0x2a, 0x12, 0x00, 			// S
0x1c, 0x2a, 0x2a, 0x22, 0x00, 			// E
0x3e, 0x20, 0x20, 0x20,					// L
0x1c, 0x2a, 0x2a, 0x22, 0x00, 			// E
0x1c, 0x22, 0x22, 0x14, 0x00,			// C
0x02, 0x3e, 0x02, 0x00					// T
}; // size 48

const uint8_t btn3next[] = {
0x3e, 0x2a, 0x2a, 0x14, 0x00,			// B
0x02, 0x3e, 0x02, 0x00,					// T
0x3e, 0x08, 0x10, 0x3e, 0x00, 			// N
0x88, 0xA8, 0xF8, 0x00,			 		// 3
0x00, 0x00, 							// space
0x3e, 0x08, 0x10, 0x3e, 0x00, 			// N
0x1c, 0x2a, 0x2a, 0x22, 0x00, 			// E
0x36, 0x08, 0x36, 0x00, 				// X
0x02, 0x3e, 0x02, 0x00					// T
}; // size 38

const uint8_t btn1save[] = {
0x3e, 0x2a, 0x2a, 0x14, 0x00,			// B
0x02, 0x3e, 0x02, 0x00,					// T
0x3e, 0x08, 0x10, 0x3e, 0x00, 			// N
0x08, 0xF8, 0x00, 0x00,				 	// 1
0x00, 0x00, 							// space
0x24, 0x2a, 0x2a, 0x12, 0x00, 			// S
0x3c, 0x0a, 0x0a, 0x3c, 0x00, 			// A
0x0e, 0x10, 0x20, 0x10, 0x0e, 0x00,		// V
0x1c, 0x2a, 0x2a, 0x22, 0x00			// E
}; // size 41

static const uint8_t const font[] = {
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
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 94, 0, 0, 0, 0,
	0, 0, 4, 3, 4, 3, 0, 0,
	0, 36, 126, 36, 36, 126, 36, 0,
	0, 36, 74, 255, 82, 36, 0, 0,
	0, 70, 38, 16, 8, 100, 98, 0,
	0, 52, 74, 74, 52, 32, 80, 0,
	0, 0, 0, 4, 3, 0, 0, 0,
	0, 0, 0, 126, 129, 0, 0, 0,
	0, 0, 0, 129, 126, 0, 0, 0,
	0, 42, 28, 62, 28, 42, 0, 0,
	0, 8, 8, 62, 8, 8, 0, 0,
	0, 0, 0, 128, 96, 0, 0, 0,
	0, 8, 8, 8, 8, 8, 0, 0,
	0, 0, 0, 0, 96, 0, 0, 0,
	0, 64, 32, 16, 8, 4, 2, 0,
	0, 62, 65, 73, 65, 62, 0, 0,
	0, 0, 66, 127, 64, 0, 0, 0,
	0, 0, 98, 81, 73, 70, 0, 0,
	0, 0, 34, 73, 73, 54, 0, 0,
	0, 0, 14, 8, 127, 8, 0, 0,
	0, 0, 35, 69, 69, 57, 0, 0,
	0, 0, 62, 73, 73, 50, 0, 0,
	0, 0, 1, 97, 25, 7, 0, 0,
	0, 0, 54, 73, 73, 54, 0, 0,
	0, 0, 6, 9, 9, 126, 0, 0,
	0, 0, 0, 102, 0, 0, 0, 0,
	0, 0, 128, 102, 0, 0, 0, 0,
	0, 0, 8, 20, 34, 65, 0, 0,
	0, 0, 20, 20, 20, 20, 0, 0,
	0, 0, 65, 34, 20, 8, 0, 0,
	0, 2, 1, 81, 9, 6, 0, 0,
	0, 28, 34, 89, 89, 82, 12, 0,
	0, 0, 126, 9, 9, 126, 0, 0,
	0, 0, 127, 73, 73, 54, 0, 0,
	0, 0, 62, 65, 65, 34, 0, 0,
	0, 0, 127, 65, 65, 62, 0, 0,
	0, 0, 127, 73, 73, 65, 0, 0,
	0, 0, 127, 9, 9, 1, 0, 0,
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

uint8_t numbersScore[]  = {
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

int main() {
	
	start();			// calls start function to init display, timers, & IO
	while(1) {
		titleScreen();
	}
	
	return 0;
	
}

void titleScreen () {
	
	while(1) {
		
		inputHandler();			// poll inputs
		
		displayTitlePage();
		
		if(inputs & BTN_1) {
			delayms(16);
			mainMenu();				// open menu
		}
	}
	return;
}

void mainMenu() {
	
	while(1) {
		
		inputHandler();			// poll inputs

		displayMenuPage();		// display Menu Screen
		
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

void gameLoop () {
	
	gameFinished = 0;
	
	/* initialise some values for start of the game */
	spritex = 10;
	spritey = 16;
	currentLevel = 1;
	lives = 5;
	lifeCounter = 4;
	livesDisplay = 0x1F;
	score = 0;
	levelPos = 0;
	/* Update LEDs to show lives
	** Port E 7-0 are the LEDs PORTE has address 0xbf886110 */
	volatile int* porte = (volatile int*) 0xbf886110;
	*porte = 0x001f & livesDisplay;
	
	setFish();		// set fish for the level
	
	while(1) {
		
		if(gameFinished == 1) {
			return;
		}
		
		inputHandler();
	
		updateLogic();
		
		if (gameFinished == 1) {
			break;
		}
		
		if(currentLevel == 1) {
			loadLevel1(levelBuffer);
		} else if (currentLevel == 2) {
			loadLevel2(levelBuffer);
		} else if (currentLevel == 3) {
			loadLevel3(levelBuffer);
		}
		
		delayms(16);		// slow it down a bit so graphics doesn't jump
		loadGraphics();
	}
	
	return;
	
}


void inputHandler() {
	
	inputs = getInputs();            //get inputs from board
	
}

void updateLogic() {
	
	// insert functions to update the logic with buttons
	if(inputs & BTN_1) {
		if(spritedirection == 1) {
			if(spritex < 120) {
				spritex++;
			}
		} else {
			spritedirection = 1;
		}
	}
	if(inputs & BTN_4) {
		if(spritedirection == -1) {
			if(spritex > 8) {
				spritex--;
			}
		} else {
			spritedirection = -1;
		}
	}
	if(inputs & BTN_3) {
		if(spritey < 23) {
			delayms(16);
			spritey++;
		}
	}
	if(inputs & BTN_2) {
		if(spritey > 8) {
			delayms(16);
			spritey--;
		}
	}
	
	// Update sprite front for collision detection
	spritefront = spritex + (6 * spritedirection);
	
	// Display Scroll
	if(spritefront >= 100 & (inputs & BTN_1)) {
		delayms(16);
		if(levelPos < 128) {
			levelPos++;
			spritex--;
		}
	}
	
	// COLLISION DETECTION
	isCollision();
	
	// FISH CATCH DETECTION
	isCatch();
	
	// DETECT HITING END OF LEVEL FLAG
	if(levelPos == 128 & spritefront >= 118) {
		
		if (currentLevel != 3) {
			
			displayNextLevelPage();
			delayms(700);			// delay so it stays up for a short while
			
		}
		
		spritex = 10;			// reinitialise shark position
		spritey = 16;			// reinitialise shark position
		currentLevel++;			// change to next level
		levelPos = 0;			// reset screen scroll position
		setFish();				// set fish for new level
	}
	
	// DETECT GAME COMPLETED
	if(currentLevel > 3) {

		displayGameWin();
		delayms(700);			// delay so it stays up for a short while
		highScoreChecker(score);
		gameFinished = 1;		// exit game loop
	}
	
	// DETECT GAME OVER
	if(lives == 0) {

		displayGameOver();
		gameFinished = 1;		// exit game loop
		delayms(700);			// delay so it stays up for a short while
	}
	
}

// COLLISION DETECTION FUNCTION
void isCollision () {
	int obj;
	for(obj = 0; obj < 5; obj++) {
		if ( ((spritey + 4) >= levelObstacles[obj][1] & (spritey + 4) <= (levelObstacles[obj][1] + 7) ) & (spritefront == (levelObstacles[obj][0] - levelPos) | spritefront == ((levelObstacles[obj][0] - levelPos)+8*spritedirection)) ) {
			
			// COLLISION DETECTED
			delayms(128);
			livesDisplay = livesDisplay - powerOf(lifeCounter, 2);
			spritex = spritex - 4*spritedirection;
			
			// Update LEDs to show lives
			// Port E 7-0 are the LEDs PORTE has address 0xbf886110
			volatile int* porte = (volatile int*) 0xbf886110;
			*porte = 0x001f & livesDisplay;
			lifeCounter--;						// decrease lifeCounter
			lives--;							// decrease life total
			
		}
	}
}

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
	loadSprite(spritex, spritey, spritedirection, shark, displayBuffer);
	loadTiles(0, 2, 16, 128, lowerLine, displayBuffer); //load wave tiles
	loadFish (levelFish, fish, levelBuffer);
	loadLevel(levelPos, levelBuffer, displayBuffer);
	loadUI(4, 3, score, lives, scoreLabel, livesLabel, displayBuffer);
	displayUpdate(displayBuffer); //displays new buffer
}

// Updates the display with graphics from displayBuffer - screens/pages only
void displayMenuPage() {
	
	clearBuffer(512, displayBuffer); 	// Clears the display buffer
	
	display_string(0, "Press:");
	display_string(1, "BTN2 - play");
	display_string(2, "BTN4 - highscores");
	display_string(3, "                 ");
	
	display_update(textBuffer);

}

void displayTitlePage() {
	
	clearBuffer(512, displayBuffer); 	//Clears the display buffer

	loadScreen (36, 1, 59, title, displayBuffer);
	loadScreen (27, 2, 77, credits, displayBuffer);
	loadScreen (42, 3, 45, pressTo, displayBuffer);
		
	displayUpdate(displayBuffer); //displays new buffer
	
}

void displayNextLevelPage() {
	
	clearBuffer(512, displayBuffer); 	//Clears the display buffer
	
	loadScreen (42, 1, 45, nextLevelDis, displayBuffer);
	
	displayUpdate(displayBuffer); //displays new buffer
	
}

void displayGameOver() {
	
	clearBuffer(512, displayBuffer); 	//Clears the display buffer
	
	loadScreen (43, 1, 43, gameOverDis, displayBuffer);
	
	displayUpdate(displayBuffer); //displays new buffer
	
}

void displayGameWin() {
	
	clearBuffer(512, displayBuffer); 	//Clears the display buffer
	loadScreen (49, 1, 30, gameWonDis, displayBuffer);
	displayUpdate(displayBuffer); //displays new buffer
	
}

void showHighScores() {
	
	while (1) {
		
		inputHandler();						// Polls inputs 		
		
		clearBuffer(512, displayBuffer); 	// Clears the display buffer
	
		int offset;
		int i;
	
		// highest
		loadCharacter (44, 1, highScores[0][1], font, displayBuffer);
		loadCharacter (54, 1, highScores[0][2], font, displayBuffer);
		loadCharacter (64, 1, highScores[0][3], font, displayBuffer);
	
		offset = 128*1 + 64 + 10;
	
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
		loadCharacter (44, 3, highScores[2][1], font, displayBuffer);
		loadCharacter (54, 3, highScores[2][2], font, displayBuffer);
		loadCharacter (64, 3, highScores[2][3], font, displayBuffer);
	
		offset = 128*3 + 64 + 10;
	
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
	
		displayUpdate(displayBuffer); //displays new buffer
		
		
		if(inputs & BTN_1) {
			delayms(16);
			mainMenu();				// open menu
		}
	}
	
	
}


void highScoreChecker(int score) {
	
	finalScore = (score * 10);
	
	int i;
	int isHigh = 0;
	
	// Check if the new score is high
	for(i = 0; i < 3; i++) {
		if(finalScore > highScores[i][0]) {
			isHigh = 1;
		}
	}
	
	if(isHigh == 1) {
		getInitials();
	}
	
	// find minimum in current high score and replace
	int min = 0;
	for (i = 1; i < 3; i++) {
		if(highScores[i][0] < highScores[min][0]){
			min = i;
		}
	}
	
	// Replace
	highScores[min][0] = finalScore;
	highScores[min][1] = newInitial[0];
	highScores[min][2] = newInitial[1];
	highScores[min][3] = newInitial[2];
	
	// Sort highScores
	sortHighScores();
	
	gameFinished = 1;		// exit game loop
	mainMenu();
	
}

void getInitials () {
	
	// Function to get intitials
	clearBuffer(512, displayBuffer); 	// Clears the display buffer
	
	display_string(1, "New High Score!");
	display_string(2, "Enter Initials!");
	display_update(textBuffer);
	delayms(800);
	
	//Initialise display	
	int initialOne = 65;
	int initialTwo = 65;
	int initialThree = 65;
	int currentInitial = 1;
	
	clearBuffer(512, displayBuffer); 	// Clears the display buffer
	
	loadScreen (20, 0, 48, btn4select, displayBuffer);
	loadScreen (20, 1, 38, btn3next, displayBuffer);
	loadScreen (20, 2, 41, btn1save, displayBuffer);
	
	loadCharacter (44, 3, initialOne, font, displayBuffer);
	loadCharacter (54, 3, initialTwo, font, displayBuffer);
	loadCharacter (64, 3, initialThree, font, displayBuffer);
	
	displayUpdate(displayBuffer); //displays new buffer
	
	while(1) {
				
		inputHandler();			// poll inputs
		
		// Logic	
		// Change Initials
		if(inputs & BTN_4) {
			// Change Initial One
			if(currentInitial == 1) {
				if(initialOne < 90) {
					initialOne++;
					delayms(128);
				} else {
					initialOne = 65;
					delayms(128);
				}
			}
			// Change Initial Two
			if(currentInitial == 2) {
				if(initialTwo < 90) {
					initialTwo++;
					delayms(128);
				} else {
					initialTwo = 65;
					delayms(128);
				}
			}
			// Change Initial Three
			if(currentInitial == 3) {
				if(initialThree < 90) {
					initialThree++;
					delayms(128);
				} else {
					initialThree = 65;
					delayms(128);
				}
			}
		}
		// Switch Initial
		if(inputs & BTN_3) {
			if(currentInitial < 3) {
				currentInitial++;
				delayms(128);
			} else {
				currentInitial = 1;
				delayms(128);
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
		
		loadScreen (20, 0, 48, btn4select, displayBuffer);
		loadScreen (20, 1, 38, btn3next, displayBuffer);
		loadScreen (20, 2, 41, btn1save, displayBuffer);
		
		loadCharacter (44, 3, initialOne, font, displayBuffer);
		loadCharacter (54, 3, initialTwo, font, displayBuffer);
		loadCharacter (64, 3, initialThree, font, displayBuffer);
		
		displayUpdate(displayBuffer); 		//displays new buffer
		
	}
	
	return;
	
}

void sortHighScores() {
	
}

// Function from Labs

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

// Little Helper Math Functions
int powerOf (int exp, int base) {
	
	int result = 1;
	
    while (exp != 0) {
        result *= base;
        exp--;
    }
	
	return result;
	
}


