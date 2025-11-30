#include "BlockBreaker.h"

BlockBreaker::BlockBreaker(LiquidCrystal& lcdRef, int pPin, int bPin) 
    : lcd(lcdRef), potPin(pPin), buttonPin(bPin) {
}

// NOVO: Chamado apenas UMA VEZ no setup() principal
void BlockBreaker::begin() {
    matrix.begin(); // Inicializa hardware da matriz apenas uma vez
    pinMode(buttonPin, INPUT);
}

// RENOMEADO: Chamado toda vez que entra no jogo pelo menu
void BlockBreaker::start() {
    // Mensagem de Introdução no LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" LED MATRIX GAME");
    lcd.setCursor(0, 1);
    lcd.print(" Block Breaker ");
    delay(1500);
    
    resetGame();
}

void BlockBreaker::stop() {
    // Zera todo o buffer (desliga todos os pixels)
    for(int y=0; y<8; y++) {
        for(int x=0; x<12; x++) {
            frame[y][x] = 0;
        }
    }
    // Envia o buffer vazio para a matriz física imediatamente
    matrix.renderBitmap(frame, 8, 12);
}

void BlockBreaker::initBricks() {
    totalBricks = 0;
    // Cria 3 linhas de tijolos preenchidos
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 12; x++) {
            bricks[y][x] = true;
            totalBricks++;
        }
    }
}

void BlockBreaker::resetGame() {
    state = BB_WAITING;
    initBricks();
    
    // Posição inicial da bola (centro inferior)
    ballX = 6;
    ballY = 6; 
    ballDirX = 0.5; // Ligeira inclinação lateral
    ballDirY = -1;  // Movimento para cima
    
    lastBallUpdate = millis();
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Score: 0");
    lcd.setCursor(0, 1);
    lcd.print("Press Jump/Start");
}

void BlockBreaker::updatePaddle() {
    int potValue = analogRead(potPin);
    // Mapeia o potenciômetro para a largura da matriz (12) menos a largura da raquete
    // Garante que a raquete não saia da tela
    paddleX = map(potValue, 0, 1023, 12 - paddleWidth, 0);
    
    // Proteção extra de limites
    if (paddleX < 0) paddleX = 0;
    if (paddleX > (12 - paddleWidth)) paddleX = 12 - paddleWidth;
}

void BlockBreaker::updateBall() {
    if (millis() - lastBallUpdate > ballSpeedDelay) {
        lastBallUpdate = millis();
        
        // Calcula próxima posição
        float nextX = ballX + ballDirX;
        float nextY = ballY + ballDirY;
        
        // 1. Colisão com Paredes Laterais (0 e 11)
        if (nextX < 0 || nextX >= 12) {
            ballDirX *= -1; // Inverte direção X
            nextX = ballX + ballDirX; // Recalcula nextX
        }
        
        // 2. Colisão com Teto (0)
        if (nextY < 0) {
            ballDirY *= -1; // Inverte direção Y
            nextY = ballY + ballDirY;
        }
        
        // 3. Colisão com Chão (Game Over)
        if (nextY >= 8) {
            state = BB_GAME_OVER;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("GAME OVER!");
            lcd.setCursor(0, 1);
            lcd.print("Btn to Restart");
            return;
        }
        
        // 4. Colisão com Raquete (Linha 7)
        // Se a bola está descendo (ballDirY > 0) e vai cruzar a linha 7
        if (ballDirY > 0 && nextY >= 7 && ballY < 8) {
            int ballIntX = (int)nextX;
            // Verifica se a bola está dentro da área horizontal da raquete
            if (ballIntX >= paddleX && ballIntX < paddleX + paddleWidth) {
                ballDirY *= -1; // Rebate para cima
                
                // Física Simples: Bater nas pontas muda o ângulo X
                if (ballIntX == paddleX) ballDirX = -0.7; // Ponta esquerda
                else if (ballIntX == paddleX + paddleWidth - 1) ballDirX = 0.7; // Ponta direita
                
                nextY = ballY + ballDirY; // Aplica o movimento
            }
        }
        
        // 5. Colisão com Tijolos (Linhas 0, 1, 2)
        int brickX = (int)nextX;
        int brickY = (int)nextY;
        
        if (brickY >= 0 && brickY < 3 && brickX >= 0 && brickX < 12) {
            if (bricks[brickY][brickX]) {
                bricks[brickY][brickX] = false; // Destrói tijolo
                totalBricks--;
                ballDirY *= -1; // Rebate bola
                
                // Atualiza Placar
                lcd.setCursor(7, 0);
                lcd.print(36 - totalBricks);
                
                if (totalBricks <= 0) {
                    state = BB_VICTORY;
                    lcd.clear();
                    lcd.setCursor(0,0);
                    lcd.print("YOU WIN!");
                }
            }
        }
        
        // Aplica nova posição
        ballX += ballDirX;
        ballY += ballDirY;
    }
}

void BlockBreaker::draw() {
    // Limpa o buffer
    for(int y=0; y<8; y++) {
        for(int x=0; x<12; x++) {
            frame[y][x] = 0;
        }
    }
    
    if (state == BB_VICTORY) {
        // Desenha "Smiley Face"
        frame[2][3]=1; frame[2][8]=1; // Olhos
        frame[5][3]=1; frame[6][4]=1; frame[6][5]=1; frame[6][6]=1; frame[6][7]=1; frame[5][8]=1; // Sorriso
    } 
    else if (state == BB_GAME_OVER) {
        // Desenha um "X" grande
        frame[1][2]=1; frame[2][3]=1; frame[3][4]=1; frame[4][5]=1; frame[5][6]=1;
        frame[5][2]=1; frame[4][3]=1; frame[3][4]=1; frame[2][5]=1; frame[1][6]=1;
    }
    else {
        // 1. Desenha Tijolos ativos
        for(int y=0; y<3; y++) {
            for(int x=0; x<12; x++) {
                if (bricks[y][x]) frame[y][x] = 1;
            }
        }
        
        // 2. Desenha Raquete (sempre na linha 7)
        for(int i=0; i<paddleWidth; i++) {
             int px = paddleX + i;
             if(px < 12) frame[7][px] = 1;
        }
        
        // 3. Desenha Bola
        int bx = (int)ballX;
        int by = (int)ballY;
        if(bx >= 0 && bx < 12 && by >= 0 && by < 8) {
            frame[by][bx] = 1;
        }
    }
    
    // Atualiza a matriz de LEDs física
    matrix.renderBitmap(frame, 8, 12);
}

void BlockBreaker::run() {
    updatePaddle(); // Leitura contínua do pot
    
    if (state == BB_WAITING) {
        draw();
        // Inicia o jogo ao pressionar o botão
        if (digitalRead(buttonPin) == HIGH) {
            state = BB_PLAYING;
            lcd.setCursor(0, 1);
            lcd.print("Running...      ");
            delay(200); // Debounce simples
        }
    }
    else if (state == BB_PLAYING) {
        updateBall();
        draw();
    }
    else {
        // Estado GAME OVER ou VICTORY
        draw();
        // Reinicia ao pressionar botão
        if (digitalRead(buttonPin) == HIGH) {
            resetGame();
            delay(500); 
        }
    }
}