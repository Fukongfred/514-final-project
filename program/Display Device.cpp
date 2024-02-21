#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <AccelStepper.h>
#include <Adafruit_NeoPixel.h>

// WiFi network settings
const char* ssid     = "YourSSID";
const char* password = "YourPassword";

// Host settings (adjust the IP address according to your ESP32S3 device's IP)
const char* host = "192.168.1.XXX"; // ESP32S3 IP address
const int httpPort = 80;

#define OLED_RESET    -1
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);

// Define pin for LED strip and number of LEDs
#define LED_STRIP_PIN 6 // Adjust to your actual pin
#define NUM_LEDS      10 // Adjust based on your LED strip

// Initialize the LED strip
Adafruit_NeoPixel strip(NUM_LEDS, LED_STRIP_PIN, NEO_GRB + NEO_KHZ800);

// Define pins for the buttons
const int buttonUpPin = 7; // Adjust to your actual pin
const int buttonDownPin = 8; // Adjust to your actual pin
const int buttonConfirmPin = 9; // Adjust to your actual pin

// Define stepper motor control pins and settings
AccelStepper stepper(AccelStepper::FULL4WIRE, 0, 1, 2, 3);

int countdownValue = 20; // Starting countdown value
bool countdownActive = false;

void setup() {
  Serial.begin(115200);

  // Initialize WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize the OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);

  // Initialize the button pins
  pinMode(buttonUpPin, INPUT_PULLUP);
  pinMode(buttonDownPin, INPUT_PULLUP);
  pinMode(buttonConfirmPin, INPUT_PULLUP);

  // Initialize the LED strip
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  // Initialize the stepper motor
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);

  // Display the initial countdown value
  updateDisplay();
}

void loop() {
  if (digitalRead(buttonUpPin) == LOW) {
    countdownValue++;
    updateDisplay();
    delay(200); // Debounce delay
  }
  if (digitalRead(buttonDownPin) == LOW) {
    countdownValue--;
    updateDisplay();
    delay(200); // Debounce delay
  }

  if (digitalRead(buttonConfirmPin) == LOW && !countdownActive) {
    countdownActive = true;
    startCountdown();
  }
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Get up in ");
  display.print(countdownValue);
  display.display();
}

void startCountdown() {
  for(int i = countdownValue; i >= 0; i--) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("Get up in ");
    display.print(i);
    display.display();
    delay(1000); // 1 second delay
  }

  // Here, we send the signal to the ESP32S3 device to start the buzzer
  sendSignalToEndCountdown();

  // Continue with your existing countdown end actions...
}

void sendSignalToEndCountdown() {
  Serial.print("Connecting to ");
  Serial.print(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, httpPort)) {
    Serial.println(" connection failed");
    return;
  }

  // This will send the request to the ESP32S3
  client.println("GET /triggerBuzzer HTTP/1.1");
  client.println("Host: " + String(host));
  client.println("Connection: close");
  client.println();

  while(client.connected() && !client.available()) delay(1); // Wait for the server's response

  while(client.available()){
    String line =
