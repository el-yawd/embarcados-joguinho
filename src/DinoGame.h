#ifndef DINOGAME_H
#define DINOGAME_H

#include <Arduino.h>
#include <LiquidCrystal.h>

class DinoGame {
public:
    // Internal status to manage playing/game over state
    enum GameStatus {
        PLAYING,
        GAME_OVER
    };

private:
    LiquidCrystal& lcd;

    // Pins
    const int buttonPin; // Dedicated jump button (now Pin 6, the menu select button)

    // State & Timing
    GameStatus currentStatus = PLAYING;
    bool jumping = false;
    unsigned long jumpStart = 0;
    const unsigned long jumpDuration = 1000; // ms

    // Obstacle state
    int obstacleX = 15;
    unsigned long lastMove = 0;
    const unsigned long moveInterval = 200; // ms (controls game speed)

    // Custom characters
    byte playerChar[8] = {
      B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111
    };
    byte obstacleChar[8] = {
      B00100, B01110, B11111, B11111, B11111, B01110, B00100, B00000
    };

    // Private helper methods
    void resetGame();
    void handleJump();
    void moveObstacle();
    bool checkCollision();
    void draw();
    void drawGameOver();

public:
    // Constructor now takes the button pin
    DinoGame(LiquidCrystal& lcdRef, int btnPin);

    void setup();
    void run();
};

#endif // DINOGAME_H
