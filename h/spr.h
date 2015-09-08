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
	u16 attribute0;
	u16 attribute1;
	u16 attribute2;
	u16 attribute3;
}OAMEntry, *pOAMEntry;

//sprite rotation information (don't worry about this for now)
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

typedef struct
{
	s16 x;			//x and y position on screen
	s16 y;
	s16 w;
	s16 h;
	s16 spriteFrame[16];     //animation frame storage
	int activeFrame;        //which frame is active
	s16 index;       //which sprite referring to
	int rotData;
}Sprite;


//create an OAM variable and make it point to the address of OAM
u16* OAM = (u16*)0x7000000;

//create the array of sprites (128 is the maximum)
OAMEntry sprites[128];
pRotData rotData = (pRotData)sprites;

//Copy our sprite array to OAM
void CopyOAM()
{
	u16 i;
	u16* temp;
	temp = (u16*)sprites;
	DMA_copy(temp, OAM, 128*4, DMA_ENABLE);
}

//Set sprites to off screen
void InitializeSprites()
{
	u16 i;
	for(i = 0; i < 128; i++)
	{
		sprites[i].attribute0 = 160;  //y to > 159
		sprites[i].attribute1 = 240;  //x to > 239
	}
}

//move the sprite
void MoveSprite(Sprite sp, int x_offset=0, int y_offset=0)
{
	int tx, ty;
	tx = sp.x + x_offset;
	if(tx < 0) tx += 512;
	ty = sp.y + y_offset;
	if(ty < 0) ty += 256;
	sprites[sp.index].attribute1 = sprites[sp.index].attribute1 & 0xFE00;  //clear the old x value
	sprites[sp.index].attribute1 = sprites[sp.index].attribute1 | tx;

	sprites[sp.index].attribute0 = sprites[sp.index].attribute0 & 0xFF00;  //clear the old y value
	sprites[sp.index].attribute0 = sprites[sp.index].attribute0 | ty;
}

void RotateSprite(int rotDataIndex, s32 angle, s32 x_scale,s32 y_scale)
{
	s32 pa,pb,pc,pd;
	pa = (((1<<8)/x_scale) * COS[angle])>>8;
	pb = (((1<<8)/y_scale) * SIN[angle])>>8;
	pc = (((1<<8)/x_scale) * -SIN[angle])>>8;
	pd = (((1<<8)/y_scale) * COS[angle])>>8;

	rotData[rotDataIndex].pa = pa;
	rotData[rotDataIndex].pb = pb;
	rotData[rotDataIndex].pc = pc;
	rotData[rotDataIndex].pd = pd;
}

#endif