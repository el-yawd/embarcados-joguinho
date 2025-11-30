#ifndef REACTIONGAME_H
#define REACTIONGAME_H

#include <Arduino.h>
#include <LiquidCrystal.h>
#include "Arduino_LED_Matrix.h" // Inclui controle da matriz

class ReactionGame {
public:
    enum GameState {
        WAITING,
        COUNTDOWN,
        GO,
        FINISHED
    };

private:
    LiquidCrystal& lcd;
    ArduinoLEDMatrix matrix; // Objeto da matriz

    // Pins
    const int player1Pin;
    const int player2Pin;
    int selectButtonPin; // Novo: Para reiniciar o jogo

    // Game State
    GameState currentState = WAITING;
    unsigned long lastTime = 0;
    unsigned long startTime = 0;
    long goDelayMs = 0; // Random delay before GO

    // Flags de segurança para botões
    bool canRestart;       // Impede reinício imediato se botão estiver pressionado
    bool buttonsReleased;  // Impede "falta" imediata no reinício

    // Result tracking
    int winner = 0; // 1 for P1, 2 for P2, 0 for Tie/Waiting
    unsigned long reactionTime = 0;

    // Custom characters (optional: simple indicators)
    byte p1Char[8] = { B10000, B11000, B10100, B10000, B10000, B10000, B10000, B10000 };
    byte p2Char[8] = { B00001, B00011, B00101, B00001, B00001, B00001, B00001, B00001 };

    // Private helper methods
    void resetGame();
    void stateCountdown();
    void stateGo();
    void stateFinished();
    void drawInstructions();
    
    // Helper para desenhar na matriz
    void clearMatrix();

public:
    // Construtor atualizado para receber o botão de Select
    ReactionGame(LiquidCrystal& lcdRef, int p1Pin, int p2Pin, int selPin);

    void begin(); // Inicialização do Hardware (Matriz)
    void setup(); // Prepara a partida
    void run();
    void stop();  // Limpa a matriz ao sair
};

#endif // REACTIONGAME_H