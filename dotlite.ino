// dotlite.ino - simplyfied dotti compatible device to be used with 
// https://github.com/WebBluetoothCG/demos/tree/gh-pages/bluetooth-led-display

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Adafruit_NeoPixel.h>

#define PIN         13
#define NUMPIXELS   64
#define BRIGHTNESS 128

#define SERVICE_UUID "0000fff0-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_UUID "0000fff3-0000-1000-8000-00805f9b34fb"

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define setPixel(p,r,g,b)  pixels.setPixelColor(p-1, pixels.Color(r, g, b))

// color map for splash screen
uint8_t colors[][3] = { 
  {0,0,0}, {255,255,0}, {255,0,0}, {255,255,255}
};

// splash screen
uint8_t splash[] = {
  0,0,1,1,1,1,0,0,
  0,1,3,1,1,3,1,0,
  1,3,3,1,1,3,3,1,
  1,3,3,1,1,3,3,1,
  1,1,1,1,1,1,1,1,
  1,1,2,2,2,2,1,1,
  0,1,1,2,2,1,1,0,
  0,0,1,1,1,1,0,0,
};

class Callbacks: public BLECharacteristicCallbacks {
  
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      // set all pixels
      if(value.length() == 5 && value[0] == 6 && value[1] == 1) {
        for(uint8_t i=1;i<=NUMPIXELS;i++) setPixel(i, value[2], value[3], value[4]);
        pixels.show();          
      }
        
      // set individiual pixel
      if(value.length() == 6 && value[0] == 7 && value[1] == 2) {
        setPixel(value[2], value[3], value[4], value[5]);
        pixels.show();
      }
    }
};

void setup() {
  BLEDevice::init("Dotti");
  BLEServer *pServer = BLEDevice::createServer();
  
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_WRITE |
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                         BLECharacteristic::PROPERTY_READ
                                       );
  BLEDescriptor *pDescriptor = new BLEDescriptor((uint16_t)0x2902); 
  pCharacteristic->addDescriptor(pDescriptor);
  
  pCharacteristic->setCallbacks(new Callbacks());
  pCharacteristic->setValue("dotilite");
  pService->start();

  // advertise service
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();  

  // setup rgb matrix display
  pixels.begin();
  pixels.clear();
  pixels.setBrightness(BRIGHTNESS);

  // splash screen for a second to indicate successful boot
  for(uint8_t i=0;i<NUMPIXELS;i++)  
    setPixel(i+1, colors[splash[i]][0], colors[splash[i]][1], colors[splash[i]][2]);
    
  pixels.show();
  delay(1000);
  pixels.clear();
  pixels.show();
}

void loop() {
  delay(1000);
}
