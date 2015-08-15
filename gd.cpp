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
#include <string.h>

void AddBlock(Sprite Blocks[], u16 index, u16 type, u16 row){
  Blocks[index].x = 240;
  Blocks[index].y = 16*row;
  Blocks[index].w = block1_WIDTH;
  Blocks[index].h = block1_HEIGHT;
  Blocks[index].index = index+1;

  sprites[Blocks[index].index].attribute0 = COLOR_256 | SQUARE | Blocks[index].y; //setup sprite info, 256 colour, shape and y-coord
  sprites[Blocks[index].index].attribute1 = SIZE_16 | Blocks[index].x;            //size 16x16 and x-coord
  sprites[Blocks[index].index].attribute2 = type;                                 //pointer to tile where sprite starts
}

int main()
{
  u16 i, j;                                                               //generic loop variable

  SetMode(MODE_1 | OBJ_ENABLE | OBJ_MAP_1D);                              //set mode 1 (2 text bg and 1 rotational bg) and enable sprites and 1d mapping

  for(i = 0; i < 256; i++)                                                //load the palette into memory
    OBJPaletteMem[i] = palette[i];

  InitializeSprites();                                                    //set all sprites off screen (stops artifact)

  // * ****** Character creation ****** * \\

  float VSpeed = 0;                                                       //Sprite vertical speed
  u16 bottomLimit = 130;
  Sprite Character;
  Character.x = 100;
  Character.y = 60;
  Character.w = character_WIDTH;
  Character.h = character_HEIGHT;
  Character.index = 0;

  sprites[Character.index].attribute0 = COLOR_256 | SQUARE | Character.y; //setup sprite info, 256 colour, shape and y-coord
  sprites[Character.index].attribute1 = SIZE_16 | Character.x;            //size 16x16 and x-coord
  sprites[Character.index].attribute2 = 0;                                //pointer to tile where sprite starts

  u16 CharSize = Character.w * Character.h / 2;

  for(i = 0; i < CharSize; i++)                      //load sprite image data
  {
    OAMData[i] = characterData[i];
  }

  // * ****** Blocks creation ****** * \\

  u16 BlockSize = block1_WIDTH * block1_HEIGHT / 2;
  for(i = 0; i < BlockSize; i++)                      //load sprite image data
  {
    OAMData[CharSize + i] = block1Data[i];
  }

  u8 blockTypes[] = {8};                             //adresses of block types

  u8 index = 0;
  Sprite Blocks[127];
  bool BlocksNFree[127];
  memset(BlocksNFree, 0, sizeof(BlocksNFree));

  //Initialize only the first 15 columns of the grid
  for(i = 0; i < 10; i++){
    for(j = 0; j < 16; j++){
      if(level[i][j] > 0){
        BlocksNFree[index] = 1;

        AddBlock(Blocks, index, blockTypes[level[i][j]-1], i);

        level[i][j] = index+1;

        index++;
      }
    }
  }

  //* ****** Main loop ****** *\\

  u16 gridShift = 0;                                     //Amount of pixel the grid is shifted to the left
  u16 CharR, CharC;                                      //Row and column of the character in the grid
  u16 counter = 0;
  bool topColl=0, rightColl=0, bottomColl=0;             //Collision to top, right and bottom

  while(1)
  {
    Character.y += (s16)VSpeed;                                //Move vertically the character

    CharR = (Character.y + Character.h/2) / 16;                //The character row in the grid
    CharC = (Character.x + gridShift + Character.w/2)  / 16;   //The character column in the grid

    //Check the collision with the three bottom cells
    bottomColl = ((level[CharR+1][CharC] > 0 || level[CharR+1][CharC+(Character.x>CharC*16?1:-1)] > 0 ) && Character.y >= CharR * 16);
    //Check the collision with the three top cells
    topColl = ((level[CharR-1][CharC] > 0 || level[CharR-1][CharC+(Character.x>CharC*16?1:-1)] > 0 ) && Character.y <= CharR * 16);

    //If collide to the top
    if(topColl){
      VSpeed = 0;                                   //Stop the vertical movement
      Character.y = CharR * 16+1;                     //Set the correct position
    }
    //If collide to the bottom
    if(bottomColl){
      VSpeed = 0;                                   //Stop the vertical movement
      Character.y = CharR * 16;                     //Set the correct position
    }else{
      VSpeed += 0.45;                               //Gravity simulation
    }
    if(CheckPressed(KEY_A) && bottomColl)
    {
      VSpeed = -7;                                  //Jump
    }
    rightColl = (level[CharR][CharC+1] > 0 || level[CharR+(Character.y>CharR*16?1:-1)][CharC+1] > 0) && Character.x+gridShift >= CharC * 16;


    u16 windowLeft = gridShift / 16 + 1;             //The left limit of the window

    //Move blocks outside (bottom) if they are outside to the left
    if(counter >= 16){
      counter = 0;

      if(windowLeft > 0){
        for(i = 0; i < 10; i++){
          if(level[i][windowLeft-1] > 0){
            u16 cur = level[i][windowLeft-1]-1;                             //Current block
            Blocks[cur].y = 160;
            BlocksNFree[cur] = 0;                                            //Free the space for the current block in the array
            MoveSprite(Blocks[cur]);
          }
        }
      }

      u16 curCol = windowLeft+14;
      for(i = 0; i < 10; i++){
        if(level[i][curCol] > 0){
          u16 cur = level[i][curCol]-1;
          for(j = 0; j < 127 && BlocksNFree[j] == 1; j++);                //Search the first free space in the array
          AddBlock(Blocks, j, blockTypes[level[i][curCol]-1], i);
          BlocksNFree[j] = 1;
          level[i][curCol] = Blocks[j].index;
        }
      }
    }

    //Move the blocks
    for(j = windowLeft; j < 16 + windowLeft && j < levWidth; j++){
      for(i = 0; i < 10; i++){
        if(level[i][j] > 0){
          u16 cur = level[i][j]-1;
          Blocks[cur].x = 16 * j - gridShift;
          MoveSprite(Blocks[cur]);
        }
      }
    }

    MoveSprite(Character);                                                //Move the character to its current position

    counter++;
    gridShift++;                                                          //Shift the grid

    WaitForVsync();                                                       //waits for the screen to stop drawing
    CopyOAM();                                                            //Copies sprite array into OAM.

    //If the grid is shifted to the end, or if the character is fallen down or if it collides to the right
    if(gridShift > 16*levWidth || CharR >= 10 || rightColl) break;
  }

  while(1);
}
