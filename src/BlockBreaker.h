#ifndef BLOCK_BREAKER_H
#define BLOCK_BREAKER_H

#include <Arduino.h>
#include <LiquidCrystal.h>
#include "Arduino_LED_Matrix.h" 

// --- Game States ---
enum BBState {
    BB_WAITING,
    BB_PLAYING,
    BB_GAME_OVER,
    BB_VICTORY
};

// --- Class Definition ---
class BlockBreaker {
public:
    // --- Constructor ---
    BlockBreaker(LiquidCrystal& lcdRef, int pPin, int bPin);
    
    // --- Main Methods ---
    void begin(); // Hardware init (run once)
    void start(); // Game session start
    void run();   // Main loop
    void stop();  // Cleanup

private:
    // --- Hardware References ---
    LiquidCrystal& lcd;
    ArduinoLEDMatrix matrix; 
    
    // --- Controls ---
    int potPin;
    int buttonPin;
    
    // --- State Variables ---
    BBState state;
    
    // --- Graphics Buffer ---
    uint8_t frame[8][12]; 
    
    // --- Paddle Physics ---
    int paddleX;
    const int paddleWidth = 3; 
    
    // --- Ball Physics ---
    float ballX, ballY;
    float ballDirX, ballDirY;
    unsigned long lastBallUpdate;
    const int ballSpeedDelay = 250; 
    
    // --- Level Data ---
    bool bricks[3][12]; 
    int totalBricks;
    
    // --- Internal Helpers ---
    void resetGame();
    void initBricks();
    void updatePaddle();
    void updateBall();
    void draw(); 
};

#endif