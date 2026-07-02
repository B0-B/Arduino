// Import the OptiCore library
#include <opticore128x64.h>

#define BAUD_RATE 115200
// ESP32-WROOM-32 pinout example
#define SDA_PIN 21
#define SCL_PIN 22

void setup() {

    // Start serial connection (optional)
    Serial.begin( BAUD_RATE );

    // Initialize the OptiCore library, this will load the Adafruit_SSD1306 on the provided pins
    optiCoreInit(SDA_PIN, SCL_PIN, BAUD_RATE); 
    
    // Now use the Adafruit library with the classic display variable
    for (size_t i = 3; i > 0; i--) {
        display.clearDisplay();
        display.setCursor(5, 20); // Always set cursor before printing
        display.print("Running OptiCore\nDemonstration in " + String (i));
        display.display();
        delay(1000);
    }
}

void loop() {
    // Loop the demo in the main loop
    demo();
}