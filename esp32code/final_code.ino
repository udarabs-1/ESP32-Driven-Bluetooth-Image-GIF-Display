#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <SPI.h>
#include <FS.h>
#define FS_NO_GLOBALS
//#include<SPIFFS.h>
#include <AnimatedGIF.h>
AnimatedGIF gif;

#define minimum(a,b)     (((a) < (b)) ? (a) : (b))
#include <JPEGDecoder.h>
#include <SPI.h>
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

#define MAX_IMAGE_WIDTH 480 

int16_t xpos = 0;
int16_t ypos = 0;

uint32_t lastFileSize = 0;

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
std::string accumulatedData = "";
uint32_t expectedLength = 0;
uint32_t receivedLength = 0;
int fileType = 0;
String currentFilename = "";

#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID    "beb5483e-36e1-4688-b7f5-ea07361b26a8"

void jpegRender(int xpos, int ypos) {

  // retrieve infomration about the image
  uint16_t  *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  uint32_t min_w = minimum(mcu_w, max_x % mcu_w);
  uint32_t min_h = minimum(mcu_h, max_y % mcu_h);

  // save the current image block size
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;
  max_x += xpos;
  max_y += ypos;

  // read each MCU block until there are no more
  while ( JpegDec.readSwappedBytes()) { // Swap byte order so the SPI buffer can be used

    // save a pointer to the image block
    pImg = JpegDec.pImage;

    // calculate where the image block should be drawn on the screen
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;  // Calculate coordinates of top left corner of current MCU
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
    else win_w = min_w;

    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
    else win_h = min_h;

    // copy pixels into a contiguous block
    if (win_w != mcu_w)
    {
      uint16_t *cImg;
      int p = 0;
      cImg = pImg + win_w;
      for (int h = 1; h < win_h; h++)
      {
        p += mcu_w;
        for (int w = 0; w < win_w; w++)
        {
          *cImg = *(pImg + w + p);
          cImg++;
        }
      }
    }

    // draw image MCU block only if it will fit on the screen
    if ( ( mcu_x + win_w) <= tft.width() && ( mcu_y + win_h) <= tft.height())
    {
      tft.pushRect(mcu_x, mcu_y, win_w, win_h, pImg);
    }

    else if ( ( mcu_y + win_h) >= tft.height()) JpegDec.abort();
  }
}


void drawJpeg(const char *filename, int xpos, int ypos) {

  // Open the named file (the Jpeg decoder library will close it after rendering image)
  //fs::File jpegFile = SPIFFS.open( filename, "r");    // File handle reference for SPIFFS
  //  File jpegFile = SD.open( filename, FILE_READ);  // or, file handle reference for SD library

  //if ( !jpegFile ) {
    //Serial.print("ERROR: File \""); Serial.print(filename); Serial.println ("\" not found!");
    //return;
  //}

  // Use one of the three following methods to initialise the decoder:
  //boolean decoded = JpegDec.decodeFsFile(jpegFile); // Pass a SPIFFS file handle to the decoder,
  //boolean decoded = JpegDec.decodeSdFile(jpegFile); // or pass the SD file handle to the decoder,
  boolean decoded = JpegDec.decodeFsFile(filename);  // or pass the filename (leading / distinguishes SPIFFS files)
  // Note: the filename can be a String or character array type
  if (decoded)  jpegRender(xpos, ypos);
  else Serial.println("Jpeg file format not supported!");
}



class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device Connected");
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device Disconnected");
    ESP.restart();
  }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
      if (fileType == 0) {
        fileType = value[0];
        return;
      }
    }

    if (receivedLength < expectedLength) {
      accumulatedData += value;
      receivedLength += value.length();
      
      if (receivedLength >= expectedLength) {
        
        
        const char* fileExtension;
        if (fileType == 1) {
          fileExtension = ".jpg";
        } else if (fileType == 2) {
          fileExtension = ".gif";
        } else {
          Serial.println("Unknown file type. Exiting.");
          return;
        }

        if (SPIFFS.exists(currentFilename)) {
          SPIFFS.remove(currentFilename);
        }

        currentFilename = "/image" + String(fileExtension);

        File file = SPIFFS.open(currentFilename.c_str(), FILE_WRITE);
        if (!file) {
          Serial.println("Failed to create file.");
          return;
        }

        file.write((uint8_t*)accumulatedData.c_str(), receivedLength);
        file.close();

        Serial.print("File saved as ");
        Serial.print(currentFilename);
        Serial.println(" on SD card.");

        fileType = 0;
        accumulatedData = "";
        receivedLength = 0;
        expectedLength = 0;
      }
    } else if (value.length() == 4) {
      accumulatedData = "";
      receivedLength = 0;
      expectedLength = 0;
      for (int i = 0; i < 4; ++i) {
        expectedLength = (expectedLength << 8) | (uint8_t)value[i];
      }
      Serial.print("Expecting ");
      Serial.print(expectedLength);
      Serial.println(" bytes of data.");
    }
  }
};

void setup() {
  Serial.begin(115200);

  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  bool formatted = SPIFFS.format();
  if(formatted){
    Serial.println("\n\nSuccess formatting");
   }else{
    Serial.println("\n\nError formatting");
   }

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_WHITE);
  gif.begin(BIG_ENDIAN_PIXELS);

  String deviceName = "TFT Display";
  BLEDevice::init(deviceName.c_str());
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                  CHARACTERISTIC_UUID,
                  BLECharacteristic::PROPERTY_READ |
                  BLECharacteristic::PROPERTY_WRITE |
                  BLECharacteristic::PROPERTY_NOTIFY  
                );
  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();


}

void loop() {
  
  fs::File rootDir = SPIFFS.open( "/", "r");  
  
    
  while (File file = rootDir.openNextFile()) {
    String filename = file.name();
    filename = "/" + filename;
    Serial.println(file.name());

    uint32_t currentFileSize = file.size();  // Get the current file size
    // Check if the file size is different from the last file size
    if (currentFileSize != lastFileSize) {
      tft.fillScreen(TFT_WHITE);
    }

    lastFileSize = currentFileSize;
   
    if (!file.isDirectory() && filename.endsWith(".jpg")) {
      // Your existing code to display JPEG
      String Jpgfilename = filename;
      char jpegs[Jpgfilename.length() + 1];
      Jpgfilename.toCharArray(jpegs, sizeof(jpegs));
      drawJpeg(jpegs, 0, 0);
      delay(1000);
    }
    
    else if (!file.isDirectory() && filename.endsWith(".gif")) {
      uint32_t gifSize = file.size();  // Get the size of the GIF

     file.seek(0); 
    
      Serial.printf("Available heap before allocation: %u bytes\n", ESP.getFreeHeap());
uint8_t *gifData = new uint8_t[gifSize];
Serial.printf("Available heap after allocation: %u bytes\n", ESP.getFreeHeap());

      if (gifData == nullptr) {
        Serial.println("Failed to allocate memory for GIF");
        continue;
      }
      
      uint32_t bytesRead = file.read(gifData, gifSize);
Serial.printf("Bytes read: %d\n", bytesRead);

// Verify if all bytes are read
if (bytesRead != gifSize) {
  Serial.println("Error: Number of bytes read doesn't match the file size");
  Serial.printf("Expected %d bytes but read %d bytes\n", gifSize, bytesRead);
  delete[] gifData;  // Don't forget to free the memory
  continue;
}



      if (gif.open(gifData, gifSize, GIFDraw)) {
        Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
        tft.startWrite();
        while (gif.playFrame(true, NULL)) {
          yield();
        }
        gif.close();
        tft.endWrite();
      } else {
        Serial.println("Failed to open GIF");
      }
      delete[] gifData;  
      
      
    }
    
    
  }
}
