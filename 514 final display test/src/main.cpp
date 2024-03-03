
void updateDisplay();
void startCountdown();

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <AccelStepper.h>
#include <Adafruit_NeoPixel.h>

#define OLED_RESET    -1
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);

// Define pin for LED strip and number of LEDs
#define LED_STRIP_PIN D6 // Adjust to your actual pin
#define NUM_LEDS      5 // Adjust based on your LED strip

// Initialize the LED strip
Adafruit_NeoPixel strip(NUM_LEDS, LED_STRIP_PIN, NEO_GRB + NEO_KHZ800);

// Define pins for the buttons
const int buttonUpPin = D9; // Adjust to your actual pin
const int buttonDownPin = D8; // Adjust to your actual pin
const int buttonConfirmPin = D7; // Adjust to your actual pin

// Define stepper motor control pins and settings
AccelStepper stepper(AccelStepper::FULL4WIRE, D0, D1, D2, D3);

unsigned int countdownValue = 5; // Starting countdown value
bool countdownActive = false;

void setup() {
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
    countdownValue = max(1U, countdownValue - 1);;
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

  // Countdown ends, trigger LED strip animation
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(153, 28, 155)); // Set the next pixel to purple
    strip.show();
    delay(100); // Adjust the speed of the "light up" effect here
  }

  // Turn off LEDs after animation
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0)); // Turn off pixel
  }
  strip.show();

  // Assuming the stepper motor represents 30 seconds in one full revolution
  stepper.move(2048); // Adjust based on your stepper motor's steps per revolution
  while(stepper.distanceToGo() != 0) {
    stepper.run();
  }

  countdownActive = false;
  countdownValue = 5; // Reset to default or based on user
}