#include <LiquidCrystal.h>
#include "DinoGame.h"
#include "ReactionGame.h"
#include "BlockBreaker.h" // INCLUIR O NOVO JOGO
#include "GameMusic.h"

// --- Hardware Setup ---
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Universal Controls
const int potPin = A5;         // Analog pin for potentiometer (Menu & BlockBreaker)
const int selectButtonPin = 6; // Digital pin for selection, Dino Jump, and BlockBreaker Start
const int exitButtonPin = 8;   // Digital pin for universal Exit to Menu
const int buzzerPin = 9;       // Digital pin for buzzer

// Dedicated Game Controls
const int player1Pin = 6;      // Reaction Game P1
const int player2Pin = 7;      // Reaction Game P2

// --- Instantiate Game Objects ---

// DinoGame (Uses LCD + Pin 6)
DinoGame dinoGame(lcd, selectButtonPin);

// ReactionGame (ATUALIZADO: Agora recebe selectButtonPin também)
ReactionGame reactionGame(lcd, player1Pin, player2Pin, selectButtonPin);

// BlockBreaker (Uses LCD + LED Matrix + Pot A5 + Pin 6)
BlockBreaker blockBreaker(lcd, potPin, selectButtonPin);

// Music System (Uses Pin 9)
GameMusic gameMusic(buzzerPin);

// --- Application State Management ---
enum AppState {
    MENU,
    RUNNING_DINO,
    RUNNING_REACTION,
    RUNNING_BLOCKS, // Novo Estado
    ABOUT_SCREEN
};
AppState currentState = MENU;

// --- Menu Variables ---
const char* menuItems[] = {
    "Dinossaur Jumper",
    "Reaction Duel",
    "Brick Breaker",
    "About/Info"
};
const int numMenuItems = sizeof(menuItems) / sizeof(menuItems[0]);
int currentSelection = 0;

const int potTolerance = 10; 
int lastPotValue = 0;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; 

// --- Helper Functions ---

void drawMenu() {
    int potValue = analogRead(potPin);

    // Potentiometer navigation logic
    if (abs(potValue - lastPotValue) > potTolerance) {
        int newSelection = map(potValue, 0, 1023, numMenuItems - 1, 0);

        if (newSelection < 0) newSelection = 0;
        if (newSelection >= numMenuItems) newSelection = numMenuItems - 1;

        if (newSelection != currentSelection) {
            currentSelection = newSelection;
            lcd.clear();
        }
        lastPotValue = potValue;
    }

    // Draw Menu UI
    lcd.setCursor(0, 0);
    lcd.print(">");
    for (int i = 0; i < 15 && menuItems[currentSelection][i] != '\0'; ++i) {
        lcd.write(menuItems[currentSelection][i]);
    }
    // Clear rest of line 0
    for(size_t i = 1 + strlen(menuItems[currentSelection]); i < 16; ++i) {
        lcd.write(' ');
    }

    // Show next item on bottom row
    lcd.setCursor(0, 1);
    if (numMenuItems > 1 && currentSelection < numMenuItems - 1) {
        lcd.print(" ");
        int nextItemIndex = currentSelection + 1;
        for (int i = 0; i < 15 && menuItems[nextItemIndex][i] != '\0'; ++i) {
            lcd.write(menuItems[nextItemIndex][i]);
        }
    } else {
        lcd.print("                ");
    }
}

void handleSelection() {
    if (digitalRead(selectButtonPin) == HIGH && (millis() - lastDebounceTime > debounceDelay)) {
        lastDebounceTime = millis();

        switch (currentSelection) {
            case 0: // Dino Game
                currentState = RUNNING_DINO;
                gameMusic.startPacmanIntro();
                dinoGame.setup();
                break;
                
            case 1: // Reaction Game
                currentState = RUNNING_REACTION;
                gameMusic.startPacmanIntro();
                reactionGame.setup();
                break;
                
            case 2: // Matrix Blocks (NOVO)
                currentState = RUNNING_BLOCKS;
                gameMusic.startPacmanIntro();
                // IMPORTANTE: Mantive blockBreaker.start() 
                blockBreaker.start(); 
                break;
                
            case 3: // About
                currentState = ABOUT_SCREEN;
                lcd.clear();
                break;
        }
    }
}

void drawAboutScreen() {
    lcd.setCursor(0, 0);
    lcd.print("HOME MADE 'GAMEBOY'"); 
    lcd.setCursor(0, 1);
    lcd.print("By Diegos e Kaique");

    if (digitalRead(selectButtonPin) == HIGH && (millis() - lastDebounceTime > debounceDelay)) {
        lastDebounceTime = millis();
        currentState = MENU;
        lcd.clear();
    }
}

// --- Main Setup ---
void setup() {
    lcd.begin(16, 2);
    
    // Initialize Input Pins
    pinMode(selectButtonPin, INPUT);
    pinMode(exitButtonPin, INPUT);
    pinMode(player1Pin, INPUT);
    pinMode(player2Pin, INPUT);
    pinMode(buzzerPin, OUTPUT);

    // IMPORTANTE: Inicializa o hardware da matriz UMA VEZ aqui
    // Se blockBreaker.begin() já chama matrix.begin(), tecnicamente não precisa chamar 
    // reactionGame.begin() se ambos usam a mesma lib singleton, mas é boa prática
    // garantir que pelo menos um chame.
    blockBreaker.begin();
    reactionGame.begin();

    lastPotValue = analogRead(potPin);

    // Intro Screen
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Arduino R4 WiFi");
    lcd.setCursor(0, 1);
    lcd.print("GAMEBOI CASERO");
    delay(1000);
    lcd.clear();
}

// --- Main Loop ---
void loop() {
    switch (currentState) {
        case MENU:
            drawMenu();
            handleSelection();
            break;

        case RUNNING_DINO:
        case RUNNING_REACTION:
        case RUNNING_BLOCKS: 
            // Universal Exit (Pin 8)
            if (digitalRead(exitButtonPin) == HIGH && (millis() - lastDebounceTime > debounceDelay)) {
                lastDebounceTime = millis();
                
                // --- FIX: Limpar Matriz ao Sair ---
                if (currentState == RUNNING_BLOCKS) {
                    blockBreaker.stop();
                }
                if (currentState == RUNNING_REACTION) {
                    reactionGame.stop(); // Limpa matriz do ReactionGame também
                }
                
                currentState = MENU;
                lcd.clear();
                break; 
            }

            // Game Logic Dispatch
            if (currentState == RUNNING_DINO) {
                dinoGame.run();
            } 
            else if (currentState == RUNNING_REACTION) {
                reactionGame.run();
            }
            else if (currentState == RUNNING_BLOCKS) {
                blockBreaker.run();
            }
            break;

        case ABOUT_SCREEN:
            drawAboutScreen();
            break;
    }

    delay(30); // Loop pacing
}