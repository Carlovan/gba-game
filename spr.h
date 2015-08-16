////////////////////////////////////////////////////////////////////////
// File: 	ch5.h                                                 //
// Description: contains all the definitions and structures necessary //
//		for displaying sprites on the GBA		      //
// Author:	dovoto (modified by gbajunkie)	                      //
// Date: 	7th February 2002                                     //
////////////////////////////////////////////////////////////////////////
#ifndef SQUARES_H
#define SQUARES_H

//define the screen width and height values to be used
#define SCREEN_WIDTH	240
#define SCREEN_HEIGHT   160

//colour convertion (converts a RGB colour to a 15-bit BGR value used by the GBA)
#define RGB16_BGR(r, g, b) ((r)+(g<<5)+(b<<10))

//attribute0 #defines
#define ROTATION_FLAG		0x100
#define SIZE_DOUBLE		0x200
#define MODE_NORMAL		0x0
#define MODE_TRANSPARENT	0x400
#define MODE_WINDOWED		0x800
#define MOSAIC			0x1000
#define COLOR_16		0x0000
#define COLOR_256		0x2000
#define SQUARE			0x0
#define TALL			0x4000
#define WIDE			0x8000

//attribute1 #defines
#define ROTDATA(n)		((n)<<9)
#define HORIZONTAL_FLIP		0x1000
#define VERTICAL_FLIP		0x2000
#define SIZE_8			0x0
#define SIZE_16			0x4000
#define SIZE_32			0x8000
#define SIZE_64			0xC000

//atrribute2 #defines
#define PRIORITY(n)	        ((n)<<10)
#define PALETTE(n)		((n)<<12)

//sprite structure definitions
typedef struct tagOAMEntry
{
	int attribute0;
	int attribute1;
	int attribute2;
	int attribute3;
}OAMEntry, *pOAMEntry;

//sprite rotation information (don't worry about this for now)
typedef struct tagRotData
{
	int filler1[3];
	int pa;
	int filler2[3];
	int pb;
	int filler3[3];
	int pc;
	int filler4[3];
	int pd;
}RotData, *pRotData;

typedef struct
{
	s16 x;			//x and y position on screen
	s16 y;
	s16 w;
	s16 h;
	s16 spriteFrame[3];     //animation frame storage
	int activeFrame;        //which frame is active
	s16 index;       //which sprite referring to
}Sprite;


//create an OAM variable and make it point to the address of OAM
int* OAM = (int*)0x7000000;

//create the array of sprites (128 is the maximum)
OAMEntry sprites[128];

//Copy our sprite array to OAM
void CopyOAM()
{
	int i;
	int* temp;
	temp = (int*)sprites;
	for(i = 0; i < 128*4; i++)
	{
		OAM[i] = temp[i];
	}
}

//Set sprites to off screen
void InitializeSprites()
{
	int i;
	for(i = 0; i < 128; i++)
	{
		sprites[i].attribute0 = 160;  //y to > 159
		sprites[i].attribute1 = 240;  //x to > 239
	}
}

//move the sprite
void MoveSprite(Sprite sp)
{
	sprites[sp.index].attribute1 = sprites[sp.index].attribute1 & 0xFE00;  //clear the old x value
	sprites[sp.index].attribute1 = sprites[sp.index].attribute1 | sp.x;

	sprites[sp.index].attribute0 = sprites[sp.index].attribute0 & 0xFF00;  //clear the old y value
	sprites[sp.index].attribute0 = sprites[sp.index].attribute0 | sp.y;
}

#endif



