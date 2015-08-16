//////////////////////////////////////////////////////////////////////////
// File:         gd.cpp                                                 //
// Description:  Geometry Dash Game                                     //
// Author:       Francesco Mistri & Giulio Carlassare                   //
// Date:         14-08-2015                                             //
//////////////////////////////////////////////////////////////////////////

#include "gba.h"                                                          //GBA register definitions and generic functions
#include "keypad.h"                                                       //button registers
#include "dispcnt.h"                                                      //REG_DISPCNT register #defines
#include "spr.h"                                                          //sprite useful functions and types
#include "character.h"                                                    //character sprite
#include "block1.h"                                                       //block number 1
#include "palette.h"                                                      //palette
#include "level.h"

//Aggiunge un blocco al vettore e imposta tutti gli attributi
void AddBlock(Sprite Blocks[], int index, int type, int row){
  Blocks[index].x = 240;          //Posizionato a destra furoi dallo schermo
  Blocks[index].y = 16 * row;     //La sua coordinata verticale
  Blocks[index].w = block1_WIDTH;
  Blocks[index].h = block1_HEIGHT;
  Blocks[index].index = index;

  sprites[index].attribute0 = COLOR_256 | SQUARE | Blocks[index].y;
  sprites[index].attribute1 = SIZE_16 | Blocks[index].x;
  sprites[index].attribute2 = type;
}

int main(){
  //+ ++++++ Variabili ++++++ +//
  //+ Uso generale +//
  int i, j, k;                    //Indici nei cicli
  int tmp, tmp1, tmp2;            //Usate per valori temporanei nei calcoli

  //+ Personaggio +//
  Sprite Character;               //L'oggetto per gestire il personaggio
  float VSpeed = 0;               //Velocita verticale (px/sec)
  int CharC, CharR;               //Colonna e riga del personaggio nella matrice del livello
  bool CharTopColl = 0;           //Collisione sopra
  bool CharRightColl = 0;         //a destra
  bool CharBotColl = 0;           //e in basso

  //+ Blocchi +//
  Sprite Blocks[127];             //Vettore per contenere le informazioni dei blocchi
  int BlockTypes[] = {8};         //Posizione in memoria del BMP per ogni tipo di blocco
  int index = 0;                  //Usato per la creazione dei blocchi

  //+ Loop +//
  int WindowLeft;                 //L'indice della prima colonna intera della matrice visualizzata nello schermo
  int GridShift = 0;              //Numero di px di cui la griglia si è spostata a sinistra


  //+ ++++++ Inizializzazione ++++++ +//
  SetMode(MODE_1 | OBJ_ENABLE | OBJ_MAP_1D);    //Imposta le modalità di utilizzo
  InitializeSprites();                          //Inizializza gli sprite

  for(i = 0; i < 256; i++)                      //Carica in memoria la pallette degli sprites
    OBJPaletteMem[i] = palette[i];


  //+ ++++++ Creazione Sprites ++++++ +//
  //+ Personaggio +//
  Character.x = 100;                           //Imposta le caratteristiche dell'oggetto
  Character.y = 60;
  Character.w = character_WIDTH;
  Character.h = character_HEIGHT;
  Character.index = 127;

  tmp = Character.index;                      //Imposta attributi dello sprite nella memoria
  sprites[tmp].attribute0 = COLOR_256 | SQUARE | Character.y;
  sprites[tmp].attribute1 = SIZE_16 | Character.x;
  sprites[tmp].attribute2 = 0;

  tmp1 = Character.w * Character.h / 2;       //Dimensione del BMP in memoria
  for(i = 0; i < tmp1; i++)                   //Carica l'immagine in memoria
    OAMData[i] = characterData[i];

  //+ Blocchi +//
  for(i = 0; i < 10; i++){                    //Scorro le righe della matrice del livello
    for(j = 0; j < levWidth; j++){            //e le colonne
      if(level[i][j] > -1){                   //Se nella cella corrente c'e qualcosa
        tmp = BlockTypes[level[i][j]];
        AddBlock(Blocks, index, tmp, i);      //aggiungo unn blocco cone le giuste caratteristiche
        level[i][j] = index;                  //Imposto nella matrice l'indice nel vettore
        index++;
      }
    }
  }

  tmp2 = block1_WIDTH * block1_HEIGHT / 2;     //Dimensione in memoria dell'immagine
  for(i = 0; i < tmp2; i++)                   //Carico l'immagine in memoria
    OAMData[tmp1 + i] = block1Data[i];


  //+ ++++++ Loop principale ++++++ +//
  while(true){
    //+ Personaggio +//
    Character.y += (int)VSpeed;                 //Aggiorno la coordinata verticale

    CharR = (Character.y + Character.h/2) / 16; //Riga e colonne nella matrice
    CharC = (Character.x + GridShift + Character.w/2) / 16;

    tmp = (Character.x>CharC*16?1:-1);          //Controlla le collisioni vericali
    CharBotColl = ((level[CharR+1][CharC] > -1 || level[CharR+1][CharC+tmp] > -1) && Character.y >= 16 * CharR);
    CharTopColl = ((level[CharR-1][CharC] > -1 || level[CharR-1][CharC+tmp] > -1) && Character.y <  16 * CharR);

    if(CharTopColl){
      VSpeed = 0;
      Character.y = CharR * 16;
    }

    if(CharBotColl){
      VSpeed = 0;
      Character.y = CharR * 16;
    }else{
      VSpeed += 0.45;
    }

    if(CheckPressed(KEY_A) && CharBotColl)
      VSpeed = -7;

    //Calcolo della collisione orizzontale
    tmp = (Character.y > CharR * 16 ? 1 : (Character.y < CharR * 16 ? -1 : 0));
    CharRightColl = ((level[CharR][CharC+1] > -1 || level[CharR+tmp][CharC+1] > -1) && Character.x + GridShift >= CharC * 16);
    MoveSprite(Character);                     //Scrivo le modifiche nella memoria


    //+ Blocchi +//
    //Arrotondato per eccesso
    WindowLeft = GridShift / 16 + (GridShift % 16 > 0 ? 1 : 0);

    //Sposta i blocchi sotto lo schermo se sono usciti dalla visuale
    if(WindowLeft > 0){
      for(i = 0; i < 10; i++){
        tmp = level[i][WindowLeft-1];         //Quello che c'e nella cella corrente
        if(tmp > -1){
          Blocks[tmp].y = 160;
          MoveSprite(Blocks[tmp]);
        }
      }
    }

    //Imposta la posizione dei blocchi sullo schermo
    for(j = WindowLeft; j < WindowLeft + 16 && j < levWidth; j++){
      for(i = 0; i < 10; i++){
        tmp = level[i][j];
        if(tmp > -1){
          Blocks[tmp].x = 16 * j - GridShift;
          MoveSprite(Blocks[tmp]);
        }
      }
    }

    GridShift++;

    //+ Disegno +//
    WaitForVsync();
    CopyOAM();

    if(GridShift > 16 * levWidth || CharR >= 10 || CharRightColl)
      break;
  }

  while(true);
  return 0;
}