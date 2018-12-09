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
float calibrationFactor = 4.5;
float flowRate;
float triggerMAX = 40;
float triggerMIN = 10;
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

            String name = Firebase.push("/NODE2/flowrate", root);
            if ((flowRate > triggerMIN) && (flowRate < triggerMAX)){
                Firebase.setInt("/NODE2/LED2",1);
            }else{
                Firebase.setInt("/NODE2/LED2",0);
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
