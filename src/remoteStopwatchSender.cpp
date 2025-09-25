#include "remoteStopwatchSender.h"

RemoteStopwatchSender* RemoteStopwatchSender::instance = nullptr;

// Constructor definition
RemoteStopwatchSender::RemoteStopwatchSender() {
  instance = this;
  // Initialization code if needed
}

void RemoteStopwatchSender::getTime(unsigned long time, uint8_t* timeState) 
{   
    // Convert time to minutes, seconds, and hundredths of a second
    timeState[0] = (time / 60000) % 60;
    timeState[1] = (time / 1000) % 60;
    timeState[2] = (time % 1000) / 10;
}

void RemoteStopwatchSender::updateLCDMessage(String message1, String message2, int time) {
    if(message1 != "")
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      // Pad message1 to 16 characters
      while(message1.length() < 16) {
        message1 += " ";
      }
      lcd.print(message1);
    }
    if (message2 != "")
    {
      lcd.setCursor(0, 1);
      // Pad message2 to 16 characters
      while(message2.length() < 16) {
        message2 += " ";
      }
      lcd.print(message2);
    }
    if (time != -1 && message2 == "") 
    {
      lcd.setCursor(0, 1);
      uint8_t timeArray[3];
      getTime(recievedTimeTemp, timeArray);
      lcd.printf("%02d:%02d:%02d", timeArray[0], timeArray[1], timeArray[2]);
    }
}

bool RemoteStopwatchSender::isButtonPressed(uint8_t pin) 
{
    static unsigned long lastPressTime = 0;
    unsigned long currentTime = millis();
    if (digitalRead(pin) == HIGH && (currentTime - lastPressTime) > 1000) {
      lastPressTime = currentTime;
      return true;
    }
    return false;
}

uint8_t RemoteStopwatchSender::getButtonPressed()
{
    if (isButtonPressed(BUTTON_PLUS1)){
      if (digitalRead(BUTTON_PLUS3)){
        DEBUG_PRINTLN("Disqualified");
        return 6; // Both buttons pressed
      }
      else{
        DEBUG_PRINTLN("Added 1 second");
        return 1; // Button 1 pressed
      }
    }
    if (isButtonPressed(BUTTON_PLUS3)){
      if (digitalRead(BUTTON_PLUS1)){
        DEBUG_PRINTLN("Disqualified");
        return 6; // Both buttons pressed
      }
      else{
        DEBUG_PRINTLN("Trigger toggled");
        return 2; // Button 2 pressed
      }
    }
    if (isButtonPressed(BUTTON_REVERT)){
      DEBUG_PRINTLN("Revert pressed");
      return 3;}
    if (isButtonPressed(BUTTON_RESET)){
      DEBUG_PRINTLN("Reset pressed");
      return 4;}
    if (isButtonPressed(BUTTON_SEND))
      return 5;

  return 0;
}

void RemoteStopwatchSender::onReceive(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    memcpy(&receivedData, incomingData, sizeof(receivedData));
    DEBUG_PRINT("Received from Device B: ");
    readIncommingMessage();
}

void RemoteStopwatchSender::readIncommingMessage()
{
  if (receivedData.code == 5)
  {
    DEBUG_PRINTLN("Timer odbrojava, blokirano slanje.");
    updateLCDMessage("Timer odbrojava", "Blokirano slanje");
  }
  else if (receivedData.code == 9)
  {
    DEBUG_PRINTLN("Timer pokrenut.");
    updateLCDMessage("Vrijeme krenulo", "Senzor OFF");
    Serial1.println("Start " + String(receivedData.startTime));
  }
  else if (receivedData.code == 8)
  {
    recievedTimeTemp = receivedData.elapsedTime;
    DEBUG_PRINTLN(recievedTimeTemp);
    updateLCDMessage("Vrijeme primljeno", "", recievedTimeTemp);
    sendData.code = 0;
    Serial1.println("Stop " + String(receivedData.stopTime));
    Serial1.println("Elapsed " + String(receivedData.elapsedTime));
  }
  else if (receivedData.code == 3)
  {
    DEBUG_PRINTLN("Trigger armed.");
    updateLCDMessage("Trigger armed", "Senzor ON");
  }
  else if (receivedData.code == 6)
  {
    DEBUG_PRINTLN("Timer resetiran.");
    updateLCDMessage("Timer resetiran", "Vrijeme 0");
  }
  else if (receivedData.code == 7)
  {
    DEBUG_PRINTLN("Timer ponisten.");
    updateLCDMessage("Timer ponisten", "Vrijeme 0");
  }
  else if (receivedData.code == 20)
  {
    DEBUG_PRINTLN("Trigger armed");
    updateLCDMessage("", "Senzor ON");
  }
  else if (receivedData.code == 21)
  {
    DEBUG_PRINTLN("Trigger disarmed");
    updateLCDMessage("", "Senzor OFF");
  }
  else
  {
    DEBUG_PRINTLN("Unknown code received. " + String(receivedData.code));
    updateLCDMessage("Nepoznat kod", String(receivedData.code));
  }
}

void RemoteStopwatchSender::onSent(const uint8_t *macAddr, esp_now_send_status_t status)
{
  DEBUG_PRINTF("Code set to: %d\n", sendData.code);
  DEBUG_PRINT("Delivery Status: ");
  DEBUG_PRINTLN(status);
}

void RemoteStopwatchSender::onReceiveStatic(const uint8_t *mac, const uint8_t *incomingData, int len) {
    if (instance != nullptr) {
        instance->onReceive(mac, incomingData, len);
    }
}

void RemoteStopwatchSender::onSentStatic(const uint8_t *macAddr, esp_now_send_status_t status) {
    if (instance != nullptr) {
            instance->onSent(macAddr, status);
    }
}

bool RemoteStopwatchSender::setupEspNow() {
  if (esp_now_init() != ESP_OK) {
    DEBUG_PRINTLN("Error initializing ESP-NOW");
    return false;
  }
  
  esp_now_register_recv_cb(RemoteStopwatchSender::onReceiveStatic);
  esp_now_register_send_cb(RemoteStopwatchSender::onSentStatic);
  
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    DEBUG_PRINTLN("Failed to add peer");
    return false;
  }
  return true;
}

void RemoteStopwatchSender::setup()
{
    Wire.begin(I2C_SDA, I2C_SCL);

    lcd.begin(LCD_WIDTH, LCD_HEIGHT);
    lcd.setBacklight(255);

    Serial.begin(115200);
    Serial1.begin(115200, SERIAL_8N1, 20, 21);

    delay(2000);

    pinMode(BUTTON_PLUS1, INPUT);
    pinMode(BUTTON_PLUS3, INPUT);
    pinMode(BUTTON_RESET, INPUT);
    pinMode(BUTTON_REVERT, INPUT);
    pinMode(BUTTON_SEND, INPUT);
    
    //Aktivni buzzer
    //pinMode(BUZZER_PIN, OUTPUT);

    //Pasivni buzzer
    ledcAttachPin(BUZZER_PIN, 0);
    ledcSetup(0, 2000, 8); // Channel 0, 2kHz frequency, 8-bit resolution

    WiFi.mode(WIFI_STA);
    String macAddress = WiFi.macAddress();
    updateLCDMessage("Podizanje sustava.......");
    delay(5000);
    DEBUG_PRINTLN("STA MAC Address: " + macAddress);
    updateLCDMessage("Spreman");

    if (!setupEspNow()) 
    {
      while (true); // Halt if ESP-NOW fails
    }
}

void RemoteStopwatchSender::loop()
{
  uint8_t numberOfSeconds = 0;
  uint8_t transmit = 0;
  uint8_t buttonPressed = getButtonPressed();

  switch (buttonPressed)
  {
  case 1: // Add 1 second
    recievedTimeTemp += 1000;
    updateLCDMessage("Dodano +1 sec", "", recievedTimeTemp);
    Serial1.println("P 1");
    break;
  case 2:
    {
      sendData.code = 10; // Add 3 seconds
      esp_err_t esp_message = esp_now_send(receiverMAC, (uint8_t *)&sendData, sizeof(sendData));
      // Add 3 seconds
      //recievedTimeTemp += 3000;
      //updateLCDMessage("Dodano +3 sec", recievedTimeTemp);
      //Serial1.println("P 3");
    }
    break;
  case 3: // Revert
    recievedTimeTemp = receivedData.elapsedTime;
    updateLCDMessage("Vrijeme vraceno", "", recievedTimeTemp);
    Serial1.println("P 0");
    break;
  case 4: // Reset
    recievedTimeTemp = 0;
    updateLCDMessage("Vrijeme ponisteno", "", recievedTimeTemp);
    break;
  case 5: // Send
    {
      if(recievedTimeTemp == 0)
      { 
        sendData.code = 3;//reset display and arm trigger
      }
      else
      {
        sendData.code = 5;
      }    
      sendData.elapsedTime = recievedTimeTemp;
      esp_err_t esp_message = esp_now_send(receiverMAC, (uint8_t *)&sendData, sizeof(sendData));
      if(esp_message == ESP_OK)
      {
        if(recievedTimeTemp == 0)
        {
          updateLCDMessage("Vrijeme spremno", "Senzor ON");
        }
        else
        {
          updateLCDMessage("Vrijeme potvdjeno", "", recievedTimeTemp);
        }
        DEBUG_PRINTLN("Message sent successfully: " + String(esp_message));
        Serial1.println("Confirmed " + String(recievedTimeTemp));
      }
      else
      {
        updateLCDMessage("Greska ponovi");
        DEBUG_PRINTLN("Error sending message: " + String(esp_message));
      }
      //Aktivni buzzer
      //digitalWrite(BUZZER_PIN, HIGH);
      //delay(200);
      //digitalWrite(BUZZER_PIN, LOW);

      //Pasivni buzzer
      ledcWrite(0, 255); // Start the buzzer at half volume
      delay(1000); 
      ledcWrite(0, 0); // Stop the buzzer
    }
    break;
  case 6: // Both buttons pressed    
    recievedTimeTemp = receivedData.elapsedTime;
    sendData.code = 7;
    esp_now_send(receiverMAC, (uint8_t *)&sendData, sizeof(sendData));
    updateLCDMessage("Diskvalifikacija", "", recievedTimeTemp);
    Serial1.println("disq");
    delay(200);
    break;
  }
}