#include "sys/time.h"
#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEServer.h"
#include "BLEBeacon.h"
#include "esp_sleep.h"
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#define GPIO_DEEP_SLEEP_DURATION     5  // sleep x seconds and then wake up


RTC_DATA_ATTR static time_t last;        // remember last boot in RTC Memory
RTC_DATA_ATTR static uint32_t bootcount; // remember number of boots in RTC Memory
int n = 0;
const int PIN = 2;
const int CUTOFF = -74;
int RSSIn1 = 0;
int RSSIn = 0;
int best = 0;
int rssi_old = 0;
int best_old = 0;
int i = 0;
int k = 0;
String nomen;

int average = 0;
int iterations = 0;
int PASSKEY = 0;
int Sleep_Key = 0;

const int numReadings = 5;

int bests[5];

const int rPin = 34;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
BLEAdvertising *pAdvertising;   // BLE Advertisement type
struct timeval now;

#define BEACON_UUID "76c74bd9-7270-49d8-b24d-17b8550bc224" // UUID 1 128-Bit (may use linux tool uuidgen or random numbers via https://www.uuidgenerator.net/)


void setBeacon() {

  BLEBeacon oBeacon = BLEBeacon();
  oBeacon.setManufacturerId(0x4C00); // fake Apple 0x004C LSB (ENDIAN_CHANGE_U16!)
  oBeacon.setProximityUUID(BLEUUID(BEACON_UUID));
  oBeacon.setMajor((bootcount & 0xFFFF0000) >> 16);
  oBeacon.setMinor(bootcount & 0xFFFF);
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();

  oAdvertisementData.setFlags(0x04); // BR_EDR_NOT_SUPPORTED 0x04

  std::string strServiceData = "";

  strServiceData += (char)26;     // Len
  strServiceData += (char)0xFF;   // Type
  strServiceData += oBeacon.getData();
  oAdvertisementData.addData(strServiceData);

  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setScanResponseData(oScanResponseData);
}

void setup() {
 pinMode(PIN, OUTPUT);
  Serial.begin(115200);
  randomSeed(analogRead(rPin));
  Serial.println("ESP32 Initialized");
  BLEDevice::init("ESP32");
 if (bootcount%2 == 0) {
     gettimeofday(&now, NULL);
      Serial.printf("start ESP32 %d\n", bootcount++);
      Serial.printf("deep sleep (%lds since last reset, %lds since last boot)\n", now.tv_sec, now.tv_sec - last);
     last = now.tv_sec;
      // Create the BLE Server
     BLEServer *pServer = BLEDevice::createServer(); // <-- no longer required to instantiate BLEServer, less flash and ram usage
     pAdvertising = BLEDevice::getAdvertising();
     BLEDevice::startAdvertising();
     setBeacon();
      // Start advertising
     pAdvertising->start();
     Serial.println("Advertizing started...");
     delay(10000);
     pAdvertising->stop();
     Serial.printf("enter deep sleep\n");
     esp_deep_sleep(1000000LL * GPIO_DEEP_SLEEP_DURATION);
     Serial.printf("in deep sleep\n");
 }

 }

void loop() {

int connect_attempt = 0;

Serial.print("bootcount: ");
Serial.println(bootcount);

//BOOTCOUNT IF LOOP//
if (bootcount%2 != 0) {
 
      gettimeofday(&now, NULL);
         Serial.printf("start ESP32 %d\n", bootcount++);
         Serial.printf("deep sleep (%lds since last reset, %lds since last boot)\n", now.tv_sec, now.tv_sec - last);
         last = now.tv_sec;
          
          BLEScan *scan = BLEDevice::getScan();
            scan->setActiveScan(true);
            BLEScanResults results = scan->start(1);
            int best = CUTOFF;
            Serial.println("out of loop");
            Serial.print("results.GetCount(): ");
            Serial.println(results.getCount());
           
while(iterations < 6){          
for (int i = 0; i < results.getCount(); i++) {
   PASSKEY = 1;
    Serial.print("Passkey"); 
    Serial.println(PASSKEY);
    BLEAdvertisedDevice device = results.getDevice(i);
    delay(5);
    std::string nomen = device.getName();
    if (nomen == "ESP32"){
       int rssi = device.getRSSI();
      Serial.println("ESP32 Found");
      if (rssi > best) {
        best = rssi;
      }
    
      else{
        best = min(best,rssi);
        delay(100);
      }
     delay(5);
    }
    Serial.print("Best: ");
    Serial.println(best);
}
    Serial.print("Passkey"); 
    Serial.println(PASSKEY);
if(PASSKEY == 1){
// subtract the last reading:

Serial.print("Bests: ");
Serial.println(bests[k]);
Serial.print("K: ");
Serial.println(k);
k++;
iterations++;
Serial.print("K: ");
Serial.println(k);

if(k == 4){
    int total = bests[0]+bests[1]+bests[2]+bests[3]+bests[4];
    average = total/4;
  


  delay(1);        // delay in between reads for stability
  Serial.print("Average: ");
  Serial.println(average);
  digitalWrite(PIN, average > CUTOFF ? HIGH : LOW);
  delay(500);
  Serial.print("Iterations: ");
  Serial.println(iterations);
  Serial.print(":::::::::::::::::::::::::::::");
}

 if (iterations == 4){
     k =0;
     delay (random(2,10));
     Serial.printf("enter deep sleep\n");
     iterations = 0;
     delay(10);
     esp_deep_sleep(1000000LL * GPIO_DEEP_SLEEP_DURATION);
     Serial.printf("in deep sleep\n");
     Sleep_Key = 1;
     
    }
}
}
    PASSKEY = 0;

}
 if (bootcount%2 == 0 && Sleep_Key == 1) {
  Serial.println("****************");
     gettimeofday(&now, NULL);
      Serial.printf("start ESP32 %d\n", bootcount++);
      Serial.printf("deep sleep (%lds since last reset, %lds since last boot)\n", now.tv_sec, now.tv_sec - last);
     last = now.tv_sec;
      // Create the BLE Device
     BLEDevice::init("ESP32");
      // Create the BLE Server
     BLEServer *pServer = BLEDevice::createServer(); // <-- no longer required to instantiate BLEServer, less flash and ram usage
     pAdvertising = BLEDevice::getAdvertising();
     BLEDevice::startAdvertising();
     setBeacon();
      // Start advertising
     pAdvertising->start();
     Serial.println("Advertizing started...");
     delay(10000);
     pAdvertising->stop();
     Serial.printf("enter deep sleep\n");
     esp_deep_sleep(1000000LL * GPIO_DEEP_SLEEP_DURATION);  
     Serial.printf("in deep sleep\n");
     Sleep_Key = 0;
 }
}
