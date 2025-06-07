/*
  --------------------------------------------------------------
  PROGETTO: Cassaforte Elettronica con Arduino, Tastierino 4x4, Buzzer e Solenoide
  Autore: Dario
  Versione: 1.0
  --------------------------------------------------------------

  DESCRIZIONE:
  Questo sketch permette di controllare una serratura elettromagnetica (solenoide)
  utilizzando un tastierino 4x4 per l'inserimento di una password.

  Il sistema supporta:
  - Salvataggio della password in EEPROM (memoria non volatile)
  - Cambio password tramite tastierino
  - Reset software alla password di default
  - Suoni di feedback (corretto, errato, serratura chiusa)
  - Apertura/chiusura alternata digitando la password corretta

  COMPONENTI UTILIZZATI:
  - Arduino Uno
  - Tastierino matriciale 4x4
  - Buzzer passivo (PIN 13)
  - Solenoide 5V per elettroserratura (PIN 12)
  - Alimentazione esterna per il solenoide (consigliata)
  - (Facoltativi: LED di stato, relè, Mosfet per gestione solenoide)

  PINOUT:
  - Tastierino righe: 9, 8, 7, 6
  - Tastierino colonne: 5, 4, 3, 2
  - Buzzer: 13
  - Solenoide (PISTON): 12

  COMANDI DISPONIBILI:
  - *         => Cancella l’input corrente
  - #         => Invia l’input per verifica
  - A         => Inizia procedura di cambio password
  - *#*#      => Reset software della password a "2612"

  PROCEDURA CAMBIO PASSWORD:
  1. Premi A
  2. Inserisci password attuale, poi premi #
  3. Inserisci nuova password, poi premi #

  COMPORTAMENTO:
  - Se la password è corretta:
      - Se la serratura è chiusa → si apre (solenoide diseccitato)
      - Se è già aperta → si richiude (solenoide eccitato)
  - Se la password è errata:
      - Segnale sonoro di errore
  - La password viene salvata in EEPROM ed è persistente anche dopo lo spegnimento

  DEFAULT:
  - Password di default: 2612
  - La password viene automaticamente inizializzata se EEPROM è vuota
*/

#include <Keypad.h>
#include <EEPROM.h>

// NOTE musicali
#define NOTE_C4 262
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_B4 494
#define NOTE_D5 587

#define PISTON_PIN 12
#define BUZZER_PIN 13
#define EEPROM_ADDR 0

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String inputPassword = "";
String currentPassword = "";
String newPassword = "";
String resetSequence = "";  // Per il reset via *#*#
bool changingPassword = false;
bool waitingNewPassword = false;

void setup() {
  Serial.begin(9600);
  pinMode(PISTON_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(PISTON_PIN, HIGH); // chiusa

  currentPassword = readPasswordFromEEPROM();
  if (currentPassword == "") {
    currentPassword = "1234"; // default
    writePasswordToEEPROM(currentPassword);
  }

  Serial.println("Sistema pronto. Premi A per cambiare password.");
  Serial.println("Per resettare la password, digita *#*#");
}

void loop() {
  char key = keypad.getKey();
  if (key == NO_KEY) return;

  Serial.print("Tasto premuto: ");
  Serial.println(key);

  if (key == '*') {
    inputPassword = "";
    newPassword = "";
    changingPassword = false;
    waitingNewPassword = false;
    Serial.println("Input cancellato.");
    return;
  }

  if (key == 'A') {
    inputPassword = "";
    changingPassword = true;
    waitingNewPassword = false;
    Serial.println("Cambio password attivato. Inserisci password attuale:");
    return;
  }

  if (key == '#') {
    if (changingPassword) {
      if (!waitingNewPassword) {
        if (inputPassword == currentPassword) {
          Serial.println("Password corretta. Inserisci nuova password:");
          inputPassword = "";
          waitingNewPassword = true;
        } else {
          Serial.println("Password attuale errata.");
          errorSound();
          changingPassword = false;
        }
      } else {
        newPassword = inputPassword;
        writePasswordToEEPROM(newPassword);
        currentPassword = newPassword;
        Serial.print("Nuova password salvata: ");
        Serial.println(newPassword);
        successSound();
        changingPassword = false;
        waitingNewPassword = false;
      }
      inputPassword = "";
      return;
    }

    // Normale sblocco/chiusura
    if (inputPassword == currentPassword) {
      if (digitalRead(PISTON_PIN) == HIGH) {
        Serial.println("Sblocco.");
        digitalWrite(PISTON_PIN, LOW);
        successSound();
      } else {
        Serial.println("Richiusura.");
        digitalWrite(PISTON_PIN, HIGH);
        lockSound();
      }
    } else {
      Serial.println("Password errata!");
      errorSound();
    }
    inputPassword = "";
    return;
  }

  // Gestione sequenza di reset *#*#
  resetSequence += key;
  if (resetSequence.length() > 4) {
    resetSequence = resetSequence.substring(resetSequence.length() - 4);
  }
  if (resetSequence == "*#*#") {
    currentPassword = "2612";
    writePasswordToEEPROM(currentPassword);
    Serial.println("Password resettata a default: 2612");
    successSound();
    resetSequence = "";
    inputPassword = "";
  }

  inputPassword += key;
}

// EEPROM I/O
void writePasswordToEEPROM(String password) {
  int len = password.length();
  EEPROM.write(EEPROM_ADDR, len);
  for (int i = 0; i < len; i++) {
    EEPROM.write(EEPROM_ADDR + 1 + i, password[i]);
  }
}

String readPasswordFromEEPROM() {
  int len = EEPROM.read(EEPROM_ADDR);
  if (len < 1 || len > 16) return "";
  String pass = "";
  for (int i = 0; i < len; i++) {
    char c = EEPROM.read(EEPROM_ADDR + 1 + i);
    pass += c;
  }
  return pass;
}

// Suoni
void successSound() {
  tone(BUZZER_PIN, NOTE_C4, 150); delay(200);
  tone(BUZZER_PIN, NOTE_E4, 150); delay(200);
  tone(BUZZER_PIN, NOTE_G4, 250); delay(300);
  noTone(BUZZER_PIN);
}

void errorSound() {
  tone(BUZZER_PIN, NOTE_B4, 200); delay(250);
  tone(BUZZER_PIN, NOTE_B4, 200); delay(250);
  noTone(BUZZER_PIN);
}

void lockSound() {
  tone(BUZZER_PIN, NOTE_G4, 150); delay(200);
  tone(BUZZER_PIN, NOTE_E4, 150); delay(200);
  tone(BUZZER_PIN, NOTE_C4, 250); delay(300);
  noTone(BUZZER_PIN);
}
