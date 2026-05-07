#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <LiquidCrystal.h>

// --- Prototypes ---
void TaskSensor(void *pvParameters);
void TaskMotor(void *pvParameters);
void TaskDisplay(void *pvParameters);
void TaskSound(void *pvParameters);

// --- Pins ---
#define TRIG_PIN 2
#define ECHO_PIN 3
#define SERVO_PIN 9
#define BUZZER_PIN 10

// --- Shared Variables ---
volatile int distance = 100;
volatile bool motorLocked = false;

// --- liquidCristal Setup ---
LiquidCrystal lcd(12, 11, 7, 6, 5, 4);

void setup() {
  Serial.begin(9600);
  while(!Serial); 
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SERVO_PIN, OUTPUT); 
  
  lcd.begin(16, 2);
  lcd.print(F("System 200cm..."));
  
  xTaskCreate(TaskSensor,  "Sens", 128, NULL, 1, NULL);
  xTaskCreate(TaskMotor,   "Motr", 128, NULL, 1, NULL);
  xTaskCreate(TaskDisplay, "Disp", 192, NULL, 2, NULL); 
  xTaskCreate(TaskSound,   "Snd",  100, NULL, 3, NULL);

  vTaskStartScheduler();
}

void loop() {}

// --- TASK 1: SENSOR ---
void TaskSensor(void *pvParameters) {
  for(;;) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    unsigned long dur = pulseIn(ECHO_PIN, HIGH, 30000);
    if(dur > 0) {
      distance = (dur * 0.034) / 2;
    } else {
      distance = 400;
    }
    
    motorLocked = (distance < 200);
    
    vTaskDelay(pdMS_TO_TICKS(150));
  }
}

// --- TASK 2: MOTOR ---
void TaskMotor(void *pvParameters) {
  int angle = 0;
  int step = 5;
  for(;;) {
    if(!motorLocked) {
      angle += step;
      if(angle >= 180 || angle <= 0) step = -step;
      
      int pulseWidth = 500 + (angle * 10.5); 
      digitalWrite(SERVO_PIN, HIGH);
      delayMicroseconds(pulseWidth);
      digitalWrite(SERVO_PIN, LOW);
    }
    vTaskDelay(pdMS_TO_TICKS(20)); 
  }
}

// --- TASK 3: DISPLAY ---
void TaskDisplay(void *pvParameters) {
  for(;;) {
    lcd.setCursor(0, 0);
    lcd.print(F("Dist: "));
    lcd.print(distance);
    lcd.print(F(" cm   "));
    
    lcd.setCursor(0, 1);
    if(motorLocked) {
      lcd.print(F("!! TOO CLOSE !! "));
    } else {
      lcd.print(F("SAFE - MOVING   "));
    }
    vTaskDelay(pdMS_TO_TICKS(400));
  }
}

// --- TASK 4: SOUND  ---
void TaskSound(void *pvParameters) {
  for(;;) {
    if(motorLocked) {
      digitalWrite(BUZZER_PIN, HIGH);
      vTaskDelay(pdMS_TO_TICKS(50));   
      digitalWrite(BUZZER_PIN, LOW);
      vTaskDelay(pdMS_TO_TICKS(750)); 
    } else {
      digitalWrite(BUZZER_PIN, LOW);
      vTaskDelay(pdMS_TO_TICKS(500));
    }
  }
}