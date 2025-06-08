/*
  --------------------------------------------------------------
  PROGETTO: Cassaforte Elettronica con Arduino, Tastierino 4x4, Buzzer e Solenoide
  Autore: Dario
  Versione: 1.2
  --------------------------------------------------------------

  DESCRIZIONE:
  Questo sketch permette di controllare una serratura elettromagnetica (solenoide)
  utilizzando un tastierino 4x4 per l'inserimento di una password.

  Il sistema supporta:
  - Salvataggio della password in EEPROM (memoria non volatile)
  - Cambio password tramite tastierino (con A#)
  - Reset software alla password di default
  - Suoni di feedback (corretto, errato, serratura chiusa)
  - Apertura/chiusura alternata digitando la password corretta

  COMPONENTI UTILIZZATI:
  - Arduino Uno
  - Tastierino matriciale 4x4
  - Buzzer passivo (PIN 13)
  - Solenoide 5V per elettroserratura (PIN 12)
  - Alimentazione esterna per il solenoide (consigliata)

  PINOUT:
  - Tastierino righe: 9, 8, 7, 6
  - Tastierino colonne: 5, 4, 3, 2
  - Buzzer: 13
  - Solenoide (PISTON): 12

  COMANDI DISPONIBILI:
  - *         => Cancella l’input corrente
  - #         => Invia l’input per verifica
  - A#        => Cambia password senza chiedere quella vecchia
  - B#        => Reset software della password a "2612"
  - D#        => Stampa via seriale la password attuale (debug)
*/

#include <Keypad.h>
#include <EEPROM.h>

#define NOTE_C4 262
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_B4 494

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
bool isLocked = true;
bool preparePasswordChange = false;

void setup() {
  Serial.begin(9600);
  pinMode(PISTON_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(PISTON_PIN, HIGH);

  writePasswordToEEPROM("2612");
  delay(50);
  currentPassword = readPasswordFromEEPROM();
  Serial.print("Password inizializzata a: ");
  Serial.println(currentPassword);

  Serial.println("Sistema pronto. Premi A# per cambiare password.");
  Serial.println("Per resettare la password, digita B#");
}

void loop() {
  char key = keypad.getKey();
  if (key == NO_KEY) return;

  tone(BUZZER_PIN, 1000, 50);
  Serial.print("Tasto premuto: ");
  Serial.println(key);

  if (key == '*') {
    inputPassword = "";
    preparePasswordChange = false;
    Serial.println("Input cancellato.");
    return;
  }

  if (key == '#') {
    if (inputPassword == "D") {
      Serial.print("Password salvata in EEPROM: ");
      Serial.println(currentPassword);
    } else if (inputPassword == "B") {
      currentPassword = "2612";
      writePasswordToEEPROM(currentPassword);
      Serial.println("Password resettata a default: 2612");
      successSound();
    } else if (inputPassword == "A") {
      Serial.println("Inserisci nuova password:");
      preparePasswordChange = true;
    } else if (preparePasswordChange) {
      writePasswordToEEPROM(inputPassword);
      currentPassword = inputPassword;
      Serial.print("Nuova password salvata: ");
      Serial.println(currentPassword);
      successSound();
      preparePasswordChange = false;
    } else {
      if (inputPassword == currentPassword) {
        if (isLocked) {
          Serial.println("Sblocco.");
          digitalWrite(PISTON_PIN, LOW);
          isLocked = false;
          successSound();
        } else {
          Serial.println("Richiusura.");
          digitalWrite(PISTON_PIN, HIGH);
          isLocked = true;
          lockSound();
        }
      } else {
        Serial.print("Inserito: "); Serial.println(inputPassword);
        Serial.print("Atteso: "); Serial.println(currentPassword);
        Serial.println("Password errata!");
        errorSound();
      }
    }
    inputPassword = "";
    return;
  }

  inputPassword += key;
}

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
    pass += char(EEPROM.read(EEPROM_ADDR + 1 + i));
  }
  return pass;
}

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
