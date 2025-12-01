#ifndef REACTIONGAME_H
#define REACTIONGAME_H

#include <Arduino.h>
#include <LiquidCrystal.h>
#include "Arduino_LED_Matrix.h" 

// --- Class Definition ---
class ReactionGame {
public:
    // --- Game States ---
    enum GameState {
        WAITING,
        COUNTDOWN,
        GO,
        FINISHED
    };

private:
    // --- Hardware References ---
    LiquidCrystal& lcd;
    ArduinoLEDMatrix matrix;

    // --- Pin Definitions ---
    const int player1Pin;
    const int player2Pin;
    int selectButtonPin; 

    // --- State Variables ---
    GameState currentState = WAITING;
    unsigned long lastTime = 0;
    unsigned long startTime = 0;
    long goDelayMs = 0; 

    // --- Safety Flags ---
    bool canRestart;       // Prevent immediate restart if button held
    bool buttonsReleased;  // Prevent false start on next round

    // --- Scoring & Results ---
    int winner = 0;        // 1=P1, 2=P2, 0=Tie/None
    unsigned long reactionTime = 0;

    // --- Custom Char Arrays ---
    byte p1Char[8] = { B10000, B11000, B10100, B10000, B10000, B10000, B10000, B10000 };
    byte p2Char[8] = { B00001, B00011, B00101, B00001, B00001, B00001, B00001, B00001 };

    // --- Internal Helpers ---
    void resetGame();
    void stateCountdown();
    void stateGo();
    void stateFinished();
    void drawInstructions();
    void clearMatrix();

public:
    // --- Constructor ---
    ReactionGame(LiquidCrystal& lcdRef, int p1Pin, int p2Pin, int selPin);

    // --- Main Methods ---
    void begin();         // Hardware initialization
    void setup();         // Session setup
    void run();           // Main game loop
    void stop();          // Cleanup on exit
};

#endif // REACTIONGAME_H