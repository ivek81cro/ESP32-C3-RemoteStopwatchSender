#ifndef MAIN_H
#define MAIN_H

#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

// Debugging macros
#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

// Pin definitions
#define BUTTON_PLUS1 4
#define BUTTON_PLUS3 3
#define BUTTON_REVERT 2
#define BUTTON_SEND 6
#define BUZZER_PIN 7
#define BUTTON_RESET 5
#define I2C_SDA 9
#define I2C_SCL 8
#define LCD_WIDTH 16
#define LCD_HEIGHT 2

// Structure definition
typedef struct {
  uint8_t seconds;
  int elapsedTime;
} DataPacket;

// Function declarations
void getTime(unsigned long time, uint8_t* timeState);
void postRecievedDataOnLCD(int elapsedSeconds, String message);
void updateLCDMessage(String message, int time);
bool isButtonPressed(uint8_t pin);
uint8_t getButtonPressed();
void manageBacklight();
void onReceive(const uint8_t *mac, const uint8_t *incomingData, int len);
void onSent(const uint8_t *macAddr, esp_now_send_status_t status);
bool setupEspNow();

// External variable declarations
extern DataPacket sendData;
extern DataPacket receivedData;
extern uint8_t receiverMAC[];
extern LiquidCrystal_PCF8574 lcd;
extern int recievedTimeTemp;

#endif // YOUR_FILE_H
