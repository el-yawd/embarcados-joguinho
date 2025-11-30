#ifndef BLOCK_BREAKER_H
#define BLOCK_BREAKER_H

#include <Arduino.h>
#include <LiquidCrystal.h>
#include "Arduino_LED_Matrix.h" // Biblioteca nativa do UNO R4 WiFi

// Estados internos do Block Breaker
enum BBState {
    BB_WAITING,
    BB_PLAYING,
    BB_GAME_OVER,
    BB_VICTORY
};

class BlockBreaker {
public:
    // Construtor: LCD para placar, pino do potenciômetro, pino do botão de ação
    BlockBreaker(LiquidCrystal& lcdRef, int pPin, int bPin);
    
    void begin(); // NOVO: Inicialização única do Hardware (chamar no setup do Arduino)
    void start(); // RENOMEADO: Inicia uma sessão de jogo (chamar no menu)
    void run();   // Loop principal do jogo
    void stop();  // Método para limpar a matriz ao sair

private:
    LiquidCrystal& lcd;
    ArduinoLEDMatrix matrix; // Objeto de controle da matriz do R4
    
    int potPin;
    int buttonPin;
    
    // Variáveis de Estado
    BBState state;
    
    // Buffer da tela (8 linhas x 12 colunas)
    // 1 = LED ligado, 0 = LED desligado
    uint8_t frame[8][12]; 
    
    // Configurações da Raquete
    int paddleX;
    const int paddleWidth = 3; 
    
    // Configurações da Bola
    float ballX, ballY;
    float ballDirX, ballDirY;
    unsigned long lastBallUpdate;
    const int ballSpeedDelay = 250; // Velocidade (menor = mais rápido)
    
    // Tijolos (3 linhas x 12 colunas)
    bool bricks[3][12]; 
    int totalBricks;
    
    // Métodos Auxiliares
    void resetGame();
    void initBricks();
    void updatePaddle();
    void updateBall();
    void draw(); // Renderiza o frame na matriz
};

#endif