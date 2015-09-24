#ifndef BG_H
#define BG_H

//BGCNT defines
#define BG_MOSAIC_ENABLE      0x40
#define BG_COLOR_256          0x80
#define BG_COLOR_16           0x0

#define CHAR_SHIFT            0x2
#define SCREEN_SHIFT          0x8
#define TEXTBG_SIZE_256x256   0x0
#define TEXTBG_SIZE_256x512   0x8000
#define TEXTBG_SIZE_512x256   0x4000
#define TEXTBG_SIZE_512x512   0xC000

#define ROTBG_SIZE_128x128    0x0
#define ROTBG_SIZE_256x256    0x4000
#define ROTBG_SIZE_512x512    0x8000
#define ROTBG_SIZE_1024x1024  0xC000

#define BG_WRAP               0x200

#define CharBaseBlock(n)      (((n)*0x4000)+0x6000000)
#define ScreenBaseBlock(n)    (((n)*0x800)+0x6000000)

typedef struct Bg
{
	Bg(){
		mosaic = colorMode = number = size = charBaseBlock = screenBaseBlock = 0;
		wraparound = x_scroll = y_scroll = DX = DY = PB = PD = 0;
		PA = PC = 1;
		priority = 3;
	}
	u16* tileData;
	u16* mapData;
	u8 mosaic;
	u8 colorMode;
	u16 number;
	u16 size;
	u16 charBaseBlock;
	u16 screenBaseBlock;
	u8 wraparound;
	s16 x_scroll,y_scroll;
	s32 DX,DY;
	s16 PA,PB,PC,PD;
	u16 priority;
}Bg;

//Inizializza il bg
void EnableBackground(Bg* bg);

void UpdateBackground(Bg bg);

//Applica modifiche al bg per poi essere updatato
void RotateBackground(Bg bg, int angle,int center_x, int center_y, FIXED zoom);

#endif
