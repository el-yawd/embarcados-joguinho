#include "ReactionGame.h"

// --- Matrix Bitmaps (0=Off, 1=On) ---

// Frame: "P1" Winner
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

// Frame: "P2" Winner
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

// Frame: "!" (GO Signal)
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

// Frame: "X" (Foul/False Start)
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

// --- Constructor ---
ReactionGame::ReactionGame(LiquidCrystal& lcdRef, int p1Pin, int p2Pin, int selPin)
    : lcd(lcdRef), player1Pin(p1Pin), player2Pin(p2Pin), selectButtonPin(selPin) {}

// --- Initialization & Control ---

void ReactionGame::begin() {
    matrix.begin(); 
}

void ReactionGame::stop() {
    clearMatrix(); 
}

void ReactionGame::clearMatrix() {
    uint8_t frame[8][12];
    for(int y=0; y<8; y++) for(int x=0; x<12; x++) frame[y][x] = 0;
    matrix.renderBitmap(frame, 8, 12);
}

void ReactionGame::setup() {
    // Pin Setup
    pinMode(player1Pin, INPUT);
    pinMode(player2Pin, INPUT);
    pinMode(selectButtonPin, INPUT);

    // Char Setup
    lcd.createChar(0, p1Char); 
    lcd.createChar(1, p2Char); 

    // Intro UI
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("REACTION GAME");
    drawInstructions();
    
    clearMatrix();
    delay(1500);

    resetGame();
}

void ReactionGame::resetGame() {
    // Randomize Delay
    randomSeed(analogRead(A1)); 
    goDelayMs = random(2000, 5000);

    // Reset State
    winner = 0;
    reactionTime = 0;
    currentState = COUNTDOWN;
    startTime = millis();
    lastTime = millis();
    
    // Safety Flags
    buttonsReleased = false;

    // Reset UI
    lcd.clear();
    clearMatrix(); 
}

void ReactionGame::drawInstructions() {
    lcd.setCursor(0, 1);
    lcd.print("<:P1 vs P2:>");
}

// --- Game Logic States ---

void ReactionGame::stateCountdown() {
    // 1. Safety Phase: Wait for buttons release
    if (!buttonsReleased) {
        if (digitalRead(player1Pin) == LOW && digitalRead(player2Pin) == LOW) {
            buttonsReleased = true;
            startTime = millis(); // Start timer now
            lcd.setCursor(0, 0);
            lcd.print("Ready...      ");
        } else {
            if (millis() - lastTime > 500) {
                lcd.setCursor(0, 0);
                lcd.print("Release Btn!  ");
                lastTime = millis();
            }
        }
        return; 
    }

    // 2. Gameplay Phase
    unsigned long elapsed = millis() - startTime;

    // Check False Start (P1)
    if (digitalRead(player1Pin) == HIGH) {
        winner = 2; // P2 Wins
        reactionTime = 0; 
        matrix.renderBitmap(frame_foul, 8, 12); 
        
        canRestart = false; 
        currentState = FINISHED;
        return;
    }
    // Check False Start (P2)
    if (digitalRead(player2Pin) == HIGH) {
        winner = 1; // P1 Wins
        reactionTime = 0; 
        matrix.renderBitmap(frame_foul, 8, 12); 
        
        canRestart = false; 
        currentState = FINISHED;
        return;
    }

    // Check Trigger Time
    if (elapsed >= goDelayMs) {
        currentState = GO;
        startTime = millis(); 
        lcd.clear();
        matrix.renderBitmap(frame_go, 8, 12); // Visual GO
        return;
    }

    // Update UI (Countdown)
    if (millis() - lastTime > 500) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Ready...");

        int secondsLeft = (goDelayMs - elapsed) / 1000 + 1;
        lcd.setCursor(0, 1);
        lcd.print("Wait for GO!");

        lastTime = millis();
    }
}

void ReactionGame::stateGo() {
    // Flash GO Message
    static unsigned long lastDisplayed = 0;
    if (millis() - lastDisplayed > 200) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("!!! GO !!!");
        lastDisplayed = millis();
    }

    // Check P1 Win
    if (digitalRead(player1Pin) == HIGH) {
        reactionTime = millis() - startTime;
        winner = 1;
        matrix.renderBitmap(frame_p1, 8, 12); 
        
        canRestart = false; 
        currentState = FINISHED;
        return;
    }

    // Check P2 Win
    if (digitalRead(player2Pin) == HIGH) {
        reactionTime = millis() - startTime;
        winner = 2;
        matrix.renderBitmap(frame_p2, 8, 12); 
        
        canRestart = false; 
        currentState = FINISHED;
        return;
    }
}

void ReactionGame::stateFinished() {
    static unsigned long lastDisplayed = 0;
    
    // --- Restart Safety Logic ---
    if (!canRestart) {
        if (digitalRead(selectButtonPin) == LOW) {
            canRestart = true; // Button released, ready for input
        }
    }
    
    // Check Restart Command
    if (canRestart && digitalRead(selectButtonPin) == HIGH) {
        lcd.clear();
        lcd.print("Restarting...");
        delay(500); 
        resetGame();
        return;
    }

    // Toggle UI Info
    if (millis() - lastDisplayed > 2000) {
        lcd.clear();
        
        // Line 0: Result
        lcd.setCursor(0, 0);
        if (reactionTime > 0) {
            if (winner == 1) lcd.print("P1 Wins! ");
            else lcd.print("P2 Wins! ");
            lcd.print(reactionTime);
            lcd.print("ms");
        } else {
            // FIX: Mensagem de falta específica
            if (winner == 1) {
                // Vencedor é P1, logo P2 cometeu a falta
                lcd.print("P2 FOUL! P1 WINS");
            } else {
                // Vencedor é P2, logo P1 cometeu a falta
                lcd.print("P1 FOUL! P2 WINS");
            }
        }
        
        // Line 1: Instructions
        lcd.setCursor(0, 1);
        static bool toggle = false;
        toggle = !toggle;
        
        if (toggle) {
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

// --- Main Loop ---
void ReactionGame::run() {
    switch (currentState) {
        case WAITING:   break;
        case COUNTDOWN: stateCountdown(); break;
        case GO:        stateGo();        break;
        case FINISHED:  stateFinished();  break;
    }
}