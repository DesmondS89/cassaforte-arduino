# Cassaforte elettronica con tastierino 4x4, buzzer, EEPROM e solenoide

Un progetto DIY basato su **Arduino Uno**, pensato per creare una **serratura elettronica** con:

- 🔢 Tastierino numerico 4x4
- 🔒 Solenoide 5V per blocco/sblocco
- 🔊 Feedback sonoro tramite buzzer
- 💾 Password salvata in EEPROM
- 🎛 Cambio e reset della password via tastierino

## 🛠 Funzionalità principali

- ✅ Apertura e chiusura controllata da password
- 🔁 Cambio password con il tasto `A`
- 🔄 Reset password con la sequenza `*#*#` → ritorna a `"2612"`
- 🧠 Memorizzazione in EEPROM per mantenere la password dopo lo spegnimento
- 🔔 Feedback sonoro per successo, errore, chiusura

## 🧩 Componenti utilizzati

| Componente           | Dettagli                             |
|----------------------|--------------------------------------|
| Arduino Uno          | Microcontrollore principale          |
| Keypad 4x4           | Input numerico                       |
| Buzzer passivo       | Per segnalazioni sonore              |
| Solenoide 5V         | Serratura elettromagnetica           |
| Alimentatore 5V/9V   | Per alimentare il solenoide          |
| EEPROM interna       | Per salvataggio password persistente |

## 🔌 Collegamenti hardware

- Keypad righe → pin 9, 8, 7, 6
- Keypad colonne → pin 5, 4, 3, 2
- Buzzer → pin 13
- Solenoide → pin 12 (consigliato usare un MOSFET e diodo di protezione)

## 🧪 Modalità d'uso

### 🔓 Sblocco

1. Inserisci la password → premi `#`
2. Se corretta, la serratura si apre (buzz positivo)

### 🔐 Richiusura

1. Reinserisci la password → premi `#`
2. La serratura si richiude (buzz differente)

### 🔄 Cambio password

1. Premi `A`
2. Inserisci password attuale + `#`
3. Inserisci nuova password + `#`

### 🧼 Reset password

- Inserisci `*#*#` → la password torna a `"2612"`

## 📂 File inclusi

- `cassaforte.ino` – sketch completo
- `media/` – immagini del montaggio e prototipo
- `docs/` – eventuale `diagram.json` Wokwi

---

## 🪪 Licenza

MIT – puoi riutilizzare, migliorare o remixare questo progetto, basta citare l'autore :)

---