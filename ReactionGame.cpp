#include "ReactionGame.h"

// Constructor: Initializes the internal references and pins
ReactionGame::ReactionGame(LiquidCrystal& lcdRef, int p1Pin, int p2Pin)
    : lcd(lcdRef), player1Pin(p1Pin), player2Pin(p2Pin) {}

void ReactionGame::setup() {
    // Setup pins (though done in main, harmless to repeat)
    pinMode(player1Pin, INPUT);
    pinMode(player2Pin, INPUT);

    // Create custom characters
    lcd.createChar(0, p1Char); // P1 indicator
    lcd.createChar(1, p2Char); // P2 indicator

    // Display introductory message
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("REACTION GAME");
    drawInstructions();
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

    lcd.clear();
}

void ReactionGame::drawInstructions() {
    // Clear instructions to show P1 vs P2 pins
    lcd.setCursor(0, 1);
    lcd.print("P1:6 vs P2:7");
}

void ReactionGame::stateCountdown() {
    // Calculate time elapsed
    unsigned long elapsed = millis() - startTime;

    // Check for premature press (Foul)
    if (digitalRead(player1Pin) == HIGH) {
        winner = 2; // P1 pressed too early, P2 wins
        reactionTime = 0; // Indicate foul
        currentState = FINISHED;
        return;
    }
    if (digitalRead(player2Pin) == HIGH) {
        winner = 1; // P2 pressed too early, P1 wins
        reactionTime = 0; // Indicate foul
        currentState = FINISHED;
        return;
    }

    // Check if the delay time is reached
    if (elapsed >= goDelayMs) {
        currentState = GO;
        startTime = millis(); // Reset startTime for reaction time measurement
        lcd.clear();
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
        currentState = FINISHED;
        return;
    }

    // Check for P2 Press
    if (digitalRead(player2Pin) == HIGH) {
        reactionTime = millis() - startTime;
        winner = 2;
        currentState = FINISHED;
        return;
    }
}

void ReactionGame::stateFinished() {
    static unsigned long lastDisplayed = 0;
    static int displayToggle = 0; // Controls switching between result and exit prompt

    // Update display every 750ms
    if (millis() - lastDisplayed > 750) {
        lcd.clear();

        if (displayToggle % 2 == 0) {
            // --- Show Result Screen ---
            lcd.setCursor(0, 0);

            if (reactionTime > 0) {
                // No foul: Display Winner
                if (winner == 1) {
                    lcd.print("P1 WINS! ");
                } else if (winner == 2) {
                    lcd.print("P2 WINS! ");
                } else {
                    lcd.print("TIE!");
                }
                // Display time
                lcd.setCursor(0, 1);
                lcd.print(reactionTime);
                lcd.print("ms");
            } else {
                // Foul: Display Foul Message
                lcd.print("FOUL! ");
                lcd.setCursor(0, 1);
                lcd.print("Opponent WINS!");
            }
        } else {
            // --- Show Exit Prompt ---
            lcd.setCursor(0, 0);
            lcd.print("Press EXIT (Pin 8)");
            lcd.setCursor(0, 1);
            lcd.print("to Menu");
        }

        displayToggle++;
        lastDisplayed = millis();
    }
}

void ReactionGame::run() {
    switch (currentState) {
        case WAITING:
            // This state is skipped by the main sketch and setup() immediately calls resetGame()
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

bool ReactionGame::isGameComplete() {
    return false; // Always return false since we use manual exit
}
