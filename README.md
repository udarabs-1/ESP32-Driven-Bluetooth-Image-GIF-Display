# ESP32-Driven-Bluetooth-Image-GIF-Display
Use ESP32 &amp; ILI9488 LCD to show images and GIFs wirelessly. Easy Bluetooth control. Great for DIY fans and tech lovers.

Component List

1. ESP32 S3-N8R16: A powerful microcontroller with integrated Wi-Fi and Bluetooth capabilities, ideal for handling both the processing of images/GIFs and the wireless communication necessary for this project.
[https://s.click.aliexpress.com/e/_DcIaS7h]

2. LCD Module - TN ILI9488 No Touch: A high-quality LCD display known for its sharpness and color accuracy, serving as the output device for displaying the selected images and GIFs.
[https://www.aliexpress.us/item/1005003315474416.html?spm=a2g0n.detail.1000013.4.515e6412wfib4j&gps-id=storeRecommendH5&scm=1007.18500.300835.0&scm_id=1007.18500.300835.0&scm-url=1007.18500.300835.0&pvid=7ddec753-78b3-48ca-993a-ed8930ceae66&_t=gps-id%3AstoreRecommendH5&gatewayAdapt=4itemAdapt]

3. Jumper Wires: Used to establish the necessary connections between the ESP32 S3-N8R16 and the ILI9488 LCD module.

Instructions for image/GIF uploading Mobile APP

First wire the system with the circuit diagram attached.

Step 1: Install Arduino IDE
1. Visit the (https://www.arduino.cc/en/software) and download the Arduino IDE suitable for your operating system.
2. Install the Arduino IDE following the installation instructions specific to your OS.

Step 2: Add ESP32 board

1. Open the Arduino IDE.
2. Go to File> Preferences.
3. In the “Additional Boards Manager” URLs field, add the following URL: ‘https://dl.espressif.com/dl/package_esp32_index.json’.
4. Click OK.

Step 3: Install ESP32 Board Package

1. Go to Tools > Board > Boards Manager.
2. In the search box, type ‘ESP32’.
3. Install the ESP32 package by Espressif Systems.

Step 4: Select ESP32 S3 Dev Module

1. Go to Tools > Board.
2. Scroll through the list and select ESP32 S3 DEV Module.

Step 5: Install Required Libraries

1. Go to Sketch > Include Library> Manage Libraries.
2. Install the following libraries by searching for them one by one and clicking “Install”:
    - JPEGDecoder

Step 6: Add TFT_eSPI, Animated GIF Library Externally

1. I have given the TFT_eSPI, animated gif library.
2. Unzip them and add that library inside Documents > Arduino> Libraries.


Step 7: Upload the Code

1. Open the Arduino Coe.
2. Connect the ESP32 S3 module to your computer using a USB cable from the USB port of the microcontroller.
3. Select the COM port by going to Tools > Port, and selecting the port where the ESP32 S3 is connected.
4. Connect GPIO 0 with GND using a jumper.
5. Click the Upload button in the Arduino IDE.
6. After uploading remove the jumper.
7. Click reset.  



