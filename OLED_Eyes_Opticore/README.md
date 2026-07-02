<h1 align=center>OLED Eyes OptiCore</h1>

Adafruit GFX overlay API and library for rendering customized robot-like eyes on SSD1306 OLED displays. Comes with built-in emotions and a toolkit for custom eye animations and transitions. 

# Requirements
- Any device with a C++ runtime and I2C bus
- Suiting C++ compiler for your hardware with c++17 or later
- 128x64px OLED display with SSD1306 drivers (most common display on the market)
- [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library/blob/master/Adafruit_GFX.h) library and [SSD1306](https://github.com/adafruit/Adafruit_SSD1306/blob/master/Adafruit_SSD1306.h) driver library

# Usage

## Getting-Started 

### Arduino Demo
---

Copy the ``opticore128x64.h`` into your project. Within the same directory open/create ``main.cpp`` and add the following code - make sure to set the correct ``SDA`` and ``SCL`` GPIO pin numbers for your micro controller. The demo will infinitely run a random robotic eye demonstration. 

```c
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
```


## Library API

The optiCore provides a toolkit for creating custom animations or call pre-defined emotions and expressions separately.

### Built-In Emotions
---

You can call all emotions in a single sequence to explore them all

```c
happy();    // Will display a happy face
angry();    // Will display an angry face with rage jiggling
sad();      // Will display a sad face
bored();    // Will display a bored face
stunned();  // Will appear stunned
leery();    // Will be looking leery
```

### Custom API Functions
---

#### ``eyeNormalize`` 
---

Reverts eyes smoothly into the start position. Useful between transitions.
```c
eyesNormalize();
```

#### ``lookAround`` 
---

The standard lookAround function keeps the eyes in normal "emotionless" state and creates a random swipe. Ican be called either once for a single look swipe or in a loop for longer effect
```c
lookAround(); // single look
// or continuous looks
void loop {
    lookAround();
}
```

#### ``blink`` 
---

The standard blink animation, can be called once or randomly in the loop function to appear spontaneously. See the example below.

**Example**
```c
// Use the builtin sampling method to blink randomly from time to time
void loop () {
    // Blink with a 20% likelihood in this iteration
    if (sample(0.2)) blink();

    // Animation code here
    lookaround();
}
```

#### ``eyesTransition``
---
Create smooth custom transition with the `eyesTransition` function which nimates a fluid visual transition of the eyes to a newly specified state.

| Parameter | Type | Default | Description |
|---|---|---|---|
| xl | int8_t | None | Target horizontal gaze offset (X-axis) for the left eye. |
| yl | int8_t | None | Target vertical gaze offset (Y-axis) for the left eye. |
| xr | int8_t | None | Target horizontal gaze offset (X-axis) for the right eye. |
| yr | int8_t | None | Target vertical gaze offset (Y-axis) for the right eye. |
| scale | float | 1.0 | Overall size multiplier for the eyes (1.0 = normal size). |
| lidDL | uint8_t | 0 | Lower eyelid closure depth for the left eye (0 = wide open). |
| lidDR | uint8_t | 0 | Lower eyelid closure depth for the right eye (0 = wide open). |
| lidUL | uint8_t | 0 | Upper eyelid closure depth for the left eye (0 = wide open). |
| lidUR | uint8_t | 0 | Upper eyelid closure depth for the right eye (0 = wide open). |
| squint | uint8_t | 0 | Squint intensity modifier for both eyes simultaneously. |
| _browShift | uint8_t | 0 | Vertical positioning offset for the eyebrows. |
| _browAngle | int8_t | 0 | Rotational angular tilt for the eyebrows to convey emotion. |

**Example**

```c
// Transition to start position - alias for eyesNormalize
eyesTransition(0, 0, 0, 0);
// Make a happy face
eyesTransition(0, 5, 0, 5, 0.8, 0, 0, 0, 0, 20);
```





#### ``curiosity``
---

Set the global curiosity flag `curiosity=true` before starting a transition to enable automatic eyes scaling based on x-axis eccentricity. This will make the eyes appear more curious. Finally disable after the transition when not needed anymore

```c
// Look curious animation
curiosity = true;
int8_t eccentricity = 10; 
for (size_t i = 0; i < 5; i++) {
    eccentricity *= -1;
    eyesTransition(eccentricity, 0, eccentricity, 0);
    delay(200);
}
curiosity = false; 
```