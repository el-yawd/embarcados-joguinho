#include <LiquidCrystal.h>
#include "DinoGame.h"
#include "ReactionGame.h" // New game include
#include "GameMusic.h" // Music system include

// --- Hardware Setup ---
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Universal Controls
const int potPin = A5;         // Analog pin for potentiometer (Menu Navigation)
const int selectButtonPin = 7; // Digital pin for selection (Menu) and JUMP (Dino)
const int exitButtonPin = 8;   // Digital pin for universal Exit to Menu
const int buzzerPin = 9;       // Digital pin for buzzer/speaker

// Dedicated Game Controls (New)
const int player1Pin = 6;      // Digital pin for Reaction Game Player 1
const int player2Pin = 7;      // Digital pin for Reaction Game Player 2


// Instantiate game objects
// DinoGame uses Pin 7 for Jump
DinoGame dinoGame(lcd, selectButtonPin);
// ReactionGame uses Pins 6 and 7 for P1 and P2
ReactionGame reactionGame(lcd, player1Pin, player2Pin);
// Music system uses Pin 9 for buzzer
GameMusic gameMusic(buzzerPin);

// --- Application State Management ---
enum AppState {
    MENU,
    RUNNING_DINO,
    RUNNING_REACTION, // New game state
    ABOUT_SCREEN
};
AppState currentState = MENU;

// --- Menu Variables ---
const char* menuItems[] = {
    "Play Dino Game",
    "Play Reaction Game", // New Menu Item
    "About/Info"
};
const int numMenuItems = sizeof(menuItems) / sizeof(menuItems[0]);
int currentSelection = 0;

const int potTolerance = 10; // Dead zone for pot movement
int lastPotValue = 0;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // ms

// Helper to draw the menu and handle potentiometer input
void drawMenu() {
    // 1. Read Potentiometer and update selection
    int potValue = analogRead(potPin);

    // Check if the pot value has changed significantly
    if (abs(potValue - lastPotValue) > potTolerance) {
        // Map the 0-1023 analog range to the menu index range (0 to numMenuItems - 1)
        int newSelection = map(potValue, 0, 1023, 0, numMenuItems - 1);

        if (newSelection < 0) newSelection = 0;
        if (newSelection >= numMenuItems) newSelection = numMenuItems - 1;

        // Update selection and redraw screen
        if (newSelection != currentSelection) {
            currentSelection = newSelection;
            lcd.clear();
        }
        lastPotValue = potValue;
    }

    // 2. Draw Menu
    // Show the currently selected item on the top row (with selector arrow)
    lcd.setCursor(0, 0);
    lcd.print(">");
    // Print the selected item (max 15 chars after '>')
    for (int i = 0; i < 15 && menuItems[currentSelection][i] != '\0'; ++i) {
        lcd.write(menuItems[currentSelection][i]);
    }
    // Clear rest of line 0
    for(size_t i = 1 + strlen(menuItems[currentSelection]); i < 16; ++i) {
        lcd.write(' ');
    }


    // Show the next item (if available) on the bottom row
    lcd.setCursor(0, 1);

    if (numMenuItems > 1 && currentSelection < numMenuItems - 1) {
        // Show the item immediately following the selected one
        lcd.print(" "); // No selector on the secondary item
        // Print the next item (max 15 chars after space)
        int nextItemIndex = currentSelection + 1;
        for (int i = 0; i < 15 && menuItems[nextItemIndex][i] != '\0'; ++i) {
            lcd.write(menuItems[nextItemIndex][i]);
        }
    } else {
        // Clear line 1 if no next item
        lcd.print("                ");
    }
}

// Handles the select button press
void handleSelection() {
    // Read the select button (using debounce logic)
    if (digitalRead(selectButtonPin) == HIGH && (millis() - lastDebounceTime > debounceDelay)) {
        lastDebounceTime = millis();

        // Execute action based on selection
        switch (currentSelection) {
            case 0: // "Play Dino Game"
                currentState = RUNNING_DINO;
                gameMusic.startPacmanIntro(); // Start background music
                dinoGame.setup(); // Initialize game assets and state
                break;
            case 1: // "Play Reaction Game"
                currentState = RUNNING_REACTION;
                gameMusic.startPacmanIntro(); // Start background music
                reactionGame.setup();
                break;
            case 2: // "About/Info"
                currentState = ABOUT_SCREEN;
                lcd.clear();
                break;
        }
    }
}

// Draws the "About" screen
void drawAboutScreen() {
    lcd.setCursor(0, 0);
    lcd.print(" LCD Arcade V1 ");
    lcd.setCursor(0, 1);
    lcd.print("P1:6 P2:7 E:8 B:9");

    // Use the select button (7) to return to menu
    if (digitalRead(selectButtonPin) == HIGH && (millis() - lastDebounceTime > debounceDelay)) {
        lastDebounceTime = millis();
        currentState = MENU;
        lcd.clear();
    }
}


void setup() {
    lcd.begin(16, 2);
    // Initialize pins
    pinMode(selectButtonPin, INPUT);
    pinMode(exitButtonPin, INPUT); // Initialize pin 8 as exit button

    // Reaction Game Pins (need to be initialized here or in ReactionGame setup)
    pinMode(player1Pin, INPUT);
    pinMode(player2Pin, INPUT);

    // Buzzer pin (initialized in GameMusic constructor, but good to be explicit)
    pinMode(buzzerPin, OUTPUT);

    // Initial read for the pot value
    lastPotValue = analogRead(potPin);

    // Initial State Message
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LCD Menu System");
    lcd.setCursor(0, 1);
    lcd.print("Ready...");
    delay(1000);
    lcd.clear();
}

void loop() {
    switch (currentState) {
        case MENU:
            drawMenu();
            handleSelection();
            break;

        case RUNNING_DINO:
        case RUNNING_REACTION:
            // --- Universal Exit Logic (Pin 8 Check) ---
            if (digitalRead(exitButtonPin) == HIGH && (millis() - lastDebounceTime > debounceDelay)) {
                lastDebounceTime = millis();
                currentState = MENU; // Exit to menu
                lcd.clear();
                break; // Break out of the switch case for this loop iteration
            }
            // --- Run Game Logic ---
            if (currentState == RUNNING_DINO) {
                dinoGame.run();
            } else { // currentState == RUNNING_REACTION
                reactionGame.run();
            }
            break;

        case ABOUT_SCREEN:
            drawAboutScreen();
            break;
    }

    // Delay controls the overall loop speed
    delay(30);
}
