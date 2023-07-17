
#include <esp_now.h>
#include <WiFi.h>
#include <DHT.h>
#include <BH1750.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <esp_wifi.h>
#include <Adafruit_Sensor.h>
#include <Arduino_JSON.h>

BH1750 lightMeter;
#define PIN 33
#define mua 34
#define DHTPIN 2  
#define DHTTYPE    DHT11     // DHT 11
DHT dht(DHTPIN, DHTTYPE);

#define BOARD_ID 1

uint8_t broadcastAddress[] = {0x0C, 0xB8, 0x15, 0xF5, 0x1D, 0x58};


#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

typedef struct struct_message {
  int id;
  float temp;
  float hum;
  float lux;
  int rain;
  int doamdat;
  float pH;
  int readingId;
} struct_message;

struct_message myData;
struct_message board1;
struct_message board2;
struct_message board3;
struct_message boardsStruct[3] = {board1, board2, board3};
JSONVar board;

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

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  boardsStruct[myData.id - 1].temp = myData.temp;
  boardsStruct[myData.id - 1].hum = myData.hum;
  boardsStruct[myData.id - 1].lux = myData.lux;
  boardsStruct[myData.id - 1].doamdat = myData.doamdat;
  boardsStruct[myData.id - 1].pH = myData.pH;
  board["readingId"] = String(myData.readingId);
  String jsonString = JSON.stringify(board);
}

void setup() {
// Init Serial Monitor
  Serial.begin(115200);
  Wire.begin();
  dht.begin();
  lightMeter.begin();
  pinMode (PIN, INPUT);
  pinMode (mua, INPUT);

    // Init OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

 int32_t channel = getWiFiChannel(WIFI_SSID);

  WiFi.printDiag(Serial); // Uncomment to verify channel number before
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  WiFi.printDiag(Serial); // Uncomment to verify channel change after
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  
    // Register peer
// esp_now_peer_info_t peerInfo;
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
    
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  } 
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

delay(2000); 
}


void loop() {

    int doam = analogRead(PIN);
    int doam_dat = 100 - map(doam,0,4095,100,0);
    int docmua = analogRead(mua);
    int rain = map(docmua,0,4095,0,100);

  myData.id = BOARD_ID;
  myData.temp = readDHTTemperature();
  myData.hum = readDHTHumidity();
  myData.rain = rain;
  myData.lux = readBH1750LightLevel();
  myData.doamdat = doam_dat;

  Serial.print("doamdat: ");
  Serial.print( doam_dat);
  Serial.println(" %");
  Serial.print("Rain: ");
  Serial.print( rain);
  Serial.println(" %");

  // Send message via ESP-NOW

  esp_err_t result = esp_now_send(0, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  
  float temperature2 = boardsStruct[1].temp;
  float humidity2 = boardsStruct[1].hum;
  float light2 = boardsStruct[1].lux;
  float doamdat2 = boardsStruct[1].doamdat;
  float pH = boardsStruct[1].pH;
 
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("KHU 2");
  display.setCursor(0, 15);
  display.print("Temp: ");
  display.print(temperature2);
  display.cp437(true);
  display.write(248);
  display.print("C");
  display.setCursor(0, 25);
  display.print("Hum: ");
  display.print(humidity2);
  display.print("%");
  display.setCursor(0, 35);
  display.print("light: ");
  display.print(light2);
  display.print("%");
  display.setCursor(0,45);
  display.print("shum: ");
  display.print(doamdat2);
  display.print("%");
  display.display();
  display.clearDisplay();
  delay(4000);
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("KHU 1");
  display.setCursor(0, 15);
  display.print("Temp: ");
  display.print( myData.temp);
  display.cp437(true);
  display.write(248);
  display.print("C");
  display.setCursor(0, 25);
  display.print("Hum: ");
  display.print(myData.hum);
  display.print("%");
  display.setCursor(0, 35);
  display.print("light: ");
  display.print(myData.lux);
  display.print("%");
  display.setCursor(0,45);
  display.print("shum: ");
  display.print(myData.doamdat);
  display.print("%");
  display.display();
  display.clearDisplay();
  delay(4000);

}
