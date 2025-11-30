#include "ReactionGame.h"

// --- DESENHOS DA MATRIZ (0 = Apagado, 1 = Aceso) ---

// Desenho: "P1"
uint8_t frame_p1[8][12] = {
  {0,0,0,0,0,0,0,0,0,0,0,0},
  {0,1,1,1,0,0,0,0,1,1,0,0},
  {0,1,0,1,0,0,0,0,0,1,0,0},
  {0,1,1,1,0,0,0,0,0,1,0,0},
  {0,1,0,0,0,0,0,0,0,1,0,0},
  {0,1,0,0,0,0,0,0,0,1,0,0},
  {0,1,0,0,0,0,0,0,1,1,1,0},
  {0,0,0,0,0,0,0,0,0,0,0,0}
};

// Desenho: "P2"
uint8_t frame_p2[8][12] = {
  {0,0,0,0,0,0,0,0,0,0,0,0},
  {0,1,1,1,0,0,0,1,1,1,0,0},
  {0,1,0,1,0,0,0,0,0,1,0,0},
  {0,1,1,1,0,0,0,1,1,1,0,0},
  {0,1,0,0,0,0,0,1,0,0,0,0},
  {0,1,0,0,0,0,0,1,1,1,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0}
};

// Desenho: "!" (GO)
uint8_t frame_go[8][12] = {
  {0,0,0,0,0,1,1,0,0,0,0,0},
  {0,0,0,0,0,1,1,0,0,0,0,0},
  {0,0,0,0,0,1,1,0,0,0,0,0},
  {0,0,0,0,0,1,1,0,0,0,0,0},
  {0,0,0,0,0,1,1,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,1,1,0,0,0,0,0},
  {0,0,0,0,0,1,1,0,0,0,0,0}
};

// Desenho: "X" (Falta/Foul)
uint8_t frame_foul[8][12] = {
  {0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,1,0,0,0,0,0,0,1,0,0},
  {0,0,0,1,0,0,0,0,1,0,0,0},
  {0,0,0,0,1,0,0,1,0,0,0,0},
  {0,0,0,0,0,1,1,0,0,0,0,0},
  {0,0,0,0,1,0,0,1,0,0,0,0},
  {0,0,0,1,0,0,0,0,1,0,0,0},
  {0,0,1,0,0,0,0,0,0,1,0,0}
};

// Constructor: Initializes the internal references and pins
ReactionGame::ReactionGame(LiquidCrystal& lcdRef, int p1Pin, int p2Pin, int selPin)
    : lcd(lcdRef), player1Pin(p1Pin), player2Pin(p2Pin), selectButtonPin(selPin) {}

void ReactionGame::begin() {
    matrix.begin(); // Inicializa hardware da matriz
}

void ReactionGame::stop() {
    clearMatrix(); // Limpa visualmente ao sair
}

void ReactionGame::clearMatrix() {
    uint8_t frame[8][12];
    for(int y=0; y<8; y++) for(int x=0; x<12; x++) frame[y][x] = 0;
    matrix.renderBitmap(frame, 8, 12);
}

void ReactionGame::setup() {
    // Setup pins (though done in main, harmless to repeat)
    pinMode(player1Pin, INPUT);
    pinMode(player2Pin, INPUT);
    pinMode(selectButtonPin, INPUT);

    // Create custom characters
    lcd.createChar(0, p1Char); // P1 indicator
    lcd.createChar(1, p2Char); // P2 indicator

    // Display introductory message
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("REACTION GAME");
    drawInstructions();
    
    // Limpa matriz ao iniciar
    clearMatrix();
    
    delay(1500);

    resetGame();
}

void ReactionGame::resetGame() {
    // Generate a random delay (e.g., 2000ms to 5000ms)
    randomSeed(analogRead(A1)); // Seed randomizer
    goDelayMs = random(2000, 5000);

    winner = 0;
    reactionTime = 0;
    currentState = COUNTDOWN;
    startTime = millis();
    lastTime = millis();
    
    // Segurança: Impede faltas imediatas se o botão ainda estiver pressionado do jogo anterior
    buttonsReleased = false;

    lcd.clear();
    clearMatrix(); // Garante matriz limpa no reset
}

void ReactionGame::drawInstructions() {
    // Clear instructions to show P1 vs P2 pins
    lcd.setCursor(0, 1);
    lcd.print("P1:6 vs P2:7");
}

void ReactionGame::stateCountdown() {
    // 1. Fase de Segurança: Espera soltar os botões antes de começar
    if (!buttonsReleased) {
        if (digitalRead(player1Pin) == LOW && digitalRead(player2Pin) == LOW) {
            buttonsReleased = true;
            startTime = millis(); // Reinicia a contagem do tempo aleatório apenas AGORA
            lcd.setCursor(0, 0);
            lcd.print("Ready...      ");
        } else {
            // Mostra mensagem para soltar botão se demorar muito
            if (millis() - lastTime > 500) {
                lcd.setCursor(0, 0);
                lcd.print("Release Btn!  ");
                lastTime = millis();
            }
        }
        return; // Não faz nada enquanto não soltar
    }

    // 2. Lógica Normal de Jogo
    unsigned long elapsed = millis() - startTime;

    // Check for premature press (Foul)
    if (digitalRead(player1Pin) == HIGH) {
        winner = 2; // P1 pressed too early, P2 wins
        reactionTime = 0; // Indicate foul
        matrix.renderBitmap(frame_foul, 8, 12); // Matriz: Mostra X
        
        canRestart = false; // Bloqueia restart imediato
        currentState = FINISHED;
        return;
    }
    if (digitalRead(player2Pin) == HIGH) {
        winner = 1; // P2 pressed too early, P1 wins
        reactionTime = 0; // Indicate foul
        matrix.renderBitmap(frame_foul, 8, 12); // Matriz: Mostra X
        
        canRestart = false; // Bloqueia restart imediato
        currentState = FINISHED;
        return;
    }

    // Check if the delay time is reached
    if (elapsed >= goDelayMs) {
        currentState = GO;
        startTime = millis(); // Reset startTime for reaction time measurement
        lcd.clear();
        
        // --- Matriz Feature: Visual GO ---
        matrix.renderBitmap(frame_go, 8, 12); // Mostra "!" na matriz
        
        return;
    }

    // Update display every 500ms
    if (millis() - lastTime > 500) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Ready...");

        // Show approximate countdown timer
        int secondsLeft = (goDelayMs - elapsed) / 1000 + 1;
        lcd.setCursor(0, 1);
        lcd.print("Wait for GO!");

        lastTime = millis();
    }
}

void ReactionGame::stateGo() {
    // Flash GO message
    static unsigned long lastDisplayed = 0;
    if (millis() - lastDisplayed > 200) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("!!! GO !!!");
        lastDisplayed = millis();
    }

    // Check for P1 Press
    if (digitalRead(player1Pin) == HIGH) {
        reactionTime = millis() - startTime;
        winner = 1;
        matrix.renderBitmap(frame_p1, 8, 12); // Matriz: Vencedor P1
        
        canRestart = false; // OBRIGATÓRIO: Bloqueia restart pois o botão está apertado
        currentState = FINISHED;
        return;
    }

    // Check for P2 Press
    if (digitalRead(player2Pin) == HIGH) {
        reactionTime = millis() - startTime;
        winner = 2;
        matrix.renderBitmap(frame_p2, 8, 12); // Matriz: Vencedor P2
        
        canRestart = false; // Bloqueia restart por segurança
        currentState = FINISHED;
        return;
    }
}

void ReactionGame::stateFinished() {
    static unsigned long lastDisplayed = 0;
    
    // --- Lógica de Segurança do Restart ---
    // Verifica se o botão de Select (que pode ser o P1) foi solto
    if (!canRestart) {
        if (digitalRead(selectButtonPin) == LOW) {
            canRestart = true; // Botão solto, agora pode aceitar comando de restart
        }
    }
    
    // Logica de Restart (Botão Select) - SÓ FUNCIONA SE canRestart FOR TRUE
    if (canRestart && digitalRead(selectButtonPin) == HIGH) {
        lcd.clear();
        lcd.print("Restarting...");
        delay(500); // Debounce
        resetGame();
        return;
    }

    // Update display text every 2 seconds to alternate info
    if (millis() - lastDisplayed > 2000) {
        lcd.clear();
        
        lcd.setCursor(0, 0);
        if (reactionTime > 0) {
            if (winner == 1) lcd.print("P1 Wins! ");
            else lcd.print("P2 Wins! ");
            lcd.print(reactionTime);
            lcd.print("ms");
        } else {
            lcd.print("FOUL! Opp. Wins");
        }
        
        lcd.setCursor(0, 1);
        static bool toggle = false;
        toggle = !toggle;
        
        if (toggle) {
            // Se ainda não pode reiniciar (botão preso), avisa para soltar
            if (!canRestart && winner == 1) { 
                lcd.print("Release Btn P1");
            } else {
                lcd.print("Select: Replay");
            }
        } else {
            lcd.print("Exit: Menu");
        }
        
        lastDisplayed = millis();
    }
}

void ReactionGame::run() {
    switch (currentState) {
        case WAITING:
            break;
        case COUNTDOWN:
            stateCountdown();
            break;
        case GO:
            stateGo();
            break;
        case FINISHED:
            stateFinished();
            break;
    }
}