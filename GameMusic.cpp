#include "GameMusic.h"

// Pacman melody data
const int GameMusic::pacmanMelody[] = {
  // Pacman
  // Score available at https://musescore.com/user/85429/scores/107109
  NOTE_B4, 16, NOTE_B5, 16, NOTE_FS5, 16, NOTE_DS5, 16, //1
  NOTE_B5, 32, NOTE_FS5, -16, NOTE_DS5, 8, NOTE_C5, 16,
  NOTE_C6, 16, NOTE_G6, 16, NOTE_E6, 16, NOTE_C6, 32, NOTE_G6, -16, NOTE_E6, 8,

  NOTE_B4, 16,  NOTE_B5, 16,  NOTE_FS5, 16,   NOTE_DS5, 16,  NOTE_B5, 32,  //2
  NOTE_FS5, -16, NOTE_DS5, 8,  NOTE_DS5, 32, NOTE_E5, 32,  NOTE_F5, 32,
  NOTE_F5, 32,  NOTE_FS5, 32,  NOTE_G5, 32,  NOTE_G5, 32, NOTE_GS5, 32,  NOTE_A5, 16, NOTE_B5, 8
};

const int GameMusic::pacmanMelodySize = sizeof(pacmanMelody) / sizeof(pacmanMelody[0]) / 2;

GameMusic::GameMusic(int pin) {
  buzzerPin = pin;
  tempo = 105; // Default tempo
  wholenote = (60000 * 4) / tempo;
  pinMode(buzzerPin, OUTPUT);
  
  // Initialize non-blocking playback state
  isPlaying = false;
  currentNote = 0;
  noteStartTime = 0;
  currentNoteDuration = 0;
  inPause = false;
}

void GameMusic::startPacmanIntro() {
  isPlaying = true;
  currentNote = 0;
  noteStartTime = millis();
  inPause = false;
  
  // Start the first note
  if (pacmanMelodySize > 0) {
    int divider = pacmanMelody[1];
    if (divider > 0) {
      currentNoteDuration = wholenote / divider;
    } else if (divider < 0) {
      currentNoteDuration = wholenote / abs(divider);
      currentNoteDuration *= 1.5; // Dotted notes
    }
    
    // Start playing the first note (90% duration)
    tone(buzzerPin, pacmanMelody[0], currentNoteDuration * 0.9);
  }
}

void GameMusic::update() {
  if (!isPlaying) return;
  
  unsigned long elapsed = millis() - noteStartTime;
  
  if (!inPause && elapsed >= (currentNoteDuration * 0.9)) {
    // Note finished, start pause
    noTone(buzzerPin);
    inPause = true;
  }
  
  if (inPause && elapsed >= currentNoteDuration) {
    // Pause finished, move to next note
    currentNote += 2; // Each note has pitch and duration
    
    if (currentNote >= pacmanMelodySize * 2) {
      // Song finished, loop it
      currentNote = 0;
    }
    
    // Calculate next note duration
    int divider = pacmanMelody[currentNote + 1];
    if (divider > 0) {
      currentNoteDuration = wholenote / divider;
    } else if (divider < 0) {
      currentNoteDuration = wholenote / abs(divider);
      currentNoteDuration *= 1.5; // Dotted notes
    }
    
    // Start next note
    tone(buzzerPin, pacmanMelody[currentNote], currentNoteDuration * 0.9);
    noteStartTime = millis();
    inPause = false;
  }
}

void GameMusic::stopMusic() {
  isPlaying = false;
  noTone(buzzerPin);
}

bool GameMusic::isPlayingMusic() {
  return isPlaying;
}