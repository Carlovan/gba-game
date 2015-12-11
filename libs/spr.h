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
struct SPR_ATTR0{
	static const int ROTATION         = 0x100;
	static const int SIZE_DOUBLE      = 0x200;
	static const int MODE_NORMAL      = 0x0;
	static const int MODE_TRANSPARENT = 0x400;
	static const int MOSAIC           = 0x1000;
	static const int COLOR16          = 0x0;
	static const int COLOR256         = 0x2000;
	static const int SQUARE           = 0x0;
	static const int TALL             = 0x4000;
	static const int WIDE             = 0x8000;
};

//attribute1 #defines
struct SPR_ATTR1{
	static const int HORIZONTAL_FLIP = 0x1000;
	static const int VERTICAL_FLIP   = 0x2000;
	static const int SIZE8           = 0x0;
	static const int SIZE16          = 0x4000;
	static const int SIZE32          = 0x8000;
	static const int SIZE64          = 0xC000;
};
#define ROTDATA(n)          ((n)<<9)

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