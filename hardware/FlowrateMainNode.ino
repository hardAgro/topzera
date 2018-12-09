#include <Arduino.h> 
#include <EEPROM.h> 
#include <ESP8266WiFi.h> 
#include <ESP8266HTTPClient.h>
#include <FirebaseArduino.h>
#include <WiFiUDP.h>
#include <NTPClient.h>

#define NTP_OFFSET   60 * 60      
#define NTP_INTERVAL 60 * 1000    
#define NTP_ADDRESS  "a.st1.ntp.br"

#define FIREBASE_HOST "flowsensor-bfbed.firebaseio.com"
#define FIREBASE_AUTH "GioHchniUN4qwKrZwIxwZbM7mYGUYw4dET5P858O"

int16_t utc = -3; 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "a.st1.ntp.br", utc*3600, 60000);

int addr = 0; 
int lp = 0;
int lp_time = 15;            
const int buttonPin = D2; 
const int led1Green = D5;
const int led1Red = D6;
const int led2Green = D7;
const int led2Red = D8;
float calibrationFactor = 4.5;
float triggerMAX = 40;
float triggerMIN = 10;
float flowRate;
byte sensorInterrupt = 0; 
char push_data[200]; 
volatile byte pulseCount;
unsigned int flowLitros;
unsigned long totalLiquid;
unsigned long oldTime;
const char * ssid = "inova.farm";
const char * password = "";

HTTPClient http;

void setup() {
    Serial.begin(115200); 
    delay(10);
    Serial.println('\n');

    startWIFI();

    pinMode(buttonPin, INPUT); //signal do sensor
    pinMode(led1Green, OUTPUT);
    pinMode(led1Red, OUTPUT);
    pinMode(led2Green, OUTPUT);
    pinMode(led2Red, OUTPUT);
    pulseCount = 0;
    flowRate = 0.0;
    flowLitros = 0;
    totalLiquid = 0;
    oldTime = 0;

    timeClient.begin();
    timeClient.update();
    
    digitalWrite(buttonPin, HIGH);
    attachInterrupt(digitalPinToInterrupt(buttonPin), pulseCounter, RISING);
    
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
 
}

void loop() {
    if (WiFi.status() == WL_CONNECTED && (millis() - oldTime) > 1000)  {
        unsigned int frac; //parte fracional

        timeClient.forceUpdate();
       
        flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
        oldTime = millis();
        flowLitros = (flowRate / 60) * 1000;

        totalLiquid += flowLitros;

        Serial.print("Flow rate: ");
        Serial.print(int(flowRate)); 
        Serial.print("."); 

        frac = (flowRate - int(flowRate)) * 10;
        Serial.print(frac, DEC); 
        Serial.print("L/min |");

        Serial.print("Total de Litros: "); 
        Serial.print(totalLiquid);
        Serial.println("mL");

        
        
        lp++;      
        if(lp >= lp_time){
            lp=0;

            StaticJsonBuffer<200> jsonBuffer;
            JsonObject &root = jsonBuffer.createObject();
            root["flowRate"] = (flowRate + frac);
            root["totalLiquid"] = totalLiquid;
            root["timestamp"] = timeClient.getFormattedTime();

            String name = Firebase.push("/NODE1/flowrate", root);
            
            if ((flowRate > triggerMIN) && (flowRate < triggerMAX)) {
                digitalWrite (led1Green, HIGH);
                digitalWrite (led1Red, LOW);
                Firebase.setInt("/NODE1/LED1",1);
            }else{
                digitalWrite (led1Green, LOW);
                digitalWrite (led1Red, HIGH);
                Firebase.setInt("/NODE1/LED1",0);
            }
            
            if(Firebase.getInt("/NODE2/LED2")){
                digitalWrite (led2Green, HIGH);
                digitalWrite (led2Red, LOW);
            }else{
                digitalWrite (led2Green, LOW);
                digitalWrite (led2Red, HIGH);
            }
            
            if (Firebase.failed()) {
                Serial.print("Firebase Pushing /sensor/flowrate failed:");
                Serial.println(Firebase.error()); 
                return;
            }else{
                Serial.print("Firebase Pushed /sensor/flowrate ");
                Serial.println(name);
                delay(2000); 
            }
            
        } 

    } else if (WiFi.status() != WL_CONNECTED) {
        startWIFI();
    }
}

void pulseCounter() {
    pulseCount++;
}

void startWIFI(void) {
    delay(100);              
    
    WiFi.begin(ssid, password);
    Serial.print("Conectando");
    Serial.print(ssid);
    Serial.println(" ...");
    oldTime = 0;
    int i = 0;
    delay(100);         
    
    while (WiFi.status() != WL_CONNECTED) { 
        delay(2000);
        Serial.print(++i);
        Serial.print('.');
        delay(100);  
    }
    delay(2000);
    Serial.print('\n');
    Serial.print("Conectado!\n");
    Serial.print("IP address:\t");
    Serial.print(WiFi.localIP()); 

}
