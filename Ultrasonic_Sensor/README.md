<h1 align="center">UltraSonicSensor Library</h1>

A lightweight, high-precision C++ class for Arduino that utilizes the **Newton-Laplace thermodynamic equation** to calculate distances with microsecond timing. By computing the exact speed of sound via the Ideal Gas Law, this class ensures precise millimeter measurements across variable environmental temperatures.

---

## 🚀 Getting Started

### 1. Installation
1. Create a file named `UltraSonicSensor.h` inside your Arduino project directory.
2. Copy and paste your class code directly into that file.
3. Include it at the top of your main `.ino` sketch:
   ```cpp
   #include "UltraSonicSensor.h"
   ```

### 2. Hardware Wiring (Example: HC-SR04)
* **VCC** $\rightarrow$ 5V / 3.3V
* **GND** $\rightarrow$ GND
* **Trig** $\rightarrow$ Arduino Pin 15 (or any digital output)
* **Echo** $\rightarrow$ Arduino Pin 2 (or any digital input)

---

## 📄 Function Definitions

### `UltraSonicSensor(int trig, int echo, float tempC = 20)`
* **Description**: Constructor that initializes the hardware pins and computes the environmental speed of sound baseline.
* **Parameters**:
  * `trig` *(int)*: The hardware pin connected to the sensor's Trigger.
  * `echo` *(int)*: The hardware pin connected to the sensor's Echo.
  * `tempC` *(float, optional)*: Current air temperature in Celsius. Defaults to `20`°C.

### `unsigned long ping(unsigned long timeout = 30000)`
* **Description**: Sends a 10-microsecond trigger pulse and returns raw microsecond flight time.
* **Parameters**: 
  * `timeout` *(unsigned long, optional)*: Max wait time in microseconds. Defaults to `30000` µs (~5m range).
* **Returns**: Round-trip duration in microseconds (`0` if timeout).

### `unsigned long timeToDistance(unsigned long microseconds)`
* **Description**: Converts raw flight time into a one-way distance using the calibrated speed of sound.
* **Returns**: Calculated distance in **millimeters**.

### `unsigned long distance()`
* **Description**: Shortcut wrapper that executes a single `ping()` and converts it immediately.
* **Returns**: Distance in **millimeters**.

### `unsigned long maxDistance(size_t iterations = 100)`
* **Description**: Loops multiple sensor pings and captures the longest flight duration to filter out localized dropouts or noise blocks.
* **Returns**: Maximum verified distance in **millimeters**.

---

## 💻 Usage Examples

### Example 1: Basic Reading (Standard Room Temp)
```cpp
#include <Arduino.h>
#include "UltraSonicSensor.h"

// Instantiate sensor on pins 15 and 2 with default 20°C calibration
UltraSonicSensor sonar(15, 2);

void setup() {
    Serial.begin(115200);
}

void loop() {
    unsigned long mm = sonar.distance();
    Serial.print("Distance: " + String(mm) + "mm\r");
    delay(60); // Small cooldown between pings
}
```

### Example 2: Freezing Environment Integration
```cpp
#include <Arduino.h>
#include "UltraSonicSensor.h"

// Instantiate sensor calibrated for cold winter weather (-20°C)
UltraSonicSensor winterSonar(15, 2, -20.0f);

void setup() {
    Serial.begin(115200);
    
    // Read the calculated thermodynamic velocity back
    Serial.print("Calibrated Speed of Sound: ");
    Serial.print(winterSonar.speedOfSound);
    Serial.println(" m/s");
}

void loop() {
    // Get the maximum distance from a sample array of 10 pings
    unsigned long filteredMM = winterSonar.maxDistance(10);
    
    Serial.print("Max Peak Distance: " + String(filteredMM) + "mm\r");
    delay(100);
}
```

---

## 🔬 Physics Applied Under the Hood
The class calculates the localized speed of sound using the absolute thermodynamic property definitions:

$$speedOfSound = \sqrt{\frac{\kappa \cdot R \cdot (tempC + 273.15)}{M_{air}}}$$

* **$\kappa$ (Adiabatic Index)**: `1.4` (Diatomic gas behavior)
* **$R$ (Universal Gas Constant)**: `8.3144626` J/(mol·K)
* **$M_{air}$ (Molar Mass of Dry Air)**: `0.0289647` kg/mol
