#include "main.h"

// Global variable definitions
DataPacket sendData;
DataPacket receivedData;
uint8_t receiverMAC[] = {0xEC, 0xDA, 0x3B, 0xBF, 0x6E, 0x6C}; // Replace with receiver's MAC address
LiquidCrystal_PCF8574 lcd(0x27);

int recievedTimeTemp = 0;

String sendStatus;

// Convert elapsed time in miliseconds to mm:ss:ml
void getTime(unsigned long time, uint8_t* timeState) 
{
  timeState[0] = (time / 60000) % 60;
  timeState[1] = (time / 1000) % 60;
  timeState[2] = (time % 1000) / 10;
}

void postRecievedDataOnLCD(int elapsedSeconds, String message)
{
  recievedTimeTemp = elapsedSeconds;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
  lcd.setCursor(0, 1);
  uint8_t timeArray[3];
  getTime(recievedTimeTemp, timeArray);
  lcd.printf("%02d:%02d:%02d", timeArray[0], timeArray[1], timeArray[2]);
}

void updateLCDMessage(String message, int time = -1) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
  if (time != -1) 
  {
    lcd.setCursor(0, 1);
    uint8_t timeArray[3];
    getTime(recievedTimeTemp, timeArray);
    lcd.printf("%02d:%02d:%02d", timeArray[0], timeArray[1], timeArray[2]);
  }
  delay(2000);
}

bool isButtonPressed(uint8_t pin) 
{
  static unsigned long lastPressTime = 0;
  unsigned long currentTime = millis();
  if (digitalRead(pin) == HIGH && (currentTime - lastPressTime) > 200) {
    lastPressTime = currentTime;
    return true;
  }
  return false;
}


uint8_t getButtonPressed()
{
  if (isButtonPressed(BUTTON_PLUS1))
    return 1;
  if (isButtonPressed(BUTTON_PLUS3))
    return 2;
  if (isButtonPressed(BUTTON_RESET))
    return 3;
  if (isButtonPressed(BUTTON_REVERT))
    return 4;
  if (isButtonPressed(BUTTON_SEND))
    return 5;
  return 0;
}

void onReceive(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  DEBUG_PRINT("Received from Device B: ");
  if (receivedData.seconds == 5)
  {
    DEBUG_PRINTLN("Timer odbrojava, blokirano slanje.");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Timer odbrojava");
    lcd.setCursor(0, 1);
    lcd.print("Prijem blokiran");
  }
  else
  {
    DEBUG_PRINTLN(receivedData.elapsedTime);
    postRecievedDataOnLCD(receivedData.elapsedTime, "Vrijeme primljeno");
    sendData.seconds = 0;
  }
}

void onSent(const uint8_t *macAddr, esp_now_send_status_t status)
{
  DEBUG_PRINT("Delivery Status: ");
  DEBUG_PRINTLN(status == ESP_NOW_SEND_SUCCESS ? "Uspjesno poslano" : "Nije poslano");
  lcd.print(status == ESP_NOW_SEND_SUCCESS ? "Uspjesno poslano" : "Nije poslano");
}

bool setupEspNow() {
  if (esp_now_init() != ESP_OK) {
    DEBUG_PRINTLN("Error initializing ESP-NOW");
    return false;
  }
  
  esp_now_register_recv_cb(onReceive);
  esp_now_register_send_cb(onSent);
  
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

void setup()
{

  Wire.begin(I2C_SDA, I2C_SCL);

  lcd.begin(LCD_WIDTH, LCD_HEIGHT);
  lcd.setBacklight(255);

  Serial.begin(115200);
  delay(2000);

  pinMode(BUTTON_PLUS1, INPUT);
  pinMode(BUTTON_PLUS3, INPUT);
  pinMode(BUTTON_RESET, INPUT);
  pinMode(BUTTON_REVERT, INPUT);
  pinMode(BUTTON_SEND, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  WiFi.mode(WIFI_STA);

  String macAddress = WiFi.macAddress();
  DEBUG_PRINTLN("STA MAC Address: " + macAddress);

  if (!setupEspNow()) 
  {
    while (true); // Halt if ESP-NOW fails
  }

}

void loop()
{
  uint8_t numberOfSeconds = 0;
  uint8_t transmit = 0;

  uint8_t buttonPressed = getButtonPressed();
  switch (buttonPressed)
  {
  case 1: // Add 1 second
    recievedTimeTemp += 1000;
    updateLCDMessage("Dodano +1 sec", recievedTimeTemp);
    break;
  case 2: // Add 3 seconds
    recievedTimeTemp += 3000;
    updateLCDMessage("Dodano +3 sec", recievedTimeTemp);
    break;
  case 3: // Reset
    recievedTimeTemp = receivedData.elapsedTime;
    updateLCDMessage("Vrijeme vraceno", recievedTimeTemp);
    break;
  case 4: // Revert
    recievedTimeTemp = 0;
    updateLCDMessage("Vrijeme ponisteno", recievedTimeTemp);
    sendData.seconds = 6;
    break;
  case 5: // Send
    sendData.elapsedTime = recievedTimeTemp;
    esp_now_send(receiverMAC, (uint8_t *)&sendData, sizeof(sendData));
    updateLCDMessage("Vrijeme potvdjeno", recievedTimeTemp);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    break;
  }
}
