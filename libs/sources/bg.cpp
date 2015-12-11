#include "../gba.h"
#include "../bg.h"
#include "../sincos.h"
#include "../dispcnt.h"

//Inizializza il bg
void EnableBackground(Bg* bg)
{
	u16 temp;

	bg->tileData = (u16*)CharBaseBlock((u64)(bg->charBaseBlock));
	bg->mapData = (u16*)ScreenBaseBlock((u64)(bg->screenBaseBlock));
	temp = bg->size | (bg->charBaseBlock<<BG_CONTROL::CHAR_SHIFT) | (bg->screenBaseBlock<<BG_CONTROL::SCREEN_SHIFT)
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

void UpdateBackground(Bg bg)
{
	switch(bg.number)
	{
	case 0:
		if((REG_DISPCNT & 7) == MODE_0 || (REG_DISPCNT & 7) == MODE_1){
			REG_BG0HOFS = bg.x_scroll;
			REG_BG0VOFS = bg.y_scroll;
			REG_BG0CNT &= ~((u16)3);
			REG_BG0CNT |= (bg.priority & 3);
		}
		break;
	case 1:
		if((REG_DISPCNT & 7) == MODE_0 || (REG_DISPCNT & 7) == MODE_1){
			REG_BG1HOFS = bg.x_scroll;
			REG_BG1VOFS = bg.y_scroll;
			REG_BG1CNT &= ~((u16)3);
			REG_BG1CNT |= (bg.priority & 3);
		}
		break;
	case 2:
		REG_BG2CNT &= ~((u16)3);
		REG_BG2CNT |= (bg.priority & 3);
		//se è un rotbg
		if((REG_DISPCNT & 7) == MODE_1 || (REG_DISPCNT & 7) == MODE_2)
		{
			REG_BG2X = bg.DX;
			REG_BG2Y = bg.DY;

			REG_BG2PA = bg.PA;
			REG_BG2PB = bg.PB;
			REG_BG2PC = bg.PC;
			REG_BG2PD = bg.PD;
		}
		else //altrimenti è un text bg...
		{
			REG_BG2HOFS = bg.x_scroll;
			REG_BG2VOFS = bg.y_scroll;
		}
		break;
	case 3:
		REG_BG3CNT &= ~((u16)3);
		REG_BG3CNT |= (bg.priority & 3);
		//se è un rotbg
		if((REG_DISPCNT & 7) == MODE_2)
		{
			REG_BG3X = bg.DX;
			REG_BG3Y = bg.DY;

			REG_BG3PA = bg.PA;
			REG_BG3PB = bg.PB;
			REG_BG3PC = bg.PC;
			REG_BG3PD = bg.PD;
		}
		else if((REG_DISPCNT & 7) == MODE_0)//altrimenti è un text bg...
		{
			REG_BG3HOFS = bg.x_scroll;
			REG_BG3VOFS = bg.y_scroll;
		}
		break;
	default: break;
	}
}

//Applica modifiche al bg per poi essere updatato
void RotateBackground(Bg bg, int angle,int center_x, int center_y, FIXED zoom)
{

	center_y = (center_y * zoom)>>8;
	center_x = (center_x * zoom)>>8;

	bg.DX = ((bg.x_scroll<<8)-center_y*(FIXED)SIN[angle]-center_x*(FIXED)COS[angle]);
	bg.DY = ((bg.y_scroll<<8)-center_y*(FIXED)COS[angle]+center_x*(FIXED)SIN[angle]);

	bg.PA = ((FIXED)COS[angle]*zoom)>>8;
	bg.PB = ((FIXED)SIN[angle]*zoom)>>8; 
	bg.PC = ((FIXED)-SIN[angle]*zoom)>>8;
	bg.PD = ((FIXED)COS[angle]*zoom)>>8;
}