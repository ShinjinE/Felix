/*
   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
   Has a characteristic of: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - used for receiving data with "WRITE" 
   Has a characteristic of: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - used to send data with  "NOTIFY"

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   In this example rxValue is the data received (only accessible inside that function).
   And txValue is the data to be sent, in this example just a byte incremented every second. 
*/
#include "HyperDisplay_UG2856KLBAG01.h"  //OLED disply, sparkfun
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define SERIAL_PORT Serial  
//SPI Setup
#define SPI_PORT SPI        
#define RES_PIN 2           // Optional
#define CS_PIN 4            
#define DC_PIN 13

UG2856KLBAG01_SPI myOLED; //OLED connect
BLECharacteristic *pCharacteristic;
bool deviceConnected = false; //BLE connected successfully
bool updateScreen = false; //update only once per rxValue received
float txValue = 0; //send to client
std::string rxValue; //receive from client


class MyServerCallbacks: public BLEServerCallbacks {
  
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.print("Connected");
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.print("Disconnected");
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  
    void onWrite(BLECharacteristic *pCharacteristic) {
        myOLED.windowClear();
        myOLED.setWindowColorSet(); //OLED background set to white and text to blue
        myOLED.setTextCursor(0,0); //top left
        updateScreen = true; 

        rxValue = pCharacteristic->getValue(); //get value from client
        if (rxValue.length() > 0) {
            Serial.println("*********");
            Serial.print("Received Value: ");

            for (int i = 0; i < rxValue.length(); i++) {
                Serial.print(rxValue[i]);
            }

            Serial.println();
            Serial.println("*********");
        }
    }
};

void setup() {
    Serial.begin(115200);

    SPI_PORT.begin();
    myOLED.begin(CS_PIN, DC_PIN, SPI_PORT);
    myOLED.windowClear();
    myOLED.setWindowColorSet();
    myOLED.setTextCursor(0,0);

    // Create the BLE Device
    BLEDevice::init("BLE Lucky Glasses"); // Give it a name

    // Create the BLE Server
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create a BLE Characteristic
    pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
                      
    pCharacteristic->addDescriptor(new BLE2902());

    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

    pCharacteristic->setCallbacks(new MyCallbacks());

    // Start the service
    pService->start();

    // Start advertising
    pServer->getAdvertising()->start();
    Serial.println("Waiting a client connection to notify...");
}

void loop() {
    if(deviceConnected) {
        if(updateScreen){
            for (int i = 0; i < rxValue.length(); i++) {
                myOLED.print(rxValue[i]);
            }
            updateScreen = false;
        }
    }
    delay(1000);
}
