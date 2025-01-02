#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

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

// Structure to hold the data to send
// Structure to receive data
typedef struct {
  uint8_t seconds;
  int elapsedTime;
} DataPacket;

// MAC address of the Receiver ESP32
uint8_t receiverMAC[] = {0xEC, 0xDA, 0x3B, 0xBF, 0x6E, 0x6C}; // Replace with receiver's MAC address

DataPacket sendData;
DataPacket receivedData;

int recievedTimeTemp = 0;

String sendStatus;

LiquidCrystal_PCF8574 lcd(0x27);

uint8_t* getTime(unsigned long time){
  uint8_t* timeState = (uint8_t*)malloc(3 * sizeof(uint8_t)); 
  timeState[0] = (time / 60000) % 60;
  timeState[1] = (time / 1000) % 60;
  timeState[2] = (time % 1000) / 10;

  return timeState;
}

void postRecievedDataOnLCD(int elapsedSeconds){
  recievedTimeTemp = elapsedSeconds;
  lcd.clear();
  lcd.setCursor(0,1);
  uint8_t* time = getTime(recievedTimeTemp);
  lcd.printf("%d:%d:%d",time[0], time[1], time[2]);
  free(time);
}

void onReceive(const uint8_t *mac, const uint8_t *incomingData, int len) {
    memcpy(&receivedData, incomingData, sizeof(receivedData));
    Serial.print("Received from Device B: ");
    Serial.print(receivedData.elapsedTime);    
    Serial.println();
    postRecievedDataOnLCD(receivedData.elapsedTime);
}

void onSent(const uint8_t *macAddr, esp_now_send_status_t status) {
    Serial.print("Delivery Status: ");
    sendStatus = ESP_NOW_SEND_SUCCESS ? "Success" : "Fail";
    Serial.println(sendStatus);
    lcd.setCursor(0,1);
    lcd.print(sendStatus);
}

void setup() {

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
    Serial.println("STA MAC Address: " + macAddress);

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_recv_cb(onReceive);
    esp_now_register_send_cb(onSent);

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, receiverMAC, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }
}

void loop() {
  uint8_t numberOfSeconds = 0;
  uint8_t transmit = 0;

  uint8_t button1 = digitalRead(BUTTON_PLUS1);
  uint8_t button2 = digitalRead(BUTTON_PLUS3);
  uint8_t button3 = digitalRead(BUTTON_RESET);
  uint8_t button4 = digitalRead(BUTTON_REVERT);
  uint8_t button5 = digitalRead(BUTTON_SEND);

  if (button1 == HIGH){
    recievedTimeTemp += 1000;
    postRecievedDataOnLCD(recievedTimeTemp);
    Serial.printf("Time added:%d sec \n",1);
    delay(2000);
  }
  else if (button2 == HIGH)
  {
    recievedTimeTemp += 3000;    
    postRecievedDataOnLCD(recievedTimeTemp);
    Serial.printf("Time added:%d sec \n",3);
    delay(2000);
  }
  else if (button3 == HIGH)
  {
    recievedTimeTemp = receivedData.elapsedTime;//RESET
    postRecievedDataOnLCD(recievedTimeTemp);
    delay(2000);
    Serial.printf("Time reseted\n");
  }
  else if (button4 == HIGH)
  {
    recievedTimeTemp = 0;//REVERT
    postRecievedDataOnLCD(recievedTimeTemp);
    delay(2000);
    Serial.printf("Time reverted\n");
  }
  else if (button5 == HIGH)
  {
    transmit = 1;
    //recievedTimeTemp = 0;//RESET COUNTER
    postRecievedDataOnLCD(recievedTimeTemp);    
    Serial.printf("Time confirmed \n");
    delay(2000);
  }


if(transmit){  
    lcd.clear();
  // Prepare message
  sendData.elapsedTime = recievedTimeTemp;  

    // Send message
    esp_now_send(receiverMAC, (uint8_t *)&sendData, sizeof(sendData));
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(1800);
  }
}
