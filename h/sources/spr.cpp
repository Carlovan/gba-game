#include "../gba.h"
#include "../spr.h"
#include "../sincos.h"

//create the array of sprites (128 is the maximum)
OAMEntry sprites[128];
pRotData rotData = (pRotData)sprites;

void InitializeSprites(){
	for(int i = 0; i < 128; i++){
		sprites[i].attribute1 = SCREEN_WIDTH  + 1;
		sprites[i].attribute0 = SCREEN_HEIGHT + 1;
	}
}

//Copy our sprite array to OAM
void CopyOAM()
{
	u16* temp;
	temp = (u16*)sprites;
	DMA_copy(temp, OAMmem, 128*4, DMA_ENABLE);
}

//Move the sprite
void MoveSprite(Sprite sp, int x_offset=0, int y_offset=0)
{
	int tx, ty;

	tx = sp.x + x_offset;
	if(tx < 0) tx += 512;

	ty = sp.y + y_offset;
	if(ty < 0) ty += 256;

	sprites[sp.index].attribute1 &= 0xFE00;               //clear the old x value
	sprites[sp.index].attribute1 |= tx;

	sprites[sp.index].attribute0 &= 0xFF00;               //clear the old y value
	sprites[sp.index].attribute0 |= ty;
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