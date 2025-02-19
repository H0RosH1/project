#include "DHT.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFiClientSecure.h>
#include <BH1750FVI.h>
#define DHTPIN D3

#define DHTTYPE DHT22
#define MQ_137 A0

DHT dht(D3,DHTTYPE);
BH1750FVI LightSensor(BH1750FVI::k_DevModeContLowRes);

float humidityData;
float temperatureData;
float fahrenheitData;

float analysis(int);
float Rs;
int sensorValue;
int hard_ware;
uint16_t lux;
int ledPin = D2;
const char* ssid = "test"; //"NP-Park-A513";// 
const char* password = "password"; //"33732036";
//WiFiClient client;
char server[] = "cksmartcare.com";   //eg: 192.168.16.61
const char* fringerprint = "6C 88 57 9F DA 5F 4F F1 FA 11 9B F5 DB A0 2D E7 AA C6 48 43";//b
                          //6C 88 57 9F DA 5F 4F F1 FA 11 9B F5 DB A0 2D E7 AA C6 48 43
WiFiClientSecure client;

void setup() {
  pinMode(D6, OUTPUT);//green ligth
  pinMode(D7, OUTPUT);//red ligth(default LOW)
  digitalWrite(D7,HIGH);// red ligth off
  Serial.begin(115200);
  delay(10);
  dht.begin();
  LightSensor.begin();
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(D6,LOW);   //green ligth on
    delay(1000);
    digitalWrite(D6,HIGH);  //green ligth off
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("Server started");
  Serial.print(WiFi.localIP());
  delay(100);
  Serial.println("connecting...");
  digitalWrite(D6,LOW);  //green ligth on
  pinMode(LED_BUILTIN, OUTPUT);
 }
void loop() { 
  while (hard_ware != 1) {
    sensorValue = analogRead(MQ_137);
    lux = LightSensor.GetLightIntensity();
    humidityData = dht.readHumidity();
    temperatureData = dht.readTemperature(),1;
    fahrenheitData = dht.readTemperature(true),1;
    check_value(temperatureData,humidityData,lux);
    digitalWrite(D6,LOW);  //green ligth off
  }
  client.setFingerprint(fringerprint); //key
  Sending_To_phpmyadmindatabase();
  //digitalWrite(LED_BUILTIN, HIGH);
  //delay(600000); // interval
  delay(100000);
}

float analysis(int adc){
  float slope = -0.4150374993;
  float A = 5.49597909;
  float Rseries = 1000;
  float V_Rseries = ((float)adc*5)/1023;
  Rs = ((5-V_Rseries)/V_Rseries)*Rseries;
  float R0 = 787.172;
  float Y = Rs/R0;
  double Methane_gas = pow(10,(log10(Y/A)/slope));
  return Methane_gas;
}

 void Sending_To_phpmyadmindatabase()  //CONNECTING WITH MYSQL
 {
   if (client.connect(server, 443)) {
    digitalWrite(D6,LOW);//green ligth on
    Serial.println("connected");
    // Make a HTTP request:
    client.print("GET /send-data-sensor?sm_temperature=");     //YOUR URL
    Serial.print("temperatureData: ");
    Serial.print(temperatureData); //องศาC
    Serial.println("C");
    client.print(temperatureData); //ความชื่น
    client.print("&sm_humudity="); //ความชื่น
    Serial.print("humidity: ");
    client.print(humidityData);
    Serial.print(humidityData);
    Serial.println("%");
    client.print("&sm_light_intensity="); //ความเข้มแสง
    client.print(lux); //ความเข้มแสง
    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lux");
    client.print("&sm_ammonia="); //nh3
    client.print(analysis(sensorValue),3);
    Serial.print("Ammonia : ");
    Serial.print(analysis(sensorValue),3);
    Serial.println(" ppm");
    Serial.println(" ");
    client.print(" ");      //SPACE BEFORE HTTP/1.1
    client.print("HTTP/1.1");
    client.println();
    client.println("Host: cksmartcare.com");
    client.println("Connection: close");
    client.println();
    digitalWrite(D6,HIGH);  //green ligth off
    delay(500);
    digitalWrite(D6,LOW);//green ligth on
  } else {
    // if you didn't get a connection to the server:
    digitalWrite(D7,LOW); //red ligth on
    digitalWrite(D6,HIGH); //red ligth on
    Serial.println("connection failed");
    Serial.print("temperatureData: ");
    Serial.print(temperatureData); //องศาC
    Serial.println("C");
    Serial.print("humidity: ");
    Serial.print(humidityData);
    Serial.println("%");
    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lux");
    Serial.print("Ammonia : ");
    Serial.print(analysis(sensorValue),3);
    Serial.println(" ppm");
    
  }
}

void check_value(float c,float h,float l){
  if(isnan(c) || isnan(h) || l == 54612 ){
    hard_ware = 0;
    Serial.println("error");
    digitalWrite(D7,LOW);// red ligth on
    delay(1000);
    digitalWrite(D7,HIGH);// red ligth off
    delay(1000);
  }else{
    hard_ware =1;
    Serial.println("ok");
  }
}
