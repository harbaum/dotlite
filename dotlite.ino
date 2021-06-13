// dotlite.ino - simplyfied dotti compatible device to be used with 
// https://github.com/WebBluetoothCG/demos/tree/gh-pages/bluetooth-led-display

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Adafruit_NeoPixel.h>

#define PIN          13
#define WIDTH         8
#define HEIGHT        8
#define NUMPIXELS   (WIDTH*HEIGHT)
#define BRIGHTNESS  128

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

// from https://github.com/lrucker1/homebridge-dotti/blob/master/dotti.js
#define CMD_SET_LED         0x0702
#define CMD_SET_LED_ALL     0x0601
#define CMD_SET_LED_ROW     0x0704
#define CMD_SET_LED_COLUMN  0x0703
#define CMD_MODE            0x0405
#define CMD_SYNC_TIME       0x0609
#define CMD_ANIMATION_SPEED 0x0415
#define CMD_SAVE_SCREEN     0x0607
#define CMD_LOAD_SCREEN     0x0608
    
class Callbacks: public BLECharacteristicCallbacks {
  
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if(value.length() >= 2) {
        // at least two bytes for command word are needed
        uint16_t cmd = 256 * value[0] + value[1];

      	switch(cmd) {
          case CMD_SET_LED:     // set individiual pixel
            if(value.length() == 6) {
              setPixel(value[2], value[3], value[4], value[5]);
              pixels.show();
            }
            break;
            
	        case CMD_SET_LED_ALL: // set all pixels
	          if(value.length() == 5) {
	            for(uint8_t i=1;i<=NUMPIXELS;i++) setPixel(i, value[2], value[3], value[4]);
	            pixels.show();          
	          }
	          break;
        
          case CMD_SET_LED_ROW: // set all pixels on a row
            if(value.length() == 5) {
              for(uint8_t i=1;i<=WIDTH;i++) setPixel(i+(value[2]-1)*WIDTH, value[3], value[4], value[5]);
              pixels.show();          
            }
            break;
            
          case CMD_SET_LED_COLUMN: // set all pixels on a column
            if(value.length() == 5) {
              for(uint8_t i=0;i<HEIGHT;i++) setPixel(value[2]+i*WIDTH, value[3], value[4], value[5]);
              pixels.show();          
            }
            break;
	        }
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
