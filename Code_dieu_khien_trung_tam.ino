#include <ESP32Servo.h>
//#include<Servo.h>
#include <FirebaseESP32.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Arduino_JSON.h>
#include<Wire.h>
#include<TimeLib.h>
#define FIREBASE_HOST "doan--iot-dut-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "QSsxQKXcWcn6F6zNP8f9Cg0GsHtvMQSK9vWctTE2"
#define WIFI_SSID "TT"
#define WIFI_PASSWORD "123456789"
// khai báo chân KHU_1:

#define AUTO_MODE_PIN_1 1
#define RELAY_FAN_1 21
#define RELAY_LIGHT_1 19
#define RELAY_PUMP_1 18
#define ledPin 4
#define BUTTON_LIGHT_1 27
#define BUTTON_FAN_1 14
#define BUTTON_PUMP_1 26
#define BUTTON_DC_1 15
#define MANUAL_AUTO_BUTTON_1 13
// KHAI BÁO CHÂN KHU_2:
#define AUTO_MODE_PIN_2 0
#define RELAY_PUMP_2 22
#define RELAY_FAN_2 17
#define RELAY_LIGHT_2 23
#define BUTTON_PUMP_2 32
#define BUTTON_LIGHT_2 33
#define BUTTON_FAN_2 25
#define MANUAL_AUTO_BUTTON_2 12

#define in1  16
#define in2  5
#define ctht1  34
#define ctht2  35
#define enA 2

// giá trị khởi tạo thời gian
const int ON_HOUR = 7;   // Giờ bật bơm
const int ON_MINUTE = 0; // Phút bật bơm
const int OFF_HOUR = 7;  // Giờ tắt bơm
const int OFF_MINUTE = 3;// Phút tắt bơm


float temp1Initial = 0;
float hum1Initial = 0;
float light1Initial = 0;
float doamdat1Initial = 0;
int rainInitial = 0;

float temp2Initial = 0;
float hum2Initial = 0;
float light2Initial = 0;
float doamdat2Initial = 0;
// GIÁ TRỊ KHỞI TẠO BAN ĐẦU NÚT NHẤN KHU_1:
int btnState_pump = 0;
int btnState_light = 0;
int btnState_fan = 0;
int btnState_auto = 0;
int btnState_dc = 0;
int automatic = 0;
// GIÁ TRỊ KHỞI TẠO BAN ĐẦU NÚT NHẤN KHU_2:
int btnState_pump_2 = 0;
int btnState_light_2 = 0;
int btnState_fan_2 = 0;
int btnState_auto_2 = 0;
int automatic_2 = 0;

FirebaseData firebaseData;
WiFiClient wifiClient;
typedef struct struct_message {
  int id;
  float temp;
  float hum;
  float lux;
  int rain;
  int doamdat;
  int readingId;
} struct_message;

struct_message myData;
struct_message board1;
struct_message board2;
struct_message board3;
struct_message boardsStruct[3] = {board1, board2, board3};
JSONVar board;
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
  boardsStruct[myData.id - 1].rain = myData.rain;
  boardsStruct[myData.id - 1].doamdat = myData.doamdat;
  board["readingId"] = String(myData.readingId);
  String jsonString = JSON.stringify(board);
}
void pushData(String key, float value) {
  Firebase.setFloat(firebaseData, key, value);
}

void pushDataBtn(String key, bool value) {
  Firebase.setBool(firebaseData, key, value);
}



void setup() {

  Serial.begin(115200);

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  // Khởi tạo kết nối Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  //KHU_1:
  pinMode(MANUAL_AUTO_BUTTON_1, INPUT_PULLUP);
  pinMode(AUTO_MODE_PIN_1, OUTPUT);
  pinMode(BUTTON_PUMP_1, INPUT_PULLUP);
  pinMode(BUTTON_LIGHT_1, INPUT_PULLUP);
  pinMode(BUTTON_FAN_1, INPUT_PULLUP);
  pinMode(BUTTON_DC_1, INPUT_PULLUP);
  pinMode(RELAY_PUMP_1, OUTPUT);
  pinMode(RELAY_FAN_1, OUTPUT);
  pinMode(RELAY_LIGHT_1, OUTPUT);
  pinMode(ledPin, OUTPUT);
  // KHU_2:
  pinMode(MANUAL_AUTO_BUTTON_2, INPUT_PULLUP);
  pinMode(BUTTON_PUMP_2, INPUT_PULLUP);
  pinMode(BUTTON_LIGHT_2, INPUT_PULLUP);
  pinMode(BUTTON_FAN_2, INPUT_PULLUP);
  pinMode(RELAY_PUMP_2, OUTPUT);
  pinMode(RELAY_LIGHT_2, OUTPUT);
  pinMode(RELAY_FAN_2, OUTPUT);
  pinMode(AUTO_MODE_PIN_2, OUTPUT);


  // ĐONG CO
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(ctht1, INPUT);
  pinMode(ctht2, INPUT);
  pinMode(enA, OUTPUT);

}

// HÀM PUSHDATA FUREBASE KHU_1:
void handleData_1(float temp1, float hum1, float light1, int doamdat1, int rain1 )
{
  if (temp1 != temp1Initial) {
    temp1Initial = temp1;
   
    pushData( "/KHU_1/temp", temp1Initial);
  }
  if (hum1 != hum1Initial) {
    hum1Initial = hum1;
    pushData( "/KHU_1/hum", hum1Initial);
  }
  if (light1 != light1Initial) {
    light1Initial = light1;
    pushData( "/KHU_1/light", light1Initial);
  }
  if (doamdat1 != doamdat1Initial) {
    doamdat1Initial = doamdat1;
    pushData( "/KHU_1/doamdat", doamdat1Initial);
  }
  if (rain1 != rainInitial) {
    rainInitial = rain1;
    
    pushData( "/KHU_1/rain", rainInitial);
  }
}
// HÀM PUSHDATA FUREBASE KHU_2:
void handleData_2(float temp2, float hum2, float light2, float doamdat2) {
  if (temp2 != temp2Initial) {
    temp2Initial = temp2;
    pushData( "/KHU_2/temp", temp2Initial);
  }
  if (hum2 != hum2Initial) {
    hum2Initial = hum2;
    pushData( "/KHU_2/hum", hum2Initial);
  }
  if (light2 != light2Initial) {
    light2Initial = light2;
    pushData( "/KHU_2/light", light2Initial);
  }
  if (doamdat2 != doamdat2Initial) {
    doamdat2Initial = doamdat2;
    pushData( "/KHU_2/doamdat", doamdat2Initial);
  }
}

void loop() {
//    time_t now = time(nullptr);
//    int currentHour = hour(now);
//    int currentMinute = minute(now);
//    if (currentHour == ON_HOUR && currentMinute >= ON_MINUTE && currentMinute < OFF_MINUTE) {
//      digitalWrite( RELAY_PUMP_1, HIGH); // Bật bơm
//    } else {
//      digitalWrite( RELAY_PUMP_1, LOW); // Tắt bơm
//    }

  // Chờ 1 giây trước khi kiểm tra lại thời gian
  //delay(1000);
  float temperature1 = boardsStruct[0].temp; 
  float humidity1 = boardsStruct[0].hum;
  float light1 = boardsStruct[0].lux;
  int rain1 = boardsStruct[0].rain;
  int doamdat1 = boardsStruct[0].doamdat;
  float temperature2 = boardsStruct[1].temp;
  float humidity2 = boardsStruct[1].hum;
  float light2 = boardsStruct[1].lux;
  float doamdat2 = boardsStruct[1].doamdat;Serial.println(doamdat2);
  handleData_1(temperature1, humidity1, light1, doamdat1, rain1);
  handleData_2(temperature2, humidity2, light2, doamdat2);


  //KHU_1:

  btnState_pump = digitalRead(BUTTON_PUMP_1);
  btnState_light = digitalRead(BUTTON_LIGHT_1);
  btnState_fan = digitalRead(BUTTON_FAN_1);
  btnState_auto = digitalRead(MANUAL_AUTO_BUTTON_1);
  btnState_dc = digitalRead(BUTTON_DC_1);

  //KHU_2:

  btnState_pump_2 = digitalRead(BUTTON_PUMP_2);
  btnState_light_2 = digitalRead(BUTTON_LIGHT_2);
  btnState_fan_2 = digitalRead(BUTTON_FAN_2);
  btnState_auto_2 = digitalRead(MANUAL_AUTO_BUTTON_2);
  // DC


  // ĐIỀU KHIỂN THỦ CÔNG KHU_1:
  Serial.println("btnState_pump");
  Serial.println(btnState_pump);
  if (automatic == 0) {
    if (btnState_pump == 1) {
      Serial.println("PUM");
      Serial.println(btnState_pump);
      int checkPump = digitalRead(RELAY_PUMP_1);
      if (checkPump == 1) {
        Serial.println("vo");
        controlPUMP(false);
        pushDataBtn("/KHU_1/control/pump", false);
      } else {
        Serial.println("0vo");
        controlPUMP(true);
        pushDataBtn("/KHU_1/control/pump", true);
      }
    }
    if (btnState_light == 1) {
      int checkLight = digitalRead(RELAY_LIGHT_1);
      if (checkLight == 1) {
        Serial.println("LIGHT23");
        controlLIGHT(false);
        pushDataBtn("/KHU_1/control/light", false);
      } else {
        Serial.println("LIGHT234");
        controlLIGHT(true);
        pushDataBtn("/KHU_1/control/light", true);
      }
    }
    if (btnState_fan == 1) {
      Serial.println("btnState_fan");
      int checkFan = digitalRead(RELAY_FAN_1);
      if (checkFan == 1) {
        Serial.println("checkFan");
        controlFAN(false);
        pushDataBtn("/KHU_1/control/fan", false);
      } else {
        Serial.println("checkFanElse");
        controlFAN(true);
        pushDataBtn("/KHU_1/control/fan", true);
      }
    }
    if ( btnState_dc  == 1) {
      int checkDC = digitalRead(ledPin);
      if (checkDC == 1) {
        stepMotor(true);
        pushDataBtn("/KHU_1/control/curtain", true);
      } else {
        stepMotor(false);
        pushDataBtn("/KHU_1/control/curtain", false);
      }
    }
  }

  if (automatic == 1) {
    handleAuto(temperature1, humidity1, light1,rain1, doamdat1);
  }


  // điều khiển thủ công KHU_2:
  // Serial.println("automatic_2");
  // Serial.println(automatic_2);
  if (automatic_2 == 0) {
    //Serial.println("AUTO2");   
    if (btnState_pump_2 == 1) {
      int checkPump = digitalRead(RELAY_PUMP_2);
      if (checkPump == 1) {
        controlPUMP_2(false);
        pushDataBtn("/KHU_2/control/pump", false);
      } else {
        controlPUMP_2(true);
        pushDataBtn("/KHU_2/control/pump", true);
      }
    }

    if (btnState_light_2 == 1) {
      //Serial.println("LIGHT2");
      //Serial.println(btnState_light_2);
      int checkLight = digitalRead(RELAY_LIGHT_2);
      if (checkLight == 1) {
        controlLIGHT_2(false);
        pushDataBtn("/KHU_2/control/light", false);
      } else {
        controlLIGHT_2(true);
        pushDataBtn("/KHU_2/control/light", true);
      }
    }

    if (btnState_fan_2 == 1) {
      int checkFan = digitalRead(RELAY_FAN_2);
      if (checkFan == 1) {
        controlFAN_2(false);
        pushDataBtn("/KHU_2/control/fan", false);
      } else {
        controlFAN_2(true);
        pushDataBtn("/KHU_2/control/fan", true);
      }
    }


  }
  if (automatic_2 == 1) {
    handleAuto_2(temperature2, humidity2, light2, doamdat2);
  }
  // điều khiển tự động KHU_1:

  if (btnState_auto == 1) {
    int checkAuto = digitalRead(AUTO_MODE_PIN_1);
    if (checkAuto == 1) {
      controlAUTO(false);
      automatic = 0;
      pushDataBtn("/KHU_1/control/auto", false);
    } else {
      controlAUTO(true);
      automatic = 1;
      pushDataBtn("/KHU_1/control/auto", true);
      //handleAuto(temperature1,humidity1,light1,doamdat1);

    }
  }

  Serial.println("AUTO=12");
  // điều khiển tự động KHU_2:
  if (btnState_auto_2 == 1) {
    int checkAuto = digitalRead(AUTO_MODE_PIN_2);
    if (checkAuto == 1) {
      controlAUTO_2(false);

      automatic_2 = 0;
      pushDataBtn("/KHU_2/control/auto", false);
    } else {
      controlAUTO_2(true);
      automatic_2 = 1;
      pushDataBtn("/KHU_2/control/auto", true);
      //handleAuto(temperature1,humidity1,light1,doamdat1);

    }
  }
  
  //dữ kiệu Khu 1;
  Serial.println("AUTO=1");
  btnAUTO ("/KHU_1/control/auto");
  if (automatic == 0) {
    //Serial.println("AUTO=0");
    btnFan("/KHU_1/control/fan");
    btnLight("/KHU_1/control/light");
    btnPump("/KHU_1/control/pump");
    btnDC("/KHU_1/control/curtain");
  }
  //dữ liệu firebase KHU_2:
  btnAuto_2 ("/KHU_2/control/auto");
  if (automatic_2 == 0) {
    btnFan_2("/KHU_2/control/fan");
    btnLight_2("/KHU_2/control/light");
    btnPump_2("/KHU_2/control/pump");

  }

}


// hàm điều điều RELAY:

void controlFAN(bool state) {
  if (state == 1) {
    digitalWrite(RELAY_FAN_1, HIGH);
  } else {
    digitalWrite(RELAY_FAN_1, LOW);
  }
}

void controlAUTO(bool state) {
  if (state == 1) {
    digitalWrite(AUTO_MODE_PIN_1, HIGH);
  } else {
    digitalWrite(AUTO_MODE_PIN_1, LOW);
  }
}
void controlLIGHT(bool state) {
  if (state == 1) {
    digitalWrite(RELAY_LIGHT_1, HIGH);
  } else {
    digitalWrite(RELAY_LIGHT_1, LOW);
  }
}
void controlPUMP(bool state) {
  if (state == 1) {
    digitalWrite(RELAY_PUMP_1, HIGH);
  } else {
    digitalWrite(RELAY_PUMP_1, LOW); // Turn
  }
}
void stepMotor(bool state) {

  int trangthaict1 = digitalRead(ctht1);
  int trangthaict2 = digitalRead(ctht2);
  if (state == 1) {
    digitalWrite(ledPin, HIGH);
    xuong();
    if (trangthaict1 == 1) {
      dung();
    }
  }
  else {
    digitalWrite(ledPin, LOW);
    len();
    if (trangthaict2 == 1) {
      dung();
    }
  }
}

void len() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(enA, 50);
}
void xuong() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(enA, 65);
}
void dung() {

  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  ;
}


void controlFAN_2(bool state) {

  if (state == 1) {
    digitalWrite(RELAY_FAN_2, HIGH);
  } else {
    digitalWrite(RELAY_FAN_2, LOW);
  }
}


void controlAUTO_2(bool state) {
  if (state == 1) {
    digitalWrite(AUTO_MODE_PIN_2, HIGH);
  } else {
    digitalWrite(AUTO_MODE_PIN_2, LOW);

  }
}
void controlLIGHT_2(bool state) {
  if (state == 1) {
    digitalWrite(RELAY_LIGHT_2, HIGH);
  } else {
    digitalWrite(RELAY_LIGHT_2, LOW);
  }
}
void controlPUMP_2(bool state) {
  if (state == 1) {
    digitalWrite(RELAY_PUMP_2, HIGH);

  } else {
    digitalWrite(RELAY_PUMP_2, LOW);

  }
}

// hàm lấy dữ liệu từ firebase:

void btnAUTO (String key) {
  if (Firebase.getBool(firebaseData, key)) {
    bool value = firebaseData.boolData();
    automatic = value;
    controlAUTO(value);
    Serial.println(value);
    //handleAuto();
  }
}

void btnLight(String key) {
  Serial.println(key);
  if (Firebase.getBool(firebaseData, key)) {
    bool value = firebaseData.boolData();
    Serial.println(value);
    controlLIGHT(value);
  }
}
void btnFan(String key) {
  Serial.println(key);
  if (Firebase.getBool(firebaseData, key)) {
    bool value = firebaseData.boolData();
    Serial.println(value);
    controlFAN(value);
  }
}
void btnPump(String key) {
  Serial.println(key);
  if (Firebase.getBool(firebaseData, key)) {
    bool value = firebaseData.boolData();
    Serial.println(value);
    controlPUMP(value);
  }
}
void btnDC(String key) {
  Serial.println(key);
  if (Firebase.getBool(firebaseData, key)) {
    bool value = firebaseData.boolData();
    Serial.println(value);
    stepMotor(value);
  }
}


void btnAuto_2(String key) {
  if (Firebase.getBool(firebaseData, key)) {
    bool value = firebaseData.boolData();
    automatic_2 = value;
    controlAUTO_2(value);
    Serial.println(value);
    //handleAuto_2();
  }
}

void btnLight_2(String key) {
  Serial.println(key);
  if (Firebase.getBool(firebaseData, key)) {
    bool value = firebaseData.boolData();
    Serial.println(value);
    controlLIGHT_2(value);
  }
}
void btnFan_2(String key) {
  Serial.println(key);
  if (Firebase.getBool(firebaseData, key)) {
    bool value = firebaseData.boolData();
    Serial.println(value);
    controlFAN_2(value);
  }
}
void btnPump_2(String key) {
  Serial.println(key);
  if (Firebase.getBool(firebaseData, key)) {
    bool value = firebaseData.boolData();
    Serial.println(value);
    controlPUMP_2(value);
  }
}


void handleAuto(float temp, float hum, float light,  int rain,int doam ) {
  if (doam > 65) {
    pushDataBtn("/KHU_1/control/pump", false);
    controlPUMP(false);
  } else {
    pushDataBtn("/KHU_1/control/pump", true);
    controlPUMP(true);
  }

  if ((temp > 26 || temp < 18 || hum > 70 || hum < 50)) {
    pushDataBtn("/KHU_1/control/fan", true);
    controlFAN(true);
  } else {
    pushDataBtn("/KHU_1/control/fan", false);
    controlFAN(false);
  }
  if (light < 5 && hum > 50 && hum < 70) {
    pushDataBtn("/KHU_1/control/light", true);
    controlLIGHT(true);
  } else {
    pushDataBtn("/KHU_1/control/light", false);
    controlLIGHT(false);
  }
  if (rain > 50 || light > 30) {
  pushDataBtn("/KHU_1/control/curtain", true);
  stepMotor(true);
} else {
  pushDataBtn("/KHU_1/control/curtain", false);
  stepMotor(false);
}
  



}
void handleAuto_2(float temp2, float hum2, float light2, int doam2 ) {
  if (doam2 > 50) {
    pushDataBtn("/KHU_2/control/pump", false);
    controlPUMP_2(false);
  } else {
    pushDataBtn("/KHU_2/control/pump", true);
    controlPUMP_2(true);
  }

  if (temp2 > 30 || hum2 > 70) {
    pushDataBtn("/KHU_2/control/fan", true);
    controlFAN_2(true);
  } else {
    pushDataBtn("/KHU_2/control/fan", false);
    controlFAN_2(false);
  }
  if (light2 < 3 && hum2 > 60) {
    pushDataBtn("/KHU_2/control/light", true);
    controlLIGHT_2(true);
  } else {
    pushDataBtn("/KHU_2/control/light", false);
    controlLIGHT_2(false);
  }

}
