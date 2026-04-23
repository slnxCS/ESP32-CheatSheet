#include "flappyBird.hpp"
#include "Arduino.h"
#include "input.hpp"
#include "display.hpp"

#define SCREEN_H 64
#define SCREEN_W 128

const uint8_t FlappyBitmap [] PROGMEM = {
	0x70, 0x88, 0x08, 0x0c, 0x0a, 0x92, 0x61, 0x01, 0x1d, 0xa3, 0xc1, 0xc1, 0xda, 0xc4, 0xf8, 0xc0, 
	0x00, 0x00, 0x03, 0x05, 0x05, 0x08, 0x08, 0x08, 0x09, 0x0b, 0x07, 0x07, 0x07, 0x07, 0x07, 0x03
};

#define FLAPPY_W 16
#define FLAPPY_H 12
#define GRAVITY 1

#define PIPE_WIDTH 6
#define GAP_HEIGHT 25
#define PIPE_SPEED 2
#define FLAPPY_FPS 60

void flappyBirdGame(void)
{
    int flappyY = 30;
    const int flappyX = 20;
    int velocity = 0;

    int pipeX = SCREEN_W;
    int gapY = 20;

    int score = 0;

    while (1)
    {
        yield();

        Input input = GetFastInput();

        // --- выход ---
        if (input.Left == ButtonState::Down) return;

        // --- прыжок ---
        if (input.Right == ButtonState::Down || input.Right == ButtonState::Hold)
        {
            velocity = -5;
        }

        // --- физика ---
        velocity += GRAVITY;
        flappyY += velocity;

        // ограничение
        if (flappyY < 0) flappyY = 0;
        if (flappyY > SCREEN_H - FLAPPY_H) flappyY = SCREEN_H - FLAPPY_H;

        // --- трубы ---
        pipeX -= PIPE_SPEED;

        if (pipeX < -PIPE_WIDTH)
        {
            pipeX = SCREEN_W;
            int margin = 5;
            gapY = random(margin, SCREEN_H - GAP_HEIGHT - margin);
            score++;
        }

        // --- столкновения ---
        bool hitX = (flappyX + FLAPPY_W > pipeX) && (flappyX < pipeX + PIPE_WIDTH);
        bool hitY = (flappyY < gapY) || (flappyY + FLAPPY_H > gapY + GAP_HEIGHT);

        if (hitX && hitY)
        {
            // GAME OVER
            display.clear();
            display.setCursor(20, 3);
            display.print("GAME OVER");
            display.setCursor(20, 5);
            display.print("Score:");
            display.print(score);
            display.update();

            delay(1500);
            return;
        }

        // --- отрисовка ---
        display.clear();

        // птица
        display.drawBitmap(flappyX, flappyY, FlappyBitmap, FLAPPY_W, FLAPPY_H);

                // верхняя труба
        if (gapY > 0)
            display.rect(pipeX, 0, PIPE_WIDTH, gapY);

        // нижняя труба
        int bottomY = gapY + GAP_HEIGHT;
        if (bottomY < SCREEN_H)
            display.rect(pipeX, bottomY, PIPE_WIDTH, SCREEN_H);

        // счет
        display.setCursor(0, 0);
        display.print(score);

        display.update();

        delay(1000 / FLAPPY_FPS); // ~30 FPS
    }
}