//////////////////////////////////////////////////////////////////////////
// File:         gd.cpp                                                 //
// Description:  Geometry Dash Game                                     //
// Author:       Francesco Mistri & Giulio Carlassare                   //
// Date:         14-08-2015                                             //
//////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <string.h>
#include "libs/gba.h"                                                       //Registri GBA e definizioni generiche
#include "libs/sincos.h"                                                    //Tabella seno e coseno
#include "libs/keypad.h"                                                    //Utility per i pulsanti
#include "libs/dispcnt.h"                                                   //Define per il REG_DISPCNT
#include "libs/spr.h"                                                       //Utility per la gestione degli sprites
#include "libs/bg.h"                                                        //utility per la gestione dei background
//Images data
#include "images/images.h"
//Backgrounds maps
#include "maps/map.h"
#include "maps/ready_map.h"
#include "maps/go_map.h"
#include "maps/you_lose_map.h"
#include "maps/you_win_map.h"
#include "maps/level.h"


int GridShift = 0; 

inline int calcCharRow(Sprite s){
	return (s.y + s.h/2) / 16;
}

inline int calcCharCol(Sprite s){
	return (s.x + GridShift + s.w/2) / 16;
}

inline bool checkTopCollision(Sprite s){
	int CharCol = calcCharCol(s);
	int CharRow = calcCharRow(s);
	int tmp = (s.x>CharCol*16?1:-1);
	if(s.y >= 0)
    	return (CharCol < lev0Width && (level0[CharRow-1][CharCol] > -1 || level0[CharRow-1][CharCol+tmp] > -1) && s.y <  16 * CharRow);
    else
    	return false;
}

inline bool checkBottomCollision(Sprite s){
	int CharCol = calcCharCol(s);
	int CharRow = calcCharRow(s);
	int tmp = (s.x>CharCol*16?1:-1);
	return (CharRow < 9 && CharCol < lev0Width && (level0[CharRow+1][CharCol] > -1 || level0[CharRow+1][CharCol+tmp] > -1) && s.y >= 16 * CharRow);
}

inline bool checkRightCollision(Sprite s){
	int CharCol = calcCharCol(s);
	int CharRow = calcCharRow(s);

	int tmp = (s.y > CharRow * 16 ? 1 : (s.y < CharRow * 16 ? -1 : 0));
    if(s.y >= 0)
	    return (CharCol < lev0Width && (level0[CharRow][CharCol+1] > -1 || level0[CharRow+tmp][CharCol+1] > -1) && s.x + GridShift >= CharCol * 16);
	else
		return false;
}                                                         //Pixel di cui è spostata la griglia di gioco

int main(){
  //+ ++++++ Variabili ++++++ +//
  //+ Uso generale +//
  int i, j;                                                                 //Indici nei cicli
  int tmp, tmp1;                                                            //Usate per valori temporanei nei calcoli
  int counter=0;                                                            //contatore(utilizzato per bg ma volendo anche ad uso generale)
  int levelcounter = 0;                                                     //contatore(utilizzato nello scorrimento del livello)
  
  //+ Background +//
  Bg background;

  background.number = 0;
  background.charBaseBlock = 0;
  background.screenBaseBlock = 31;
  background.colorMode = BG_CONTROL::COLOR256;
  background.size = BG_CONTROL::TEXTBG_SIZE_256x256;
  background.priority = 3;

  Bg text;

  text.number = 1;
  text.charBaseBlock = 0;
  text.screenBaseBlock = 30;
  text.colorMode = BG_CONTROL::COLOR256;
  text.size = BG_CONTROL::TEXTBG_SIZE_256x256;
  text.priority = 0;
  
  Bg level;
  
  level.number = 2;
  level.charBaseBlock = 1;
  level.screenBaseBlock = 29;
  level.colorMode = BG_CONTROL::COLOR256;
  level.size = BG_CONTROL::TEXTBG_SIZE_256x256;
  level.priority = 2;
 
  //+ Personaggio +//
  Sprite Character;                                    //L'oggetto per gestire il personaggio
  int CharCol, CharRow;                                //Colonna e riga del personaggio nella matrice del livello
  bool CharTopColl = 0;                                //Collisione sopra
  bool CharRightColl = 0;                              //a destra
  bool CharBotColl = 0;                                //e in basso
  int CharRotation = 0;
  FIXED VSpeed = 0;                                    //Velocita verticale (px/sec). Fixed-point: fattore 2^16 (0X100 = 1)
  const FIXED GRAVITY = 0x50;                          //Valore da aggiungere alla velocita verticale per ogni frame
  const FIXED JUMP = -0x600;                           //Velocita verticale quando salta
  const int ROT_ANGLE = 3;

  //+ Loop +//
  bool first = true;                                   //True se è la prima iterazione del main loop
  bool you_win = 0;


  //+ ++++++ Inizializzazione ++++++ +//
  SetMode(MODE_0 | OBJ_ENABLE | OBJ_MAP_1D);          //Imposta le modalità di utilizzo
  InitializeSprites();

  DMA_copy(palette, OBJPaletteMem, 256, DMA_ENABLE);  //Carica in memoria la palette degli sprites usando il DMA

  //+ ++++++ Disposizione Background ++++++ +//
  EnableBackground(&background);
  EnableBackground(&text);
  EnableBackground(&level);

  DMA_copy(tiles0Palette, BGPaletteMem, 256, DMA_ENABLE);                               //Carica la palette dei background
  DMA_copy(tiles0Data, background.tileData, tiles0_WIDTH*tiles0_HEIGHT/2, DMA_ENABLE);
  DMA_copy(blocksData, level.tileData, blocks_WIDTH*blocks_HEIGHT/2, DMA_ENABLE);
  DMA_copy(map, background.mapData, 1024, DMA_ENABLE);                                  //Mappa della disposizione dei tiles
  DMA_copy(ready_map, text.mapData, 1024, DMA_ENABLE);

  //+ ++++++ Creazione Sprites ++++++ +//
  //+ Personaggio +//
  Character.x = 100;                                                                    //Imposta le caratteristiche dell'oggetto
  Character.y = 60;
  Character.w = character_WIDTH;
  Character.h = character_HEIGHT/4;
  Character.index = 127;
  Character.activeFrame = 0;
  Character.rotData = 0;
  for(i = 0; i < 4; i++)                                                               //Imposta la posizione dei frame dell'animazione
    Character.spriteFrame[i] = i*8;

  tmp = Character.index;                                                               //Imposta attributi dello sprite nella memoria
  sprites[tmp].attribute0 = SPR_ATTR0::COLOR256 | SPR_ATTR0::SQUARE | Character.y | SPR_ATTR0::ROTATION | SPR_ATTR0::SIZE_DOUBLE;
  sprites[tmp].attribute1 = SPR_ATTR1::SIZE16 | Character.x | ROTDATA(Character.rotData);
  sprites[tmp].attribute2 = 0 | PRIORITY(1);
  RotateSprite(Character.rotData, 0, 1, 1);

  tmp1 = character_WIDTH * character_HEIGHT / 2;                                       //Dimensione del BMP in memoria
  DMA_copy(characterData, OAMData, tmp1, DMA_ENABLE);                                  //Carica l'immagine in memoria

  //+ Livello +//
  int blockTypes[][4] = {{0x02,0x03,0x04,0x05,},};                                     //Matrice dei tile dei blocchi
  
  s16 level0tmp[32*32];                                                                //Mappa dei tile temporanea
  for(j = 0; j<32; j+=2)                                                               //Scorro la matrice del livello
  {
    for(i = 0; i<20; i+=2)
    {
      if(level0[i/2][j/2]==-1)                                                         //Se trovo -1(spazio vuoto ) sovrascrivo con un tile vuoto
      {
        level0tmp[j+i*32] = 0x00;
        level0tmp[j+i*32+1] = 0x00;
        level0tmp[j+(1+i)*32] = 0x00;
        level0tmp[j+(1+i)*32+1] = 0x00;
        
      }
      else                                                                             //Altrimenti attingo alla matrice dei blocchi
      {
        level0tmp[j+i*32] = blockTypes[level0[i/2][j/2]][0];
        level0tmp[j+i*32+1] = blockTypes[level0[i/2][j/2]][1];
        level0tmp[j+(1+i)*32] = blockTypes[level0[i/2][j/2]][2];
        level0tmp[j+(1+i)*32+1] = blockTypes[level0[i/2][j/2]][3];
        
      }
    }
  }
  DMA_copy(level0tmp,level.mapData,1024,DMA_ENABLE);                                   //Aggiorno la mappa del livello

  //+ ++++++ Loop principale ++++++ +//
  while(true){
    //+ Personaggio +//
    Character.y += (VSpeed >= 0 ? (VSpeed>>8) : -((-VSpeed)>>8));                     //Aggiorno la coordinata verticale
    CharRotation += ROT_ANGLE;
    if(CharRotation >= 360) CharRotation -= 360;

    CharRow = calcCharRow(Character);                                                 //Riga e colonne nella matrice
    CharCol = calcCharCol(Character);

    //Controlla le collisioni vericali
    CharBotColl = checkBottomCollision(Character);
    CharTopColl = checkTopCollision(Character);

    if(CharTopColl || CharBotColl){
      for(tmp = CharRotation; tmp >= 90; tmp -= 90);
      if(tmp < 45)
        CharRotation -= tmp;
      else
        CharRotation += 90-tmp;
    }
    RotateSprite(Character.rotData, CharRotation, 1, 1);

    if(CharTopColl){
      VSpeed = 0;
      Character.y = CharRow * 16;
    }

    if(CharBotColl){
      VSpeed = 0;
      Character.y = CharRow * 16;
    }else{
      VSpeed += GRAVITY;
    }

    if(CheckPressed(KEY_A) && CharBotColl)
      VSpeed = JUMP;

    //Calcolo della collisione orizzontale
    CharRightColl = checkRightCollision(Character);
    MoveSprite(Character , -Character.w/2, -Character.h/2);                     //Scrivo le modifiche nella memoria
                                                                                //valori dimezzati perche usata doppia area di rendering


    //+ Livello +//
    GridShift += 2;
    level.x_scroll=GridShift;
    if(GridShift%16 == 0)                                                      //Se una fila di blocchi è uscita dallo schermo
    {
      
      for(i = 0;i<20;i+=2)                                                     //La sovrascrivo con una nuova
      {
        if(level0[i/2][15+GridShift/16]==-1)
        {
          level0tmp[levelcounter+i*32] = 0x00;
          level0tmp[levelcounter+i*32+1] = 0x00;
          level0tmp[levelcounter+(1+i)*32] = 0x00;
          level0tmp[levelcounter+(1+i)*32+1] = 0x00;
          
        }
        else
        {
          level0tmp[levelcounter+i*32] = blockTypes[level0[i/2][15+GridShift/16]][0];
          level0tmp[levelcounter+i*32+1] = blockTypes[level0[i/2][15+GridShift/16]][1];
          level0tmp[levelcounter+(1+i)*32] = blockTypes[level0[i/2][15+GridShift/16]][2];
          level0tmp[levelcounter+(1+i)*32+1] = blockTypes[level0[i/2][15+GridShift/16]][3];
          
        }
      }
      levelcounter+=2;                      //Determina quale colonna della mappa è effettivamente uscita dallo schermo
      if(levelcounter>=32)
        levelcounter = 0;
    }
    
    //Gestione movimento background
    counter++;
    if(counter%4 == 0)                      //Muove gradualmente il bg
    {
      background.x_scroll++;
    }
    
    if(background.x_scroll >= 256)
      background.x_scroll = 256;

    //+ Disegno +//
    WaitForVsync();
    UpdateBackground(level);
    UpdateBackground(background);
    UpdateBackground(text);
    DMA_copy(level0tmp,level.mapData,1024,DMA_ENABLE);
    CopyOAM();
    
    if(CharCol > lev0Width || CharRow >= 10){
      you_win = 1;
      break;
    }
    if(CharRightColl)
    {
      for(i = 0; i<3; i++)
      {
        Character.activeFrame++;
        sprites[Character.index].attribute2 = Character.spriteFrame[Character.activeFrame];
        sleep(51);
        WaitForVsync();
        CopyOAM();
      }
      sleep(51);
      Character.x = 250;
      MoveSprite(Character, -Character.w/2, -Character.h/2);
      CopyOAM();
      break;
    }

    if(first){
      first = 0;
      for(i=0; i < 2; i++){
        sleep(512);
        text.priority = 3;
        WaitForVsync();
        UpdateBackground(text);
        sleep(512);
        text.priority = 0;
        WaitForVsync();
        UpdateBackground(text);
      }
      WaitForVsync();
      DMA_copy(go_map, text.mapData, 1024, DMA_ENABLE);
      sleep(720);
      text.priority = 3;
      WaitForVsync();
      UpdateBackground(text);
    }
  }

  WaitForVsync();
  DMA_copy((you_win?you_win_map:you_lose_map), text.mapData, 1024, DMA_ENABLE);
  text.priority = 0;
  UpdateBackground(text);

  while(true);
  return 0;
}
