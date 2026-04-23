#include "Arduino.h"
#include "dino.hpp"
#include "LittleFS.h"
#include "input.hpp"
#include "main.hpp"

#define DINO_GAME_FPS 30 // FPS

/* ---------------------------------------- 1. Битмапы ----------------------------------------*/
const uint8_t DinoStandL_bmp[] PROGMEM = {   
  0xC0, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xC0, 0xFE, 0xFF, 0xFD, 0xBF, 0xAF, 0x2F, 0x2F, 0x0E,
  0x03, 0x07, 0x1E, 0x1E, 0xFF, 0xBF, 0x1F, 0x3F, 0x7F, 0x4F, 0x07, 0x00, 0x01, 0x00, 0x00, 0x00,
};

const uint8_t DinoStandR_bmp[] PROGMEM = {   
  0xC0, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xC0, 0xFE, 0xFF, 0xFD, 0xBF, 0xAF, 0x2F, 0x2F, 0x0E,
  0x03, 0x07, 0x1E, 0x1E, 0x7F, 0x5F, 0x1F, 0x3F, 0xFF, 0x8F, 0x07, 0x00, 0x01, 0x00, 0x00, 0x00,
};

const uint8_t DinoStandDie_bmp[] PROGMEM = {  
  0xC0, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xC0, 0xFE, 0xF1, 0xF5, 0xB1, 0xBF, 0x2F, 0x2F, 0x0E,
  0x03, 0x07, 0x1E, 0x1E, 0xFF, 0xBF, 0x1F, 0x3F, 0xFF, 0x8F, 0x07, 0x00, 0x01, 0x00, 0x00, 0x00,
};

const uint8_t DinoCroachL_bmp[] PROGMEM = {   
  0x03, 0x06, 0x6C, 0x5C, 0x1C, 0xFE, 0xBE, 0x1E, 0x7E, 0x5E, 0x0E, 0x1C, 0x3E, 0x2A, 0x2E, 0x0E,
};

const uint8_t DinoCroachR_bmp[] PROGMEM = {   
  0x03, 0x06, 0xEC, 0x9C, 0x1C, 0x7E, 0x5E, 0x1E, 0x7E, 0x5E, 0x0E, 0x1C, 0x3E, 0x2A, 0x2E, 0x0E,
};

const uint8_t CactusSmall_bmp[] PROGMEM = {  
  0x00, 0x00, 0x00, 0xE0, 0xC0, 0x00, 0xF8, 0xFC, 0xFC, 0xF8, 0x80, 0xFC, 0xFE, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x03, 0x07, 0x06, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
};

const uint8_t CactusBig_bmp[] PROGMEM = {  
  0xF0, 0x00, 0xFC, 0xFE, 0xFE, 0xC0, 0x7C, 0x00, 0xF0, 0x00, 0xF8, 0xFC, 0x60, 0x3E, 0x00, 0x80, 0x00, 0xF8, 0x80, 0xF8, 0xFC, 0xF8, 0x30, 0x1F,
  0x03, 0x07, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01, 0x03, 0xFF, 0xFF, 0x00, 0x1F, 0x30, 0xFF, 0x60, 0x3C, 0x01, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
};

const uint8_t BirdL_bmp[] PROGMEM = {    
  0x30, 0x38, 0x3C, 0x3C, 0x3F, 0x3F, 0x7F, 0x7C, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xE0, 0xE0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0x7F, 0x1F, 0x0F, 0x0F, 0x0F, 0x07, 0x07, 0x07, 0x07, 0x07, 0x03, 0x03, 0x03, 0x00,
};

const uint8_t BirdR_bmp[] PROGMEM = {   
  0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF0, 0xF0, 0xC0, 0x0F, 0xFE, 0xF8, 0xF8, 0xF0, 0xE0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x07, 0x0E, 0x1F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFC, 0xF8, 0xF8, 0x78, 0x68, 0x68, 0x68,
};

GyverOLED<SSD1306_128x64>* oled;

void dino_init(GyverOLED<SSD1306_128x64>* _oled)
{
    oled = _oled;
    if (!LittleFS.exists(F("/dino_game_score.sys")))
    {
        File f = LittleFS.open(F("/dino_game_score.sys"), "w", true);
        f.print("0!");
        f.close();
    }
}

uint32_t uiTimer = 0;

void playDinosaurGame(void);

int getBestScore(void)
{
    if (!LittleFS.exists(F("/dino_game_score.sys")))
    {
        File f = LittleFS.open(F("/dino_game_score.sys"), "w", true);
        f.print("0!");
        f.close();
        return 0;
    }
    File score = LittleFS.open(F("/dino_game_score.sys"));
    uint8_t buffer[9];
    score.readBytesUntil('!', buffer, 8);
    score.close();
    return atoi((char*)buffer);
}

void setBestScore(int score)
{
    File file = LittleFS.open(F("/dino_game_score.sys"), "w");
    file.print(score);
    file.print('!');
    file.close();
}

void dinosaurGame(void) {                                                           
  while (true) {                                                                    
    int bestScore = getBestScore();
    uint32_t oldF = getCpuFrequencyMhz();
    uint32_t oldC = Wire.getClock();
    int oldH = RightButton.holdDelay;
    
    oled->clear();                                                                   
    oled->roundRect(0, 9, 127, 46, OLED_STROKE);                                     
    oled->setCursor(3, 0); oled->print(F("GOOGLE DINOSAUR GAME"));                    
    oled->setCursor(18, 6); oled->print(F("Лучший счет:")); oled->print(bestScore);    
    oled->setCursor(0, 7); oled->print(F("<- Вверх"));                                
    oled->setCursor(96, 7); oled->print(F("Ок ->"));                                  
    oled->drawBitmap(10, 30, DinoStandL_bmp, 16, 16);                                
    oled->drawBitmap(46, 30, CactusBig_bmp, 24, 16);                                 
    oled->drawBitmap(94, 20, BirdL_bmp, 24, 16);                                     
    oled->update();                                                                  
    Wire.setClock(1E6);
    
    setCpuFrequencyMhz(160);
    RightButton.holdDelay = 30;
    while (true) {                                                                  
      Input input = GetInput();                                                                

      if(input.Left == ButtonState::Down){
        Wire.setClock(oldC);
        setCpuFrequencyMhz(oldF);
        RightButton.holdDelay = oldH;
        UserLocation = Location::FileSelecter;
        return;
      }

      if (input.Center == ButtonState::Down) {                                                         
        playDinosaurGame();                                                     
        break;                                                                  
      }

      yield();
    }
  }
}

#define EFV           -5        
#define EFH           -5        
#define DINO_GROUND_Y 47        
#define DINO_GRAVITY  0.170f       

void playDinosaurGame(void) {

startDinoGame:                         
  uint8_t gameSpeed = 10;              
  uint16_t score = 0;                  
  uint16_t bestScore = 0;              
  int8_t oldEnemyPos = 128;            
  int8_t oldEnemyType = 0;             
  int8_t newEnemyPos = 128;            
  int8_t newEnemyType = random(0, 3);  
  bool dinoStand = true;               
  bool legFlag = true;                 
  bool birdFlag = true;                
  int8_t dinoY = DINO_GROUND_Y;        
  float dinoU = 0.0;                   
  setCpuFrequencyMhz(160);
  int dinoBestScore = getBestScore();

  

  while (1) {                                                   
    yield();
                                                                 

    Input input = GetInput();
    if (input.Left == ButtonState::Down) return;                                                  

    /* ------------------ User input ------------------ */
    if (input.Center == ButtonState::Down and dinoY == DINO_GROUND_Y) {                             
      dinoU = -2.8;                                                          
      dinoY -= 4;                                                            
    } else if (input.Center == ButtonState::Hold and dinoY == DINO_GROUND_Y) {     
      dinoU = -3.4;                                                          
      dinoY -= 4;                                                            
    } else if (input.Right == ButtonState::Down) {                                               
      dinoU = 3.2;                                                           
      if (dinoY >= DINO_GROUND_Y) {                                          
        dinoY = DINO_GROUND_Y;                                               
        dinoU = 0.0;                                                         
      }
    }

    if ((input.Right == ButtonState::Down || input.Right == ButtonState::Hold) and dinoY >= DINO_GROUND_Y) {                         
      dinoStand = false;                                                     
    } else {
      dinoStand = true;                                                      
    }

    /* ------------------ Game processing ------------------ */
    static uint32_t scoreTimer = millis();                                   
    if (millis() - scoreTimer >= 100) {
      scoreTimer = millis();
      score++;                                                               
      if (score < 1000) 
        gameSpeed = constrain(map(score, 900, 0, 4, 10), 4, 10);             
      else 
        gameSpeed = constrain(map(score%1000, 900, 0, 4, 7), 4, 7);          
    }

    static uint32_t enemyTimer = millis();                                   
    if (millis() - enemyTimer >= gameSpeed) {                                
      enemyTimer = millis();
      if (--newEnemyPos < 16) {                                              
        oldEnemyPos = newEnemyPos;                                           
        oldEnemyType = newEnemyType;                                         
        newEnemyPos = 128;                                                   
        do newEnemyType = random(0, 3);                                      
        while(newEnemyType == oldEnemyType);                                 
      }
      if (oldEnemyPos >= -24) {                                              
        oldEnemyPos--;                                                       
      }
    }

    static uint32_t legTimer = millis();                                     
    if (millis() - legTimer >= 130) {
      legTimer = millis();
      legFlag = !legFlag;                                                    
    }

    static uint32_t birdTimer = millis();                                    
    if (millis() - birdTimer >= 200) {
      birdTimer = millis();
      birdFlag = !birdFlag;                                                  
    }

    static uint32_t dinoTimer = millis();                                    
    if (millis() - dinoTimer >= 15) {                                        
      dinoTimer = millis();
      dinoU += (float)DINO_GRAVITY;                                          
      dinoY += (float)dinoU;                                                 
      if (dinoY >= DINO_GROUND_Y) {                                          
        dinoY = DINO_GROUND_Y;                                               
        dinoU = 0.0;                                                         
      }
    }

    /* ------------------ Drawing ------------------ */
    static uint32_t oledTimer = millis();                                    
    if (millis() - oledTimer >= (1000 / DINO_GAME_FPS)) {                    
      oledTimer = millis();

      oled->clear();                                                                                     
      oled->setCursor(0, 0); oled->print("Best: ");
      oled->setCursor(30, 0); oled->print(dinoBestScore); oled->print(":"); oled->print(score);        
      oled->line(0, 63, 127, 63);
      processBat();                                                      

      switch (oldEnemyType) {                                                                           
        case 0: oled->drawBitmap(oldEnemyPos, 48, CactusSmall_bmp, 16, 16);                   break;     
        case 1: oled->drawBitmap(oldEnemyPos, 48, CactusBig_bmp, 24, 16);                     break;     
        case 2: oled->drawBitmap(oldEnemyPos, 35, birdFlag ? BirdL_bmp : BirdR_bmp, 24, 16);  break;     
      }

      switch (newEnemyType) {  
        case 0: oled->drawBitmap(newEnemyPos, 48, CactusSmall_bmp, 16, 16);                     break;
        case 1: oled->drawBitmap(newEnemyPos, 48, CactusBig_bmp, 24, 16);                       break;
        case 2: oled->drawBitmap(newEnemyPos, 35, birdFlag ? BirdL_bmp : BirdR_bmp, 24, 16);    break;
      }

      if (oldEnemyPos <= (16 + EFH) and oldEnemyPos >= (oldEnemyType > 0 ? -24 - EFH : -16 - EFH)) {
        if (oldEnemyType != 2 ? dinoY > 32 - EFH : dinoStand and dinoY > 19 - EFH) {
          uiTimer = millis();
          oled->drawBitmap(0, dinoY, DinoStandDie_bmp, 16, 16);     
          oled->roundRect(0, 10, 127, 40, OLED_CLEAR); oled->roundRect(0, 10, 127, 40, OLED_STROKE);
          oled->setScale(2); oled->setCursor(7, 2); oled->print(F("GAME OVER"));
          oled->setScale(1); oled->setCursor(3, 4); oled->print(F("<- Вверх"));
          oled->setCursor(96, 4); oled->print(F("Ок ->"));
          oled->update();
          if (score > dinoBestScore) {
            dinoBestScore = score;
            setBestScore(dinoBestScore);
          }                                       
          while (1) {                                                                                   
            Input input = GetInput();
            if (input.Center == ButtonState::Down) goto startDinoGame;
            if (input.Left == ButtonState::Down) return;
            yield();
          }
        }
      }

      if (dinoStand) {                                                                                  
        oled->drawBitmap(0, dinoY, legFlag ? DinoStandL_bmp : DinoStandR_bmp, 16, 16);                   
      } else {                                                                                          
        oled->drawBitmap(0, 56, legFlag ? DinoCroachL_bmp : DinoCroachR_bmp, 16, 8);                     
      }

      oled->update();                                                                                    
    }
    yield();
  }
}