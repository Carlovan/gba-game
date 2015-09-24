////////////////////////////////////////////////////////////////////////
// File:  spr.h                                                       //
// Description: contains all the definitions and structures necessary //
//    for displaying sprites on the GBA                               //
// Author:  dovoto (modified by gbajunkie)                            //
// Date:  7th February 2002                                           //
////////////////////////////////////////////////////////////////////////
#ifndef SPRITES_H
#define SPRITES_H

//Colour convertion (converts a RGB colour to a 15-bit BGR value used by the GBA)
#define RGB16_BGR(r, g, b)  ((r)+(g<<5)+(b<<10))

//attribute0 #defines
#define ROTATION_FLAG       0x100
#define SIZE_DOUBLE         0x200
#define MODE_NORMAL         0x0
#define MODE_TRANSPARENT    0x400
#define MODE_WINDOWED       0x800
#define MOSAIC              0x1000
#define COLOR_16            0x0000
#define COLOR_256           0x2000
#define SQUARE              0x0
#define TALL                0x4000
#define WIDE                0x8000

//attribute1 #defines
#define ROTDATA(n)          ((n)<<9)
#define HORIZONTAL_FLIP     0x1000
#define VERTICAL_FLIP       0x2000
#define SIZE_8              0x0
#define SIZE_16             0x4000
#define SIZE_32             0x8000
#define SIZE_64             0xC000

//atrribute2 #defines
#define PRIORITY(n)         ((n)<<10)
#define PALETTE(n)          ((n)<<12)

//sprite structure definitions
typedef struct tagOAMEntry
{
	u16 attribute0;
	u16 attribute1;
	u16 attribute2;
	u16 attribute3;
}OAMEntry, *pOAMEntry;

//sprite rotation information
typedef struct tagRotData
{
	u16 filler1[3];
	s16 pa;
	u16 filler2[3];
	s16 pb;
	u16 filler3[3];
	s16 pc;
	u16 filler4[3];
	s16 pd;
}RotData, *pRotData;

typedef struct Sprite
{
	Sprite(){
		x = SCREEN_WIDTH  + 1;
		y = SCREEN_HEIGHT + 1;
		w = h = activeFrame = index = rotData = 0;
	}
	s16 x;                    //x and y position on screen
	s16 y;
	s16 w;                    //width and height
	s16 h;
	s16 spriteFrame[16];      //animation frame storage
	int activeFrame;          //which frame is active
	s16 index;                //which sprite referring to
	int rotData;
}Sprite;

extern OAMEntry sprites[128];
extern pRotData rotData;

void InitializeSprites();

//Copy our sprite array to OAM
void CopyOAM();

//Move the sprite
void MoveSprite(Sprite sp, int x_offset, int y_offset);

void RotateSprite(int rotDataIndex, s32 angle, s32 x_scale,s32 y_scale);

#endif