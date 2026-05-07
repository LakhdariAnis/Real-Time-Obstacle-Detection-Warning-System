# freertos-obstacle-detection

Real-time obstacle detection on Arduino Mega 2560. Four FreeRTOS tasks,
ultrasonic ranging, software PWM servo control, LCD status, and a buzzer alarm.
Built and simulated in SimulIDE.

---

## What it does

An HC-SR04 measures distance every 150ms. If anything comes within 200cm,
the servo stops, the buzzer pulses, and the LCD screams at you.
Everything runs as independent preemptive tasks under FreeRTOS.

---

## Hardware

- Arduino Mega 2560 (ATmega2560, 8KB SRAM — the Uno dies instantly)
- HC-SR04 ultrasonic sensor
- SG90 servo motor
- HD44780 LCD 16x2 (4-bit parallel)
- Active buzzer 5V

## Software

- C++ / AVR-GCC
- FreeRTOS (Arduino port)
- LiquidCrystal.h
- SimulIDE for simulation

---

## Task architecture

| Task        | Priority | Period  | Job                          |
|-------------|----------|---------|------------------------------|
| TaskSound   | 3        | ~800ms  | Buzzer pulse on threat       |
| TaskDisplay | 2        | 400ms   | LCD update                   |
| TaskSensor  | 1        | 150ms   | HC-SR04 ranging              |
| TaskMotor   | 1        | 20ms    | Servo sweep / lock           |

Shared state: `volatile int distance` and `volatile bool motorLocked`.
Written by TaskSensor, read by everyone else.
No mutex — bool writes are atomic on AVR.

---

## Why Mega and not Uno

The Uno has 2KB SRAM. Four FreeRTOS task stacks plus the heap killed it
immediately. The Mega's 8KB gives enough room to breathe.
Stack sizes were tuned manually: Sound 100w, Sensor 128w, Motor 128w, Display 192w.
All LCD strings go through `F()` to stay in Flash and off the SRAM budget.

## The Servo.h problem

`Servo.h` and `Arduino_FreeRTOS.h` both want Timer 1. Conflict.
Solution: throw out `Servo.h` and bit-bang the PWM manually.

```cpp
int pulseWidth = 500 + (angle * 10.5); // 0-180deg mapped to 500-2390us
digitalWrite(SERVO_PIN, HIGH);
delayMicroseconds(pulseWidth);
digitalWrite(SERVO_PIN, LOW);
// vTaskDelay(20ms) yields between pulses
```

Standard RC servo period is 20ms. This hits it.

---

## Simulation (SimulIDE)

1. Install SimulIDE
2. Open `simulation/project.sim1`
3. The circuit loads with the Mega, sensor, servo, LCD, and buzzer wired up
4. Start simulation
5. Adjust the voltage knob to simulate distance — low voltage = close obstacle

No hardware needed to test.

---

## Build & flash (if you have real hardware)

Arch Linux setup:

```bash
# PlatformIO
pip install --user platformio

# Add yourself to uucp for serial access
sudo usermod -aG uucp $USER && newgrp uucp

# Build
pio init --board megaatmega2560
pio run --target upload --upload-port /dev/ttyUSB0
```

Or just open `src/main.cpp` in Arduino IDE, select Mega 2560, upload.

---

## Repo structure

```
.
├── src/
│   └── main.cpp
├── simulation/
│   └── project.sim1
├── docs/
│   └── report.pdf
└── README.md
```

`.gitignore` should cover: `.pio/`, `build/`, `*.hex`, `*.elf`, `.vscode/`

---

Lakhdari Anis Charef Eddine — ESI Sidi Bel Abbes, 2026