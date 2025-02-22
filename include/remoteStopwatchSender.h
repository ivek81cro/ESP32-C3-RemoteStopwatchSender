#ifndef REMOTESTOPWATCHSENDER_H
#define REMOTESTOPWATCHSENDER_H

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

// Structure definition for data packet
typedef struct {
  uint8_t seconds;
  int elapsedTime;
} DataPacket;

// Class definition for RemoteStopwatchSender
class RemoteStopwatchSender {
public:
  RemoteStopwatchSender(); // Constructor
  void setup(); // Setup method
  void loop(); // Loop method
  void getTime(unsigned long time, uint8_t* timeState); // Get time in formatted state
  void postRecievedDataOnLCD(int elapsedSeconds, String message); // Post received data on LCD
  void updateLCDMessage(String message, int time = -1); // Update LCD message
  bool isButtonPressed(uint8_t pin); // Check if button is pressed
  uint8_t getButtonPressed(); // Get which button is pressed
  void manageBacklight(); // Manage LCD backlight
  void onReceive(const uint8_t *mac, const uint8_t *incomingData, int len); // Callback for receiving data
  void onSent(const uint8_t *macAddr, esp_now_send_status_t status); // Callback for sending data
  bool setupEspNow(); // Setup ESP-NOW

  static void onReceiveStatic(const uint8_t *mac, const uint8_t *incomingData, int len); // Static callback for receiving data
  static void onSentStatic(const uint8_t *macAddr, esp_now_send_status_t status); // Static callback for sending data

private:
  static RemoteStopwatchSender* instance; // Singleton instance
  DataPacket sendData; // Data to send
  DataPacket receivedData; // Data received
  uint8_t receiverMAC[6] = {0xEC, 0xDA, 0x3B, 0xBF, 0x6E, 0x6C}; // Receiver's MAC address
  LiquidCrystal_PCF8574 lcd = LiquidCrystal_PCF8574(0x27); // LCD instance
  int recievedTimeTemp = 0; // Temporary storage for received time
};

#endif // REMOTESTOPWATCHSENDER_H
