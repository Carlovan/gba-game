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
  sprites[index].attribute2 = type;
}

int main(){
  //+ ++++++ Variabili ++++++ +//
  //+ Uso generale +//
  int i, j, k;                    //Indici nei cicli
  int tmp, tmp1, tmp2;            //Usate per valori temporanei nei calcoli
  int counter=0;                  //contatore(utilizzato per bg ma volendo anche ad uso generale)
  
  //+ Background +//
  Bg background, background0;
  
  background.number = 2;                  //Utilizza il bg n2
  background.charBaseBlock = 0;           //Specifica il CBB in cui si trova la mappa
  background.screenBaseBlock = 17;        //Specifica il SBB in cui si trovano i tile
  background.colorMode = BG_COLOR_256;    //Colore del bg(b-bit)
  background.size = ROTBG_SIZE_256x256;   //Dimensione del bg
  background.x_scroll = 120;              //Posizione orizzontale di partenza
  background.y_scroll = 80;               //Posizione verticale di partenza
  background.mosaic = 0;                  //Mosaic mode(va bhe)
  background.wraparound = 0;              //Wraparound(solo Text bg?)
  
  background0 = background;               //Secondo background(simulazione di continuità)
  background0.number = 3;                     //è identico al primo se non per dove si trova la sua mappa e la posizione
  background0.x_scroll = 120-256;
  background0.y_scroll = 80;
  
  //+ Personaggio +//
  Sprite Character;               //L'oggetto per gestire il personaggio
  float VSpeed = 0;               //Velocita verticale (px/sec)
  int CharC, CharR;               //Colonna e riga del personaggio nella matrice del livello
  bool CharTopColl = 0;           //Collisione sopra
  bool CharRightColl = 0;         //a destra
  bool CharBotColl = 0;           //e in basso
  const float GRAVITY = 0.5;      //Valore da aggiungere alla velocita verticale per ogni frame
  const float JUMP = -7;          //Velocita verticale quando salta

  //+ Blocchi +//
  Sprite Blocks[127];             //Vettore per contenere le informazioni dei blocchi
  int BlockTypes[] = {8};         //Posizione in memoria del BMP per ogni tipo di blocco
  int index = 0;                  //Usato per la creazione dei blocchi
  bool BlocksUsed[127];           //Segna quali posizioni del vettore Blocks possono essere usate (0 se possono essere)
  memset(BlocksUsed, 0, sizeof(BlocksUsed));

  //+ Loop +//
  int WindowLeft, OldWindowLeft=0;  //L'indice della prima colonna intera della matrice visualizzata nello schermo
  int GridShift = 0;                //Numero di px di cui la griglia si è spostata a sinistra
  const int SHIFTPF = 2;            //Pixel di cui si sposta la griglia ad ogni frame
  const int MAXCOL = 16;            //Numero massimo di colonne visibili contemporaneamente


  //+ ++++++ Inizializzazione ++++++ +//
  SetMode(MODE_2 | OBJ_ENABLE | OBJ_MAP_1D);    //Imposta le modalità di utilizzo
  InitializeSprites();                          //Inizializza gli sprite

  for(i = 0; i < 256; i++)                      //Carica in memoria la pallette degli sprites
    OBJPaletteMem[i] = palette[i];

  //+ ++++++ Disposizione Background ++++++ +//
  EnableBackground(&background);
  EnableBackground(&background0);
  
  for(i = 0; i < 256; i++)                               //Palette del bg
    BGPaletteMem[i] = tiles0Palette[i];
    
  for(i = 0; i < tiles0_WIDTH*tiles0_HEIGHT /2; i++)     //Dati dei tile
  {
    background.tileData[i] = tiles0Data[i];
    background0.tileData[i] = tiles0Data[i];
  }
  
  u16* temp = (u16*) map;
  for(i = 0; i < map_WIDTH*map_HEIGHT /2; i++)           //Mappa della disposizione dei tile
    background.mapData[i] = temp[i];  
    
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
  for(i = 0; i < tmp2; i++)                   //Carico l'immagine in memoria
    OAMData[tmp1 + i] = block1Data[i];


  //+ ++++++ Loop principale ++++++ +//
  while(true){
    //+ Personaggio +//
    Character.y += (int)VSpeed;                 //Aggiorno la coordinata verticale

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
      background0.x_scroll++;
    }
    
    if(background.x_scroll == 120+256)     //Simula la continuità
      background.x_scroll = 120-256;
    
    if(background0.x_scroll == 120+256)
      background0.x_scroll = 120-256;
      
    RotateBackground(&background,angle,120,80,zoom);        //Aggiorna correttamente i valori della struct
    RotateBackground(&background0,angle,120,80,zoom);       //N.B. non serve solo a ruotare, è fondamentale anche per il disegno
    
    //+ Disegno +//
    WaitForVsync();
    UpdateBackground(&background);
    UpdateBackground(&background0);
    CopyOAM();
    
    if(GridShift > 16 * levWidth || CharR >= 10 || CharRightColl)
      break;
  }

  while(true);
  return 0;
}
