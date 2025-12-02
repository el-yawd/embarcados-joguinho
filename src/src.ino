#include <LiquidCrystal.h>
#include "DinoGame.h"
#include "ReactionGame.h"
#include "BlockBreaker.h"
#include "GameMusic.h"

// --- Hardware Setup ---
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// --- Universal Controls ---
const int potPin = A5;         
const int selectButtonPin = 6; 
const int exitButtonPin = 8;   
const int buzzerPin = 9;       

// --- Dedicated Game Controls ---
const int player1Pin = 6;      
const int player2Pin = 7;      

// --- Instantiate Game Objects ---

// DinoGame (Uses LCD + Pin 6)
DinoGame dinoGame(lcd, selectButtonPin);

// ReactionGame (Uses LCD + LED Matrix + Pins 6,7 + Select Button)
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
    RUNNING_BLOCKS,
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

// --- Scrolling Variables ---
unsigned long lastScrollTime = 0;
int scrollPosition = 0;
const int scrollSpeed = 400; 
const int scrollInitialDelay = 1000; 

// --- Helper Functions ---

void printScrollingText(String text, int limit) {
    if (text.length() <= limit) {
        lcd.print(text);
        for (int i = text.length(); i < limit; i++) lcd.print(" ");
    } else {
        String scrollBuffer = text + "   " + text;
        int offset = scrollPosition % (text.length() + 3);
        lcd.print(scrollBuffer.substring(offset, offset + limit));
    }
}

void drawMenu() {
    int potValue = analogRead(potPin);

    // Potentiometer navigation logic
    if (abs(potValue - lastPotValue) > potTolerance) {
        int newSelection = map(potValue, 0, 1023, numMenuItems - 1, 0);

        if (newSelection < 0) newSelection = 0;
        if (newSelection >= numMenuItems) newSelection = numMenuItems - 1;

        if (newSelection != currentSelection) {
            currentSelection = newSelection;
            scrollPosition = 0; 
            lastScrollTime = millis();
            lcd.clear();
        }
        lastPotValue = potValue;
    }
    
    // Update scroll timer
    int currentDelay = (scrollPosition == 0) ? scrollInitialDelay : scrollSpeed;
    if (millis() - lastScrollTime > currentDelay) {
        scrollPosition++;
        lastScrollTime = millis();
    }

    // Draw Menu UI
    lcd.setCursor(0, 0);
    lcd.print(">");
    printScrollingText(menuItems[currentSelection], 15); 

    lcd.setCursor(0, 1);
    if (numMenuItems > 1 && currentSelection < numMenuItems - 1) {
        lcd.print(" "); 
        String nextItem = menuItems[currentSelection + 1];
        if (nextItem.length() > 15) {
            lcd.print(nextItem.substring(0, 15)); 
        } else {
            lcd.print(nextItem);
            for(int i = nextItem.length(); i < 15; i++) lcd.print(" ");
        }
    } else {
        lcd.print("                ");
    }
}

void handleSelection() {
    if (digitalRead(selectButtonPin) == HIGH && (millis() - lastDebounceTime > debounceDelay)) {
        lastDebounceTime = millis();
        
        scrollPosition = 0;
        lastScrollTime = millis();

        switch (currentSelection) {
            case 0: // Dinossaur Jumper
                currentState = RUNNING_DINO;
                gameMusic.startPacmanIntro();
                dinoGame.setup();
                break;
                
            case 1: // Reaction Duel
                currentState = RUNNING_REACTION;
                gameMusic.startPacmanIntro();
                reactionGame.setup();
                break;
                
            case 2: // Brick Breaker
                currentState = RUNNING_BLOCKS;
                gameMusic.startPacmanIntro();
                blockBreaker.start(); 
                break;
                
            case 3: // About
                currentState = ABOUT_SCREEN;
                lcd.clear();
                break;
        }
    }
}

// --- About Info Screen ---
void drawAboutScreen() {
    // Update scroll timer
    int currentDelay = (scrollPosition == 0) ? scrollInitialDelay : scrollSpeed;
    if (millis() - lastScrollTime > currentDelay) {
        scrollPosition++;
        lastScrollTime = millis();
    }

    lcd.setCursor(0, 0);
    printScrollingText("HOME MADE 'GAMEBOY'", 16); 
    lcd.setCursor(0, 1);
    printScrollingText("By Diegos e Kaique ", 16);

    if ((digitalRead(exitButtonPin) == HIGH || digitalRead(selectButtonPin) == HIGH) && (millis() - lastDebounceTime > debounceDelay)) {
        lastDebounceTime = millis();
        currentState = MENU;
        scrollPosition = 0; 
        lastScrollTime = millis();
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
                
                if (currentState == RUNNING_BLOCKS) {
                    blockBreaker.stop();
                }
                if (currentState == RUNNING_REACTION) {
                    reactionGame.stop();
                }
                
                gameMusic.stopMusic(); // Stop music when exiting games
                currentState = MENU;
                scrollPosition = 0; 
                lastScrollTime = millis();
                lcd.clear();
                break; 
            }

            // Update music playback
            gameMusic.update();

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