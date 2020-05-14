#include <WiFi.h> 
const char ssid[] = "JOJO";
const char password[] = "jojo@elmo";
#include <ThingsBoard.h>
#include <PubSubClient.h>

#include <BLEDevice.h>
#include <BLEAdvertisedDevice.h>

#define TOKEN "VydEpUIDFGnybnrNhMPK"
#define THINGSBOARD_SERVER "demo.thingsboard.io"

// Initialize ThingsBoard client
WiFiClient espClient;
// Initialize ThingsBoard instance
ThingsBoard tb(espClient);

static BLEUUID EnvironmentalService_UUID("0000181A-0000-1000-8000-00805F9B34FB");
static BLEUUID tempCharacteristic_UUID("00002A6E-0000-1000-8000-00805F9B34FB");
static BLEUUID humidCharacteristic_UUID("00002A6F-0000-1000-8000-00805F9B34FB");

static BLEUUID IndoorService_UUID("00001821-0000-1000-8000-00805F9B34FB");
static BLEUUID LatitudeCharacteristic_UUID("00002AAE-0000-1000-8000-00805F9B34FB");
static BLEUUID LongitudeCharacteristic_UUID("00002AAF-0000-1000-8000-00805F9B34FB");

static BLEAddress* pServerAddress;
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLERemoteCharacteristic* pRemoteCharacteristic2;
static BLERemoteCharacteristic* pRemoteCharacteristic3;
static BLERemoteCharacteristic* pRemoteCharacteristic4;
static BLEAdvertisedDevice* myDevice;
BLEScan *pBLEScan;

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,uint8_t* pData,size_t length, bool isNotify) {}
static void notifyCallback2(BLERemoteCharacteristic* pBLERemoteCharacteristic2,uint8_t* pData,size_t length, bool isNotify) {}
static void notifyCallback3(BLERemoteCharacteristic* pBLERemoteCharacteristic3,uint8_t* pData,size_t length, bool isNotify) {}
static void notifyCallback4(BLERemoteCharacteristic* pBLERemoteCharacteristic4,uint8_t* pData,size_t length, bool isNotify) {}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }
  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");
    pClient->setClientCallbacks(new MyClientCallback());

    pClient->connect(myDevice);  
    Serial.println(" - Connected to server");

    BLERemoteService* pRemoteService = pClient->getService(EnvironmentalService_UUID);
    BLERemoteService* pRemoteService2 = pClient->getService(IndoorService_UUID);
    if (pRemoteService == nullptr && pRemoteService2 == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(EnvironmentalService_UUID.toString().c_str());
      Serial.println(IndoorService_UUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");

    pRemoteCharacteristic = pRemoteService->getCharacteristic(tempCharacteristic_UUID);
    pRemoteCharacteristic2 = pRemoteService->getCharacteristic(humidCharacteristic_UUID);
    pRemoteCharacteristic3 = pRemoteService2->getCharacteristic(LatitudeCharacteristic_UUID);
    pRemoteCharacteristic4 = pRemoteService2->getCharacteristic(LongitudeCharacteristic_UUID);
    if (pRemoteCharacteristic == nullptr && pRemoteCharacteristic2 == nullptr && pRemoteCharacteristic3 == nullptr && pRemoteCharacteristic4 == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(tempCharacteristic_UUID.toString().c_str());
      Serial.println(humidCharacteristic_UUID.toString().c_str());
      Serial.println(LatitudeCharacteristic_UUID.toString().c_str());
      Serial.println(LongitudeCharacteristic_UUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    if(pRemoteCharacteristic->canRead() && pRemoteCharacteristic2->canRead()&& pRemoteCharacteristic3->canRead()&& pRemoteCharacteristic4->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
      std::string value2 = pRemoteCharacteristic2->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value2.c_str());
      std::string value3 = pRemoteCharacteristic3->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value3.c_str());
      std::string value4 = pRemoteCharacteristic4->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value4.c_str());
    }

    pRemoteCharacteristic->registerForNotify(notifyCallback);
    pRemoteCharacteristic2->registerForNotify(notifyCallback2);
    pRemoteCharacteristic3->registerForNotify(notifyCallback3);
    pRemoteCharacteristic4->registerForNotify(notifyCallback4);

    connected = true;
}
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {

  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(EnvironmentalService_UUID) && advertisedDevice.isAdvertisingService(IndoorService_UUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } 
  } 
}; 


void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(10);

  WiFi.mode(WIFI_STA);
} 

void loop() {

  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  if (WiFi.status() != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(ssid);
      while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(ssid, password);
        Serial.print(".");
        delay(5000);
      }
      Serial.println("\nConnected.");
  }

  if (!tb.connected()) {
    //subscribed = false;

    // Connect to the ThingsBoard
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect");
      return;
    }
  }

  if (connected) {
    Serial.print("Hasil DHT11 : ");
    std::string newValue = pRemoteCharacteristic->readValue();
    std::string newValue2 = pRemoteCharacteristic2->readValue();
    std::string newValue3 = pRemoteCharacteristic3->readValue();
    std::string newValue4 = pRemoteCharacteristic4->readValue();  
    
    String dataSensor = newValue.c_str();
    String dataSensor2 = newValue2.c_str();
    String dataSensor3 = newValue3.c_str();
    String dataSensor4 = newValue4.c_str();
    
    int suhu = dataSensor.substring(0,5).toInt();
    int kelembaban = dataSensor2.substring(0,5).toInt();
    String latStr = dataSensor3.substring(0,5);
    String longiStr = dataSensor4.substring (0,5);
    float latitude = latStr.toFloat();
    float longitude = longiStr.toFloat(); 
    
    tb.sendTelemetryFloat("temperature", suhu);
    tb.sendTelemetryFloat("humidity", kelembaban);
    tb.sendTelemetryFloat("latitude", latitude);
    tb.sendTelemetryFloat("longitude", longitude);
    
    Serial.print("Suhu : "); 
    Serial.print(suhu);
    Serial.println();
    Serial.print("Kelembaban : "); 
    Serial.print(kelembaban);
    Serial.println();
    Serial.print("Latitude :");
    Serial.print(latitude);
    Serial.println();
    Serial.print("Longitude :");
    Serial.print(longitude);
    Serial.println();

  }else if(doScan){
    BLEDevice::getScan()->start(0);  
  }
  //tb.loop.toInt();
  delay(3000); 
} 
