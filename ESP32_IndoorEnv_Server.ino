#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h> 
#include <DHT.h>

#define EnvironmentalService_UUID "0000181A-0000-1000-8000-00805F9B34FB"
#define tempCharacteristic_UUID "00002A6E-0000-1000-8000-00805F9B34FB"
#define tempDescriptor_UUID     "00002901-0000-1000-8000-00805F9B34FB"
#define humidCharacteristic_UUID "00002A6F-0000-1000-8000-00805F9B34FB"
#define humidDescriptor_UUID     "00002901-0000-1000-8000-00805F9B34FB"

#define IndoorService_UUID          "00001821-0000-1000-8000-00805F9B34FB"
#define LatitudeCharacteristic_UUID "00002AAE-0000-1000-8000-00805F9B34FB"
#define LatitudeDescriptor_UUID     "00002901-0000-1000-8000-00805F9B34FB"
#define LongitudeCharacteristic_UUID "00002AAF-0000-1000-8000-00805F9B34FB"
#define LongitudeDescriptor_UUID     "00002901-0000-1000-8000-00805F9B34FB"

BLECharacteristic EnvironmentalTempCharacteristic(BLEUUID(tempCharacteristic_UUID), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
BLEDescriptor EnvironmentalTempDescriptor(BLEUUID(tempDescriptor_UUID));
BLECharacteristic EnvironmentalHumidCharacteristic(BLEUUID(humidCharacteristic_UUID), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
BLEDescriptor EnvironmentalHumidDescriptor(BLEUUID(humidDescriptor_UUID));
BLECharacteristic IndoorLatitudeCharacteristic(BLEUUID(LatitudeCharacteristic_UUID), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_BROADCAST);
BLEDescriptor IndoorLatitudeDescriptor(BLEUUID(LatitudeDescriptor_UUID));
BLECharacteristic IndoorLongitudeCharacteristic(BLEUUID(LongitudeCharacteristic_UUID), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_BROADCAST);
BLEDescriptor IndoorLongitudeDescriptor(BLEUUID(LongitudeDescriptor_UUID));

bool _BLEClientConnected = false;

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
float kelembaban;
float suhu;

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      _BLEClientConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      _BLEClientConnected = false;
    }
};

void InitBLE() {
  BLEDevice::init("BLE Environmental Sensing & Indoor Positioning");
  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pEnvironmental = pServer->createService(EnvironmentalService_UUID);
  BLEService *pIndoor = pServer->createService(IndoorService_UUID);
  
  pEnvironmental->addCharacteristic(&EnvironmentalTempCharacteristic);
  EnvironmentalTempDescriptor.setValue("Temperature Range (0 - 100)");
  EnvironmentalTempCharacteristic.addDescriptor(&EnvironmentalTempDescriptor);
  EnvironmentalTempCharacteristic.addDescriptor(new BLE2902());

  pIndoor->addCharacteristic(&IndoorLatitudeCharacteristic);
  IndoorLatitudeDescriptor.setValue("Latitude Range (-90) - 90");
  IndoorLatitudeCharacteristic.addDescriptor(&IndoorLatitudeDescriptor);
  IndoorLatitudeCharacteristic.addDescriptor(new BLE2902());

  pEnvironmental->addCharacteristic(&EnvironmentalHumidCharacteristic);
  EnvironmentalHumidDescriptor.setValue("Temperature Range (0 - 100)");
  EnvironmentalHumidCharacteristic.addDescriptor(&EnvironmentalHumidDescriptor);
  EnvironmentalHumidCharacteristic.addDescriptor(new BLE2902());

  pIndoor->addCharacteristic(&IndoorLongitudeCharacteristic);
  IndoorLongitudeDescriptor.setValue("Longitude Range -180 to 80");
  IndoorLongitudeCharacteristic.addDescriptor(&IndoorLongitudeDescriptor);
  IndoorLongitudeCharacteristic.addDescriptor(new BLE2902());
  
  pServer->getAdvertising()->addServiceUUID(EnvironmentalService_UUID);
  pServer->getAdvertising()->addServiceUUID(IndoorService_UUID);
  
  pEnvironmental->start();
  pIndoor->start();
  
  // Start advertising
  pServer->getAdvertising()->start();
}

void setup () {
    Serial.begin(115200); 
    InitBLE();
    dht.begin();
}

  float latitude = -6.175867;
  float longitude = -176.523651;
  
void loop () {
  delay(3000);
   if (_BLEClientConnected) {
    suhu = dht.readTemperature();
    kelembaban = dht.readHumidity();
    
    char suhuconv[8];
    char humidconv[8];
    char latconv[10];
    char longconv[10];
    
    latitude -=0.002005;
    longitude +=0.002005;
    
    std::string suhuvalue = dtostrf(suhu, 3, 2, suhuconv);
    std::string kelembabanvalue = dtostrf(kelembaban, 3, 2, humidconv);
    std::string latvalue = dtostrf(latitude, 3, 2, latconv);
    std::string longvalue = dtostrf(longitude, 3, 2, longconv);
    
    EnvironmentalTempCharacteristic.setValue(suhuvalue);
    EnvironmentalTempCharacteristic.notify(); //Send data to client
    EnvironmentalHumidCharacteristic.setValue(kelembabanvalue);
    EnvironmentalHumidCharacteristic.notify();
    IndoorLatitudeCharacteristic.setValue(latvalue);
    IndoorLatitudeCharacteristic.notify();
    IndoorLongitudeCharacteristic.setValue(longvalue);
    IndoorLongitudeCharacteristic.notify();
    
    if (isnan(suhu) || isnan(kelembaban)) {// isnan, digunakan untuk melakukan cek value pada variable.
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
    
    Serial.print(F("Temperature: "));
    Serial.print(suhu);
    Serial.print(F("°C || Humidity: "));
    Serial.print(kelembaban);
    Serial.print(F("% || Latitude: "));
    Serial.print(latitude, 6);
    Serial.print(F(" || Longitude: "));
    Serial.print(longitude, 6);
    Serial.println();  
  }
}
