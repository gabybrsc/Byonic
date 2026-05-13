# 🤖 Byonic — Robot Autonom de Cartografiere a Mediului

Byonic este un robot autonom de dimensiuni mici care se deplasează independent și colectează în timp real date despre **temperatură**, **umiditate**, **calitatea aerului**, **gaze** și **umiditatea solului**. Datele sunt vizualizate live într-un dashboard web accesibil de pe orice telefon sau laptop, fără router sau internet — robotul își creează propria rețea WiFi.

---

## 📋 Componente necesare

| Componentă | Cantitate |
|---|---|
| Arduino Uno | 1 |
| ESP32 Dev Board | 1 |
| DHT11 (senzor temperatură + umiditate) | 1 |
| Senzor umiditate sol | 1 |
| Senzor gaz MQ-2 | 1 |
| HC-SR04 (senzor ultrasonic) | 1 |
| Breadboard | 1 |
| Fire jumper | ~30 |
| Rezistențe 10kΩ | 1 |
| Rezistențe 1kΩ și 2kΩ | câte 1 |
| Baterie 9V + conector | 1 |
| Servo continuu SG90 | 2 |
| Șuruburi M4×12 și M4×16 | ~10 |
| Filament PLA (pentru șasiu) | ~80g |

---

## 🔌 Schema de conectare

### ESP32
| Pin ESP32 | Conectat la |
|---|---|
| GPIO4 | DHT11 DATA |
| GPIO34 | Senzor sol AOUT |
| GPIO35 | MQ-2 AOUT |
| GPIO18 | HC-SR04 TRIG |
| GPIO19 | HC-SR04 ECHO (prin divizor 1kΩ + 2kΩ) |
| GPIO17 | Arduino pin 0 (RX) |
| 3V3 | VCC senzori |
| GND | GND comun |

### Arduino Uno
| Pin Arduino | Conectat la |
|---|---|
| D5 (PWM) | Servo stânga — semnal |
| D6 (PWM) | Servo dreapta — semnal |
| Pin 0 (RX) | ESP32 GPIO17 |
| 5V | VCC toate servo-urile |
| GND | GND comun |
| Vin | Baterie 9V (+) |

### Conectare la dashboard
1. Pe telefon sau laptop, deschide **Setări WiFi**
2. Conectează-te la rețeaua **`Byonic`** (parolă: `byonic1234`)
3. Deschide browserul și accesează **`http://192.168.4.1/`**

### Controlul robotului
| Buton / Tastă | Acțiune |
|---|---|
| Forward / ↑ | Înainte |
| Back / ↓ | Înapoi |
| Left / ← | Viraj stânga (pivot) |
| Right / → | Viraj dreapta (pivot) |
| Stop / Spațiu | Oprire imediată |
| Auto mode | Deplasare autonomă cu evitare obstacole |

> În modul **Auto**, robotul se oprește automat când detectează un obstacol la mai puțin de 15 cm.

---

## 📊 Date colectate

| Senzor | Măsoară | Interval |
|---|---|---|
| DHT11 | Temperatură (°C) | 0–50°C ±2°C |
| DHT11 | Umiditate relativă (%) | 20–90% ±5% |
| MQ-2 | Calitate aer / gaz (ADC) | 0–4095 (raw) |
| Senzor sol | Umiditate sol (ADC) | 0–4095 (raw) |
| HC-SR04 | Distanță obstacol (cm) | 2–400 cm |

Dashboard-ul afișează valorile în timp real și păstrează un istoric grafic al ultimelor 2 minute pentru temperatură, umiditate și calitatea aerului.

---


## 🖨️ Șasiu 3D

Șasiul este imprimat pe **Bambu Lab P1S** din PLA, 0.20mm strat, 20% infill.

## ⚙️ Specificații tehnice

| Parametru | Valoare |
|---|---|
| Microcontroler principal | ESP32 (Xtensa LX6, 240 MHz, WiFi 2.4GHz) |
| Microcontroler auxiliar | Arduino Uno (ATmega328P, 16 MHz) |
| Comunicație inter-board | UART Serial, 9600 baud |
| Protocol rețea | WiFi 802.11 b/g/n, Access Point mode |
| Adresă dashboard | http://192.168.4.1/ |
| Frecvență citire senzori | 1 citire la 2 secunde |
| Istoric date | Ultimele 120 citiri (~4 minute) |
| Alimentare | 9V baterie (Arduino) + USB 5V (ESP32) |
| Sistem de propulsie | Differential drive, 2× servo continuu SG90 |
| Viteză maximă | ~0.3 m/s (reglabilă din cod) |
| Distanță oprire obstacol | 15 cm (reglabilă din cod) |
EOF
