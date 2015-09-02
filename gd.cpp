//////////////////////////////////////////////////////////////////////////
// File:         gd.cpp                                                 //
// Description:  Geometry Dash Game                                     //
// Author:       Francesco Mistri & Giulio Carlassare                   //
// Date:         14-08-2015                                             //
//////////////////////////////////////////////////////////////////////////

#include <math.h>
#include "h/gba.h"                                                          //registri GBA e definizioni generiche
#include "h/sincos.h"                                                       //Tabella seno e coseno
#include "h/keypad.h"                                                       //registri dei bottoni
#include "h/dispcnt.h"                                                      //define per il REG_DISPCNT
#include "h/spr.h"                                                          //funzioni e tipi utili per gestione sprites
#include "h/bg.h"                                                           //background useful functions and types
//Sprites
#include "h/character.h"                                                    //character sprite
#include "h/block1.h"                                                       //block number 1
#include "h/palette.h"                                                      //palette(sprites)
//Backgrounds
#include "h/tiles0.h"                                                       //first(last?) set of tiles used
#include "c/map.c"                                                          //background map(using tiles0)
#include "c/ready_map.c"
#include "c/go_map.c"
#include "c/you_lose_map.c"
#include "c/you_win_map.c"
//Others
#include "h/level.h"
#include <string.h>


//Aggiunge un blocco al vettore e imposta tutti gli attributi
void AddBlock(Sprite Blocks[], int index, int type, int row){
  Blocks[index].x = 240;          //Posizionato a destra furoi dallo schermo
  Blocks[index].y = 16 * row;     //La sua coordinata verticale
  Blocks[index].w = block1_WIDTH;
  Blocks[index].h = block1_HEIGHT;
  Blocks[index].index = index;

  sprites[index].attribute0 = COLOR_256 | SQUARE | Blocks[index].y;
  sprites[index].attribute1 = SIZE_16 | Blocks[index].x;
  sprites[index].attribute2 = type | PRIORITY(1);
}

int main(){
  //+ ++++++ Variabili ++++++ +//
  //+ Uso generale +//
  int i, j, k, ii;                    //Indici nei cicli
  int tmp, tmp1, tmp2;            //Usate per valori temporanei nei calcoli
  int counter=0;                  //contatore(utilizzato per bg ma volendo anche ad uso generale)
  
  //+ Background +//
  Bg background;

  background.number = 0;
  background.charBaseBlock = 0;
  background.screenBaseBlock = 31;
  background.colorMode = BG_COLOR_256;
  background.size = TEXTBG_SIZE_256x256;

  Bg text;

  text.number = 1;
  text.charBaseBlock = 0;
  text.screenBaseBlock = 30;
  text.colorMode = BG_COLOR_256;
  text.size = TEXTBG_SIZE_256x256;
  text.priority = 0;

  //+ Personaggio +//
  Sprite Character;               //L'oggetto per gestire il personaggio
  int CharC, CharR;               //Colonna e riga del personaggio nella matrice del livello
  bool CharTopColl = 0;           //Collisione sopra
  bool CharRightColl = 0;         //a destra
  bool CharBotColl = 0;           //e in basso
  FIXED VSpeed = 0;               //Velocita verticale (px/sec). Fixed-point: fattore 2^16 (0X100 = 1)
  const FIXED GRAVITY = 0x80;     //Valore da aggiungere alla velocita verticale per ogni frame
  const FIXED JUMP = -0x700;      //Velocita verticale quando salta

  //+ Blocchi +//
  Sprite Blocks[127];             //Vettore per contenere le informazioni dei blocchi
  int BlockTypes[] = {32};        //Posizione in memoria del BMP per ogni tipo di blocco
  int index = 0;                  //Usato per la creazione dei blocchi
  bool BlocksUsed[127];           //Segna quali posizioni del vettore Blocks possono essere usate (0 se possono essere)
  memset(BlocksUsed, 0, sizeof(BlocksUsed));

  //+ Loop +//
  int WindowLeft, OldWindowLeft=0;  //L'indice della prima colonna intera della matrice visualizzata nello schermo
  int GridShift = 0;                //Numero di px di cui la griglia si è spostata a sinistra
  const int SHIFTPF = 2;            //Pixel di cui si sposta la griglia ad ogni frame
  const int MAXCOL = 16;            //Numero massimo di colonne visibili contemporaneamente
  bool first = true;                //True se è la prima iterazione del main loop
  bool you_win = 0;


  //+ ++++++ Inizializzazione ++++++ +//
  SetMode(MODE_0 | OBJ_ENABLE | OBJ_MAP_1D);    //Imposta le modalità di utilizzo
  InitializeSprites();                          //Inizializza gli sprite

  DMA_copy(palette, OBJPaletteMem, 256, DMA_ENABLE);  //Carica in memoria la palette degli sprites usando il DMA

  //+ ++++++ Disposizione Background ++++++ +//
  EnableBackground(&background);
  EnableBackground(&text);

  DMA_copy(tiles0Palette, BGPaletteMem, 256, DMA_ENABLE);       //Carica la palette dei background
  DMA_copy(tiles0Data, background.tileData, tiles0_WIDTH*tiles0_HEIGHT/2, DMA_ENABLE);
  DMA_copy(map, background.mapData, 1024, DMA_ENABLE);   //Mappa della disposizione dei tiles
  DMA_copy(ready_map, text.mapData, 1024, DMA_ENABLE);

  //+ ++++++ Creazione Sprites ++++++ +//
  //+ Personaggio +//
  Character.x = 100;                           //Imposta le caratteristiche dell'oggetto
  Character.y = 60;
  Character.w = character_WIDTH;
  Character.h = character_HEIGHT/4;
  Character.index = 127;
  Character.activeFrame = 0;
  for(i = 0; i < 4; i++)                       //Imposta la posizione dei frame dell'animazione
    Character.spriteFrame[i] = i*8;

  tmp = Character.index;                      //Imposta attributi dello sprite nella memoria
  sprites[tmp].attribute0 = COLOR_256 | SQUARE | Character.y;
  sprites[tmp].attribute1 = SIZE_16 | Character.x;
  sprites[tmp].attribute2 = 0 | PRIORITY(1);

  tmp1 = character_WIDTH * character_HEIGHT / 2;       //Dimensione del BMP in memoria
  DMA_copy(characterData, OAMData, tmp1, DMA_ENABLE);   //Carica l'immagine in memoria

  //+ Blocchi +//
  for(i = 0; i < 10; i++){                    //Scorro le righe della matrice del livello
    for(j = 0; j < levWidth && j <= MAXCOL; j++){  //e le colonne ma solo per una schermata
      if(level[i][j] > -1){                   //Se nella cella corrente c'e qualcosa
        tmp = BlockTypes[level[i][j]];
        AddBlock(Blocks, index, tmp, i);      //aggiungo unn blocco cone le giuste caratteristiche
        level[i][j] = index;                  //Imposto nella matrice l'indice nel vettore
        BlocksUsed[index] = 1;
        index++;
      }
    }
  }

  tmp2 = block1_WIDTH * block1_HEIGHT / 2;     //Dimensione in memoria dell'immagine
  DMA_copy(block1Data, &OAMData[tmp1], tmp2, DMA_ENABLE); //Carica l'immagine in memoria

  //+ ++++++ Loop principale ++++++ +//
  while(true){
    //+ Personaggio +//
    Character.y += (VSpeed >= 0 ? (VSpeed>>8) : -((-VSpeed)>>8));                 //Aggiorno la coordinata verticale

    CharR = (Character.y + Character.h/2) / 16; //Riga e colonne nella matrice
    CharC = (Character.x + GridShift + Character.w/2) / 16;

    tmp = (Character.x>CharC*16?1:-1);          //Controlla le collisioni vericali
    CharBotColl = (CharR < 9 && CharC < levWidth && (level[CharR+1][CharC] > -1 || level[CharR+1][CharC+tmp] > -1) && Character.y >= 16 * CharR);
    CharTopColl = (CharC < levWidth && (level[CharR-1][CharC] > -1 || level[CharR-1][CharC+tmp] > -1) && Character.y <  16 * CharR);

    if(CharTopColl){
      VSpeed = 0;
      Character.y = CharR * 16;
    }

    if(CharBotColl){
      VSpeed = 0;
      Character.y = CharR * 16;
    }else{
      VSpeed += GRAVITY;
    }

    if(CheckPressed(KEY_A) && CharBotColl)
      VSpeed = JUMP;

    //Calcolo della collisione orizzontale
    tmp = (Character.y > CharR * 16 ? 1 : (Character.y < CharR * 16 ? -1 : 0));
    CharRightColl = (CharC < levWidth && (level[CharR][CharC+1] > -1 || level[CharR+tmp][CharC+1] > -1) && Character.x + GridShift >= CharC * 16);
    MoveSprite(Character);                     //Scrivo le modifiche nella memoria


    //+ Blocchi +//
    //Arrotondato per difetto
    WindowLeft = GridShift / 16;

    //Sposta i blocchi sotto lo schermo se sono usciti dalla visuale
    if(WindowLeft > 0){
      for(i = 0; i < 10; i++){
        tmp = level[i][WindowLeft-1];         //Quello che c'e nella cella corrente
        if(tmp > -1){
          Blocks[tmp].y = 160;
          MoveSprite(Blocks[tmp]);
          BlocksUsed[tmp] = 0;
        }
        level[i][WindowLeft-1] = -1;
      }
    }

    //Carica i nuovi blocchi
    if(OldWindowLeft != WindowLeft){
      OldWindowLeft = WindowLeft;
      for(i = 0; i < 10; i++){
        tmp = level[i][WindowLeft + MAXCOL];
        if(tmp > -1){
          tmp = BlockTypes[tmp];
          for(j = 0; j < 127 && BlocksUsed[j] != 0; j++);                 //Ricerca il primo posto libero nel vettore
          AddBlock(Blocks, j, tmp, i);
          level[i][WindowLeft+MAXCOL] = j;
          BlocksUsed[j] = 1;
        }
      }
    }

    //Imposta la posizione dei blocchi sullo schermo
    for(j = WindowLeft; j <= WindowLeft + MAXCOL && j < levWidth; j++){
      for(i = 0; i < 10; i++){
        tmp = level[i][j];
        if(tmp > -1){
          tmp1 = 16 * j - GridShift;
          Blocks[tmp].x = (tmp1 >= 0 ? tmp1 : 512+tmp1);
          MoveSprite(Blocks[tmp]);
        }
      }
    }

    GridShift += SHIFTPF;

    //Gestione movimento background
    counter++;
    if(counter%4 == 0)                  //Muove gradualmente il bg
    {
      background.x_scroll++;
    }
    
    if(background.x_scroll >= 256)
      background.x_scroll = 256;

    //+ Disegno +//
    WaitForVsync();
    UpdateBackground(&background);
    UpdateBackground(&text);
    CopyOAM();
    
    if(CharC > levWidth || CharR >= 10){
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
      MoveSprite(Character);
      CopyOAM();
      break;
    }

    if(first){
      first = 0;
      for(i=0; i < 2; i++){
        sleep(512);
        text.priority = 3;
        WaitForVsync();
        UpdateBackground(&text);
        sleep(512);
        text.priority = 0;
        WaitForVsync();
        UpdateBackground(&text);
      }
      WaitForVsync();
      DMA_copy(go_map, text.mapData, 1024, DMA_ENABLE);
      sleep(720);
      text.priority = 3;
      WaitForVsync();
      UpdateBackground(&text);
    }
  }

  WaitForVsync();
  DMA_copy((you_win?you_win_map:you_lose_map), text.mapData, 1024, DMA_ENABLE);
  text.priority = 0;
  UpdateBackground(&text);

  while(true);
  return 0;
}
