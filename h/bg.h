	#ifndef BG_H
#define BG_H

FIXED angle = 0;
FIXED zoom = 1<<8;  //lo zoom è un numero a virgola fissa(1<<8 = zoomx1)

//BGCNT defines
#define BG_MOSAIC_ENABLE		0x40
#define BG_COLOR_256			0x80
#define BG_COLOR_16			0x0

#define CharBaseBlock(n)		(((n)*0x4000)+0x6000000)
#define ScreenBaseBlock(n)		(((n)*0x800)+0x6000000)

#define CHAR_SHIFT			2
#define SCREEN_SHIFT			8
#define TEXTBG_SIZE_256x256		0x0
#define TEXTBG_SIZE_256x512		0x8000
#define TEXTBG_SIZE_512x256		0x4000
#define TEXTBG_SIZE_512x512		0xC000

#define ROTBG_SIZE_128x128		0x0
#define ROTBG_SIZE_256x256		0x4000
#define ROTBG_SIZE_512x512		0x8000
#define ROTBG_SIZE_1024x1024		0xC000

#define WRAPAROUND              	0x200

typedef struct Bg
{
	Bg(){
		mosaic = colorMode = number = size = charBaseBlock = screenBaseBlock = 0;
		wraparound = x_scroll = y_scroll = DX = DY = PA = PB = PC = PD = 0;
	}
	u16* tileData;
	u16* mapData;
	u8 mosaic;
	u8 colorMode;
	u8 number;
	u16 size;
	u8 charBaseBlock;
	u8 screenBaseBlock;
	u8 wraparound;
	s16 x_scroll,y_scroll;
	s32 DX,DY;
	s16 PA,PB,PC,PD;
}Bg;


void EnableBackground(Bg* bg)           //inizializza il bg
{
	u16 temp;

	bg->tileData = (u16*)CharBaseBlock(bg->charBaseBlock);
	bg->mapData = (u16*)ScreenBaseBlock(bg->screenBaseBlock);
	temp = bg->size | (bg->charBaseBlock<<CHAR_SHIFT) | (bg->screenBaseBlock<<SCREEN_SHIFT)
		| bg->colorMode | bg->mosaic | bg->wraparound;

	switch(bg->number)
	{
	case 0:
		{
			REG_BG0CNT = temp;
			REG_DISPCNT |= BG0_ENABLE;
		}break;
	case 1:
		{
			REG_BG1CNT = temp;
			REG_DISPCNT |= BG1_ENABLE;
		}break;
	case 2:
		{
			REG_BG2CNT = temp;
			REG_DISPCNT |= BG2_ENABLE;
		}break;
	case 3:
		{
			REG_BG3CNT = temp;
			REG_DISPCNT |= BG3_ENABLE;
		}break;

	default:break;

	}
}

void UpdateBackground(Bg* bg)
{
	switch(bg->number)
	{
	case 0:
		REG_BG0HOFS = bg->x_scroll;
		REG_BG0VOFS = bg->y_scroll;
		break;
	case 1:
		REG_BG1HOFS = bg->x_scroll;
		REG_BG1VOFS = bg->y_scroll;
		break;
	case 2:
		if(!(REG_DISPCNT & MODE_0))//se è un rotational bg...
		{
			REG_BG2X = bg->DX;
			REG_BG2Y = bg->DY;

			REG_BG2PA = bg->PA;
			REG_BG2PB = bg->PB;
			REG_BG2PC = bg->PC;
			REG_BG2PD = bg->PD;
		}
		else //altrimenti è un text bg...
		{
			REG_BG2HOFS = bg->x_scroll;
			REG_BG2VOFS = bg->y_scroll;
		}
		break;
	case 3:
		if(!(REG_DISPCNT & MODE_0))//se è un rotational bg...
		{
			REG_BG3X = bg->DX;
			REG_BG3Y = bg->DY;

			REG_BG3PA = bg->PA;
			REG_BG3PB = bg->PB;
			REG_BG3PC = bg->PC;
			REG_BG3PD = bg->PD;
		}
		else //altrimenti è un text bg...
		{
			REG_BG3HOFS = bg->x_scroll;
			REG_BG3VOFS = bg->y_scroll;
		}
		break;
	default: break;
	}
}
/*
void RotateBackground(Bg* bg, int angle,int center_x, int center_y, FIXED zoom)   //Applica modifiche al bg per poi essere updatato
{

	center_y = (center_y * zoom)>>8;
	center_x = (center_x * zoom)>>8;

	bg->DX = ((bg->x_scroll<<8)-center_y*(FIXED)SIN[angle]-center_x*(FIXED)COS[angle]);
	bg->DY = ((bg->y_scroll<<8)-center_y*(FIXED)COS[angle]+center_x*(FIXED)SIN[angle]);

	bg->PA = ((FIXED)COS[angle]*zoom)>>8;
	bg->PB = ((FIXED)SIN[angle]*zoom)>>8; 
	bg->PC = ((FIXED)-SIN[angle]*zoom)>>8;
	bg->PD = ((FIXED)COS[angle]*zoom)>>8;
}*/
#endif
