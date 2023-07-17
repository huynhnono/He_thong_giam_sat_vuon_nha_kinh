#include <esp_now.h>
#include <WiFi.h>
#include <DHT.h>
#include <BH1750.h>
#include <Adafruit_Sensor.h>
#include <esp_wifi.h>
#include <ESP32Servo.h>
BH1750 lightMeter;
#define PIN 33
#define DHTPIN 2  
#define DHTTYPE    DHT11     // DHT 11
DHT dht(DHTPIN, DHTTYPE);
#define PHSENSOR 34 
#define BOARD_ID 2
int irPin = 13;  // Khai báo chân kết nối cảm biến hồng ngoại
int threshold = HIGH;  // Giá trị cảm biến hồng ngoại cho phép mở cửa
Servo myservo;
float PHValue=0;
float Calibration_value = 21.34;

// REPLACE WITH THE RECEIVER'S MAC Address
uint8_t broadcastAddress1[] = {0x0C, 0xB8, 0x15, 0xF5, 0x1D, 0x58};
uint8_t broadcastAddress2[] = {0xB0, 0xB2, 0x1C, 0xA7, 0x34, 0xA8};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
    int id; // must be unique for each sender board
    float temp;
    float hum;
    float lux;
    int rain;
    int doamdat;
    float pH;
    int readingId;
} struct_message;

// Create a struct_message called myData
struct_message myData;

unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 10000;        // Interval at which to publish sensor readings

unsigned int readingId = 0;

// Insert your SSID
constexpr char WIFI_SSID[] = "REBOOT";

int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
      for (uint8_t i=0; i<n; i++) {
          if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
              return WiFi.channel(i);
          }
      }
  }
  return 0;
}

// Create peer interface
esp_now_peer_info_t peerInfo;

float readBH1750LightLevel() {
float lux = map(lightMeter.readLightLevel(),0,32767,0,100);
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" %");
} 
float readDHTTemperature() {
  float t = dht.readTemperature();
  Serial.print("Temperature: ");
  Serial.print( t);
  Serial.println(" ºC");
 
}

float readDHTHumidity() {
  float h = dht.readHumidity();
  Serial.print("Humidity: ");
  Serial.print( h);
  Serial.println(" %");
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  Serial.print("Packet to: ");
  // Copies the sender mac address to a string
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  Wire.begin();
  dht.begin();
  lightMeter.begin();
  myservo.attach(5); 
  pinMode(PHSENSOR,INPUT);
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  pinMode (PIN, INPUT);
  pinMode(irPin, INPUT);
 int32_t channel = getWiFiChannel(WIFI_SSID);

  WiFi.printDiag(Serial); // Uncomment to verify channel number before
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  WiFi.printDiag(Serial); // Uncomment to verify channel change after
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
// esp_now_peer_info_t peerInfo;
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
    
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  
  memcpy(peerInfo.peer_addr, broadcastAddress2, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
delay(2000);
}
 
void loop() {
int irValue = digitalRead(irPin);  // Đọc giá trị từ cảm biến hồng ngoại
if(irValue ==0){
  myservo.write(0);
   delay(1000);  // Chờ 1 giây
    Serial.println("value");
    Serial.print( irValue);
}
  else if(irValue ==1){
    myservo.write(90);
    delay(1000);  // Chờ 1 giây
      Serial.println("value");
      Serial.print( irValue);
  }
 unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;

 float PHVolt;
 unsigned long ADCValue;
 PHVolt = 0;
 ADCValue=0;
 for(int i=0; i<100; i++)
 {
   ADCValue += analogRead(PHSENSOR);
   delay(3);
  }

    ADCValue=ADCValue/100;
    PHVolt=(float)ADCValue*3.6/4095;
    PHVolt=PHVolt-0.46; //offset on physical
    PHValue = -5.70 * PHVolt + Calibration_value; 
    Serial.print("PH: ");
    Serial.println(PHValue);
    
    int doam = analogRead(PIN);
    int doam_dat = 100 - map(doam,0,4095,100,0);

  myData.id = BOARD_ID;
  myData.temp = readDHTTemperature();
  myData.hum = readDHTHumidity();
  myData.lux = readBH1750LightLevel();
  myData.doamdat = doam_dat;
  myData.pH =  PHValue;
  myData.readingId = readingId++;
  Serial.print("doamdat: ");
  Serial.print( doam_dat);
  Serial.println(" %");


  // Send message via ESP-NOW

  esp_err_t result = esp_now_send(0, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

}
delay(2000);
}
