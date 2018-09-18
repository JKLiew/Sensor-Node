//  ESP8266 library
#include <ESP8266WiFi.h>

//  Firebase library
#include <FirebaseArduino.h>
#include <ArduinoJson.h>

//  DHT22 library
#include <dht.h>

//  Firebase databse secret
#define FIREBASE_HOST "jk-fyp-1234.firebaseio.com"
#define FIREBASE_AUTH "brhDDULhPFbIpgEd9McHx5kfLVf7oYjCA9AhVoS2"

//  SSID and password
#define WIFI_SSID "JK Honor"
#define WIFI_PASSWORD "technical"

//  DHT22 Sensor declaration
#define DHT22_1 2
dht DHT_1;
int chk_1;

//  Declare JSON buffer
const size_t bufferSize = JSON_OBJECT_SIZE(2) + 76;
DynamicJsonBuffer jsonBuffer(bufferSize);
JsonObject& SensorJson = jsonBuffer.createObject();
JsonObject& tempTime = SensorJson.createNestedObject("timestamp");

//  Timer1 variable setup
unsigned int TickCount = 0;
boolean inteuurptT1Flag = 0;
unsigned int Period = 20; // in seconds

void setup() {
//  Timer1 interrupt setup 
  timer1_disable();
  timer1_attachInterrupt(timer1_callISR);
  timer1_isr_init();
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP); // 5MHz
  timer1_write(5000000);  // Tick every 1 seconds

//  Enable watchdog timer for 8 seconds period
  ESP.wdtDisable();
  ESP.wdtEnable(WDTO_8S);

//  Setup serial monitor
  Serial.begin(74880);

//  Connect to Wi-fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  
//  Connect to firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

//  Initialize timestamp
  tempTime[".sv"] = "timestamp";
}

void loop() {
  if (inteuurptT1Flag){
    readSensor();
    pushHumidity(DHT_1.humidity);
    pushTemperature(DHT_1.temperature);
    inteuurptT1Flag = 0;
  }

  int refreshFlag = (Firebase.getString("Refresh Control/Sensor_1")).toInt(); 
  if (refreshFlag){
    readSensor();
    pushHumidity(DHT_1.humidity);
    pushTemperature(DHT_1.temperature);
    Firebase.setString("Refresh Control/Sensor_1", "0");
    refreshFlag = 0;
  }
}

void readSensor (void){
  chk_1 = DHT_1.read22(DHT22_1);
  Serial.print("Temp_1 = ");
  Serial.print(DHT_1.temperature);
  Serial.print("\t");
  Serial.print("Humi_1 = ");
  Serial.println(DHT_1.humidity);
}

void pushTemperature(double tempTemp) {
  String thisString = String(tempTemp);
  SensorJson["Value"] = thisString;
//  Firebase.pushString("Sensors/Sensor_1/Temperature/Sensor Value", thisString);
  Firebase.push("Sensors/Sensor_1/Temperature", SensorJson);
  if (Firebase.failed()) {
    Serial.print("Sensors/Sensor_1/Temperature failed:");
    Serial.println(Firebase.error());
    return;
  }
}

void pushHumidity(double tempHumid) {
  String thisString = String(tempHumid);
  SensorJson["Value"] = thisString;
//  Firebase.pushString("Sensors/Sensor_1/Humidity/Sensor Value", thisString);
  Firebase.push("Sensors/Sensor_1/Humidity", SensorJson);
  if (Firebase.failed()) {
    Serial.print("Sensors/Sensor_1/Humidity failed:");
    Serial.println(Firebase.error());
    return;
  }
}

void timer1_callISR(void) {
  TickCount++;
  wdt_reset();
  if (TickCount == Period) {
    TickCount = 0;
    inteuurptT1Flag = 1;
  }
}

