#ifndef REMOTESTOPWATCHSENDER_H
#define REMOTESTOPWATCHSENDER_H

#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <HardwareSerial.h>

// Debugging macros
#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

// Pin definitions  
#define BUTTON_PLUS1 4
#define BUTTON_PLUS3 3
#define BUTTON_REVERT 1 
#define BUTTON_SEND 0   
#define BUZZER_PIN 5    
#define BUTTON_RESET 2  
#define I2C_SDA 9
#define I2C_SCL 8

// LCD dimensions
#define LCD_WIDTH 16
#define LCD_HEIGHT 2

// Data Structures
/*Codes:    5 = recieve blocked, stopwatch running
            9 = stopwatch started, start timestamp
            8 = stopwatch stopped, stop timestamp
            6 = trigger reset
*/
typedef struct {
    uint8_t id; //Stopwatch Id
    uint8_t code; //code for messages between MCU's
    int stopTime; //elapsed time in milliseconds
    int startTime; //start timestamp
    int elapsedTime; //elapsed time in milliseconds
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
  void readIncommingMessage();
  void onSent(const uint8_t *macAddr, esp_now_send_status_t status); // Callback for sending data
  bool setupEspNow(); // Setup ESP-NOW

  static void onReceiveStatic(const uint8_t *mac, const uint8_t *incomingData, int len); // Static callback for receiving data
  static void onSentStatic(const uint8_t *macAddr, esp_now_send_status_t status); // Static callback for sending data

private:
  static RemoteStopwatchSender* instance; // Singleton instance
  DataPacket sendData; // Data to send
  DataPacket receivedData; // Data received
  uint8_t receiverMAC[6] = {0xEC, 0xDA, 0x3B, 0xC0, 0x18, 0x9C}; // Receiver's MAC address
  //{0x7C, 0x2C, 0x67, 0xD2, 0x26, 0x54} 
  //{0xEC, 0xDA, 0x3B, 0xBF, 0x6E, 0x6C}
  //{0xEC, 0xDA, 0x3B, 0xC0, 0x18, 0x9C}
  //{0x08, 0xB6, 0x1F, 0x37, 0xE5, 0x0C}-IR
  //{0xD8, 0xBC, 0x38, 0xF9, 0x64, 0x74}-IR
  LiquidCrystal_PCF8574 lcd = LiquidCrystal_PCF8574(0x27); // LCD instance
  int recievedTimeTemp = 0; // Temporary storage for received time
};

#endif // REMOTESTOPWATCHSENDER_H
