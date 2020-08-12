#include "sys/time.h"
#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEServer.h"
#include "BLEBeacon.h"
#include "esp_sleep.h"
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define GPIO_DEEP_SLEEP_DURATION     10  // sleep x seconds and then wake up
RTC_DATA_ATTR static time_t last;        // remember last boot in RTC Memory
RTC_DATA_ATTR static uint32_t bootcount; // remember number of boots in RTC Memory


BLEAdvertising *pAdvertising;
struct timeval now;

#define BEACON_UUID           "8ec76ea3-6668-48da-9866-75be8bc86f4d"
const int PIN = 2;
const int CUTOFF = -70;
int k = 0;
int average = 0;

int bests[5]; 

void setBeacon() {

  BLEBeacon oBeacon = BLEBeacon();
  oBeacon.setManufacturerId(0x4C00); // fake Apple 0x004C LSB (ENDIAN_CHANGE_U16!)
  oBeacon.setProximityUUID(BLEUUID(BEACON_UUID));
  oBeacon.setMajor((bootcount & 0xFFFF0000) >> 16);
  oBeacon.setMinor(bootcount&0xFFFF);
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
  Serial.begin(115200);
  Serial.print("Scanning...");

  gettimeofday(&now, NULL);
  Serial.printf("start ESP32 %d\n",bootcount++);
  Serial.printf("deep sleep (%lds since last reset, %lds since last boot)\n",now.tv_sec,now.tv_sec-last);
  last = now.tv_sec;
  
  pinMode(PIN, OUTPUT);
  BLEDevice::init("ESP32");

  pAdvertising = BLEDevice::getAdvertising();
  setBeacon();
   // Start advertising
  pAdvertising->start();
  Serial.println("Advertizing started...");
  delay(100);
  //pAdvertising->stop();
  //Serial.printf("enter deep sleep\n");
  //esp_deep_sleep(1000000LL * GPIO_DEEP_SLEEP_DURATION);
  //Serial.printf("in deep sleep\n");
}

void loop() {
  //////////////////////////////////////////
  BLEScan *scan = BLEDevice::getScan();
  scan->setActiveScan(true);
  BLEScanResults results = scan->start(1);
  int best = CUTOFF;
 
            Serial.println("out of loop");
            Serial.print("results.GetCount(): ");
            Serial.println(results.getCount());
            Serial.println("==========================");
            
  //////////////////////////////////////////
 for (int i = 0; i < results.getCount(); i++) {
    BLEAdvertisedDevice device = results.getDevice(i);
    
    int rssi = device.getRSSI();
       std::string nomen = device.getName();
       Serial.print("Nomen: ");
       Serial.println(device.getName().c_str());
   
    if (rssi > best) {
      best = rssi;
    }
    Serial.println(best);
  }
  digitalWrite(PIN, best > CUTOFF ? HIGH : LOW);
}
