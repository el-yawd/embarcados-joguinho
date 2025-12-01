#include "BlockBreaker.h"

// --- Constructor ---
BlockBreaker::BlockBreaker(LiquidCrystal& lcdRef, int pPin, int bPin) 
    : lcd(lcdRef), potPin(pPin), buttonPin(bPin) {
}

// --- Initialization & Control ---

void BlockBreaker::begin() {
    matrix.begin(); // Hardware init
    pinMode(buttonPin, INPUT);
}

void BlockBreaker::start() {
    // Intro UI
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("BRICK");
    lcd.setCursor(0, 1);
    lcd.print("BREAKER");
    delay(3000);
    
    resetGame();
}

void BlockBreaker::stop() {
    // Clear Buffer
    for(int y=0; y<8; y++) {
        for(int x=0; x<12; x++) {
            frame[y][x] = 0;
        }
    }
    // Update Matrix
    matrix.renderBitmap(frame, 8, 12);
}

// --- Game Setup Helpers ---

void BlockBreaker::initBricks() {
    totalBricks = 0;
    // Create 3 rows of bricks
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
    
    // Initial Ball Position
    ballX = 6;
    ballY = 6; 
    ballDirX = 0.5; 
    ballDirY = -1;  
    
    lastBallUpdate = millis();
    
    // Reset UI
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Score: 0");
    lcd.setCursor(0, 1);
    lcd.print("Press Jump/Start");
}

// --- Physics Logic ---

void BlockBreaker::updatePaddle() {
    int potValue = analogRead(potPin);
    // Map Pot to Matrix Width
    paddleX = map(potValue, 0, 1023, 12 - paddleWidth, 0);
    
    // Bounds Check
    if (paddleX < 0) paddleX = 0;
    if (paddleX > (12 - paddleWidth)) paddleX = 12 - paddleWidth;
}

void BlockBreaker::updateBall() {
    if (millis() - lastBallUpdate > ballSpeedDelay) {
        lastBallUpdate = millis();
        
        // Calculate Next Pos
        float nextX = ballX + ballDirX;
        float nextY = ballY + ballDirY;
        
        // 1. Wall Collision (Left/Right)
        if (nextX < 0 || nextX >= 12) {
            ballDirX *= -1; 
            nextX = ballX + ballDirX; 
        }
        
        // 2. Ceiling Collision
        if (nextY < 0) {
            ballDirY *= -1; 
            nextY = ballY + ballDirY;
        }
        
        // 3. Floor Collision (Game Over)
        if (nextY >= 8) {
            state = BB_GAME_OVER;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("GAME OVER!");
            lcd.setCursor(0, 1);
            lcd.print("Btn to Restart");
            return;
        }
        
        // 4. Paddle Collision
        if (ballDirY > 0 && nextY >= 7 && ballY < 8) {
            int ballIntX = (int)nextX;
            if (ballIntX >= paddleX && ballIntX < paddleX + paddleWidth) {
                ballDirY *= -1; // Bounce Up
                
                // Paddle Angle Physics
                if (ballIntX == paddleX) ballDirX = -0.7; 
                else if (ballIntX == paddleX + paddleWidth - 1) ballDirX = 0.7; 
                
                nextY = ballY + ballDirY; 
            }
        }
        
        // 5. Brick Collision
        int brickX = (int)nextX;
        int brickY = (int)nextY;
        
        if (brickY >= 0 && brickY < 3 && brickX >= 0 && brickX < 12) {
            if (bricks[brickY][brickX]) {
                bricks[brickY][brickX] = false; // Destroy
                totalBricks--;
                ballDirY *= -1; // Bounce
                
                // Update Score
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
        
        // Apply Physics
        ballX += ballDirX;
        ballY += ballDirY;
    }
}

// --- Rendering ---

void BlockBreaker::draw() {
    // Clear Buffer
    for(int y=0; y<8; y++) {
        for(int x=0; x<12; x++) {
            frame[y][x] = 0;
        }
    }
    
    if (state == BB_VICTORY) {
        // Smiley Face
        frame[2][3]=1; frame[2][8]=1; 
        frame[5][3]=1; frame[6][4]=1; frame[6][5]=1; frame[6][6]=1; frame[6][7]=1; frame[5][8]=1; 
    } 
    else if (state == BB_GAME_OVER) {
        // Big X
        frame[1][2]=1; frame[2][3]=1; frame[3][4]=1; frame[4][5]=1; frame[5][6]=1;
        frame[5][2]=1; frame[4][3]=1; frame[3][4]=1; frame[2][5]=1; frame[1][6]=1;
    }
    else {
        // 1. Bricks
        for(int y=0; y<3; y++) {
            for(int x=0; x<12; x++) {
                if (bricks[y][x]) frame[y][x] = 1;
            }
        }
        
        // 2. Paddle
        for(int i=0; i<paddleWidth; i++) {
             int px = paddleX + i;
             if(px < 12) frame[7][px] = 1;
        }
        
        // 3. Ball
        int bx = (int)ballX;
        int by = (int)ballY;
        if(bx >= 0 && bx < 12 && by >= 0 && by < 8) {
            frame[by][bx] = 1;
        }
    }
    
    // Push to Hardware
    matrix.renderBitmap(frame, 8, 12);
}

// --- Main Loop ---
void BlockBreaker::run() {
    updatePaddle(); 
    
    if (state == BB_WAITING) {
        draw();
        // Start Trigger
        if (digitalRead(buttonPin) == HIGH) {
            state = BB_PLAYING;
            lcd.setCursor(0, 1);
            lcd.print("Running...      ");
            delay(200); 
        }
    }
    else if (state == BB_PLAYING) {
        updateBall();
        draw();
    }
    else {
        // End State (Win/Loss)
        draw();
        // Restart Trigger
        if (digitalRead(buttonPin) == HIGH) {
            resetGame();
            delay(500); 
        }
    }
}