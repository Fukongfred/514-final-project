// BLE server
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <stdlib.h>

// libraries for OLED display, stepper motor, and neopixel
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <AccelStepper.h>
#include <Adafruit_NeoPixel.h>

// BLE server setup--------------------------------------------------------------
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
unsigned long previousMillis = 0;
const long interval = 1000;
#define SERVICE_UUID        "cbc8b292-c579-417d-bdae-9219a6362a83"
#define CHARACTERISTIC_UUID "5a645de1-e060-40f5-9745-646e2b954b27"


// OLED display setup--------------------------------------------------------------
#define OLED_RESET    -1
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);

// Define pin for LED strip and number of LEDs ----------------------------------
#define LED_STRIP_PIN D6 // Adjust to your actual pin
#define NUM_LEDS      10 // Adjust based on your LED strip
// Initialize the LED strip
Adafruit_NeoPixel strip(NUM_LEDS, LED_STRIP_PIN, NEO_GRB + NEO_KHZ800);



// Stepper motor setup--------------------------------------------------------------
#define MOTOR_STEPS 2038 // Define the number of steps per revolution 
#define MOTOR_PIN1  D0
#define MOTOR_PIN2  D1
#define MOTOR_PIN3  D2
#define MOTOR_PIN4  D3
AccelStepper stepper(AccelStepper::FULL4WIRE, MOTOR_PIN1, MOTOR_PIN2, MOTOR_PIN3, MOTOR_PIN4);

// Variables for countdown --------------------------------------------------------
unsigned int countdownTime = 5; // default 5 seconds
bool countdownStarted = false;


// Button pin--------------------------------------------------------------
const int buttonUpPin = D9; // Adjust to your actual pin
const int buttonDownPin = D8; // Adjust to your actual pin
const int buttonConfirmPin = D7; // Adjust to your actual pin
// ------------------------------------------------------------------------



class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
    }
};

// TODO: add DSP algorithm functions here

void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE work!");

    // TODO: name your device to avoid conflictions
    BLEDevice::init("XIAO_ESP32S3_Fucong");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setValue("Device connected!");
    pService->start();
    // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("Waiting for a client connection to notify...");

    // Initialize the button pins -----------------------------------------------
    pinMode(buttonUpPin, INPUT_PULLUP);
    pinMode(buttonDownPin, INPUT_PULLUP);
    pinMode(buttonConfirmPin, INPUT_PULLUP);

    // Initialize the OLED display ------------------------------------------------
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);

    // Initialize the LED strip ----------------------------------------------------
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'

    // Initialize the stepper motor ------------------------------------------------
    stepper.setMaxSpeed(1000);
    stepper.setAcceleration(500);


}

void loop() {

    // Countdown setup with buttons
    if (!countdownStarted) {
        if (digitalRead(buttonUpPin) == LOW) {
        countdownTime++;
        Serial.print("Up");
        display.clearDisplay();
        display.setCursor(0,0);
        display.println("Get Up In:");
        display.println(countdownTime);
        display.display();
        } else if (digitalRead(buttonDownPin) == LOW) {
        countdownTime--;
        Serial.print("Down");
        display.clearDisplay();
        display.setCursor(0,0);
        display.println("Get Up In:");
        display.println(countdownTime);
        display.display();
        } else if (digitalRead(buttonConfirmPin) == LOW) {
        Serial.print("OK");
        countdownStarted = true;
        // delay(200); // Debounce
        }
    }
  

    else {

        int temp = countdownTime;
        while (temp > 0) {
            display.clearDisplay();
            display.setCursor(0,0);
            display.println("Get Up In:");
            display.println(temp);
            display.display();
            Serial.print("Get Up In: ");
            Serial.println(temp);
            temp--;

            delay(1000);
        }

        if (temp == 0) {
                        // Update stepper motor and LED strip as a visualization
            stepper.move(2048); // Adjust based on your stepper motor's steps per revolution
            while(stepper.distanceToGo() != 0) {
             stepper.run();
            }

            for(int j = 0; j < NUM_LEDS; j++) {
                strip.setPixelColor(j, strip.Color(153, 28, 155)); // Red color
                strip.show();
                delay(100); // Update LED strip in segments
            }

            // Send BLE signal to Device2
            if (deviceConnected) {
                pCharacteristic->setValue("Alarm");
                pCharacteristic->notify();
                Serial.println("Notify value: Alarm");
            }

            // Turn off LEDs after animation
            for (int i = 0; i < NUM_LEDS; i++) {
                strip.setPixelColor(i, strip.Color(0, 0, 0)); // Turn off pixel
                }
            strip.show();

            display.clearDisplay();
            display.setCursor(0,0);
            display.println("Wake UPPPP!!!!!");
            display.display();
            delay(2000); // Debounce

            // Reset for next countdown
            
            display.clearDisplay();
            display.setCursor(0,0);
            display.println("Get Up In:");
            display.println(countdownTime);
            display.display();
            countdownStarted = false;
            countdownTime = 5; // Reset to default or based on user input
        }

    }

    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);  // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising();  // advertise again
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
    delay(1000);
}
