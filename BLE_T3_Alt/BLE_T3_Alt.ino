#include "sys/time.h"
#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEServer.h"
#include "BLEBeacon.h"
#include "esp_sleep.h"
#include "BLEScan.h"
#include "BLEAdvertisedDevice.h"

#define GPIO_DEEP_SLEEP_DURATION     10  // sleep x seconds and then wake up
RTC_DATA_ATTR static time_t last;        // remember last boot in RTC Memory
RTC_DATA_ATTR static uint32_t bootcount; // remember number of boots in RTC Memory

String nomen;
String SUUID;

BLEAdvertising *pAdvertising;
struct timeval now;
String UUID;
#define BEACON_UUID           "8ec76ea3-6668-48da-9866-75be8bc86f4d"
const int PIN = 2;
const int CUTOFF = -75;
int k = 0;
int average = 0;

int bests[5];




class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
void onResult(BLEAdvertisedDevice advertisedDevice) {
Serial.println(advertisedDevice.toString().c_str());

}
};

void setBeacon() {

  BLEBeacon oBeacon = BLEBeacon();
  oBeacon.setManufacturerId(0x4C00); // fake Apple 0x004C LSB (ENDIAN_CHANGE_U16!)
  oBeacon.setProximityUUID(BLEUUID(BEACON_UUID));
  oBeacon.setMajor((bootcount & 0xFFFF0000) >> 16);
  oBeacon.setMinor(bootcount & 0xFFFF);
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();

  oAdvertisementData.setFlags(0x04); // BR_EDR_NOT_SUPPORTED 0x04

  std::string strServiceData = "Prox";

  strServiceData += (char)26;     // Len
  strServiceData += (char)0xFF;   // Type
  strServiceData += oBeacon.getData();
  oAdvertisementData.addData(strServiceData);

  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setScanResponseData(oScanResponseData);
  pAdvertising->addServiceUUID(BEACON_UUID);
}


void setup() {
  Serial.begin(115200);
  Serial.print("Scanning...");

  gettimeofday(&now, NULL);
  Serial.printf("start ESP32 %d\n", bootcount++);
  Serial.printf("deep sleep (%lds since last reset, %lds since last boot)\n", now.tv_sec, now.tv_sec - last);
  last = now.tv_sec;

  pinMode(PIN, OUTPUT);
  BLEDevice::init("ESP32");
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BEACON_UUID);
  
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

  //////////////////////////////////////////
  for (int i = 0; i < results.getCount(); i++) {
    BLEAdvertisedDevice device = results.getDevice(i);
   // std::string nomen = device.getName();
    int rssi = device.getRSSI();
    Serial.println("In Loop");
    Serial.println("==========================");

 nomen = device.getAddress().toString().c_str();
 Serial.print("Address: ");
 Serial.println(device.getAddress().toString().c_str());
  Serial.print("RSSI: ");
 Serial.println(rssi);
  if (nomen.startsWith("24:62")){
// Serial.print("Address: ");
// Serial.println(device.getAddress().toString().c_str());
 Serial.print("RSSI: ");
 Serial.println(rssi);
    Serial.println("DEVICE FOUND");
    if (rssi > best) {
      best = rssi;
    }
    else{
      best = min(best,rssi);
      delay(10);
    }

  }

    }

    bests[k] = best;
     k++;
    if (k == 3){
    int total = bests[0]+bests[1]+bests[2];
    average = total/3;


    Serial.print("Average: ");
    Serial.println(average);
    digitalWrite(PIN, average > CUTOFF ? HIGH : LOW);
    delay(100);
    k = 0;

    }
    delay(20); 
}

 
