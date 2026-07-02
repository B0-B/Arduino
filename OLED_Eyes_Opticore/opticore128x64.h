/**
 * ============================================================================================
 * OptiCore OLED Eyes Source Code
 * ============================================================================================
 * This library is intended for 128x64 oled (SSD1306) displays steered by Arduino-compatible 
 * MCUs, single-board computers, and other platforms with c++ runtimes and I2C bus. 
 * OptiCore leverages the Adafruit GFX library for fast and efficient rendering and is mainly 
 * targeted for small robotics- and DIY projects.
 * ============================================================================================
 * License and Copyright
 * ============================================================================================
 * Copyright (c) 2026 github.com/b0-b
 * This program is free software: you can redistribute it and/or modify it under the terms of 
 * the GNU General Public License as published by the Free Software Foundation, either version 
 * 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU General Public License for more details. You should have received a copy of the 
 * GNU General Public License along with this program. If not, see <https://gnu.org>.
 */

#include <Wire.h>
#include <math.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Hardcoded parameters
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define EYE_WIDTH 40
#define EYE_HEIGHT 40
#define EYE_RADIUS 10
#define EYE_ECCENTRICITY 1.0
#define EYE_GAP_BIAS 4
#define FRAME_DELAY_MS 5
#define LOOP_DELAY_MS 100



// Project-wide declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The parameter (-1) means the display shares the Arduino reset pin
inline Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


// ------------------------- Functions ----------------------------

/**
 * Generates a random number uniformly from the provided interval [a, b].
 */
inline long u (long a, long b) {
  // Makes the range [a, b] fully inclusive using pure integer math
  return random(a, b + 1);
}

/**
 * Generates an independent and identically distributed float between 0 and 1.  
 */
inline double iid () {
    // Divides a random number by the maximum possible random number
    return (double)random(0, LONG_MAX) / LONG_MAX;
}

/// @brief Creates a boolean response based on a random sampling experiment.
/// @param likelihood Sampling likelihood 
/// @return boolean
bool sample(float likelihood) {
  return iid() < likelihood;
}

// -------------------------------------------------------------------



// ------------------------- Eye Code ----------------------------
// Eye current state vars
int8_t  eyePosXL = 0, 
        eyePosYL = 0, 
        eyePosXR = 0, 
        eyePosYR = 0,
        browAngle = 0;
uint8_t eyeLidUL = 0,
        eyeLidDL = 0,
        eyeLidUR = 0,
        eyeLidDR = 0,
        browShift = 0;

// Eye state target vars
int8_t  eyePosTXL = 0, 
        eyePosTYL = 0, 
        eyePosTXR = 0, 
        eyePosTYR = 0,
        browAngleT = 0;
uint8_t eyeLidTUL = 0,
        eyeLidTDL = 0,
        eyeLidTUR = 0,
        eyeLidTDR = 0,
        browShiftT = 0,
        eyeLowerSquint = 0, 
        eyeLowerSquintT = 0;

float   eyeScale = 1,
        eyeScaleT = 1,
        transitionFactor = 0.4;

bool    curiosity = false;

// Base rendering functions

/**
 * Draws a basic eye with x and y coordinates, a scale and upper and lower eye lids.
 */
inline void drawBaseEye (int8_t x, int8_t y, float scale=1, uint8_t upperEyeLid=0, uint8_t lowerEyeLid=0, uint8_t lowerSquint=0, uint8_t _browShift=0, int8_t _browAngle=0) {
  // Draw round rect
  const uint8_t height = scale * ( EYE_HEIGHT - upperEyeLid - lowerEyeLid);
  uint8_t radius = (height < 2 * scale * EYE_RADIUS) ? max(0, height / 2 - 1) : scale * EYE_RADIUS;
  display.fillRoundRect(  x, 
                          y + upperEyeLid * scale, 
                          EYE_WIDTH * scale, 
                          EYE_HEIGHT * scale - lowerEyeLid * scale, 
                          radius, 
                          1); // Draw white
  if (lowerSquint) {
    display.fillRoundRect(  x, 
                            y + upperEyeLid * scale + height - lowerSquint, 
                            EYE_WIDTH * scale, 
                            EYE_HEIGHT * scale - lowerEyeLid * scale, 
                            radius, 
                            0); // Draw black
  }
  if (_browAngle || _browShift) {
    const uint8_t browWidth  = 1.5 * scale * EYE_WIDTH,
                  browHeight = scale * EYE_HEIGHT / 2 + abs(sin(_browAngle) * scale * EYE_WIDTH/2);
    display.fillRotatedRect(x + scale * EYE_WIDTH / 2, 
                            y - browHeight / 2 + _browShift,
                            browWidth, 
                            browHeight,
                            _browAngle, 
                            0);
  }
}

inline void drawLeftEye () {
  // Compute eccentricity
  float ecc = curiosity ? EYE_ECCENTRICITY * 2 * eyePosXR / SCREEN_WIDTH : 0;
  const uint8_t x = eyePosXL + 32 - (EYE_WIDTH * (eyeScale - ecc) * 0.5) + EYE_GAP_BIAS,
                y = eyePosYL + 32 - (EYE_HEIGHT * (eyeScale - ecc) * 0.5);
  drawBaseEye(x,
              y, 
              (eyeScale - ecc), 
              eyeLidUL, 
              eyeLidDL, 
              eyeLowerSquint,
              browShift,
              browAngle
  );
}

inline void drawRightEye () {
  float ecc = curiosity ? EYE_ECCENTRICITY * 2 * eyePosXR / SCREEN_WIDTH : 0;
  const uint8_t x = eyePosXR + 96 - (EYE_WIDTH * (eyeScale + ecc) * 0.5) - EYE_GAP_BIAS,
                y = eyePosYR + 32 - (EYE_HEIGHT * (eyeScale + ecc) * 0.5);
  drawBaseEye(x, 
              y, 
              (eyeScale + ecc), 
              eyeLidUR, 
              eyeLidDR, 
              eyeLowerSquint,
              browShift,
              -browAngle);
}

void drawEyes () {
  drawLeftEye();
  drawRightEye();
}

/**
 * Sets new target for the eyes to move to.
 * Afterwards call eyesTransition() repeatedly until inTransition variable is false again.
 */
inline void setTarget (int8_t xl, 
                int8_t yl, 
                int8_t xr, 
                int8_t yr, 
                float s, 
                uint8_t lidDL=0, 
                uint8_t lidDR=0, 
                uint8_t lidUL=0, 
                uint8_t lidUR=0, 
                uint8_t squint=0,
                uint8_t _browShift=0, 
                int8_t _browAngle=0) {
  eyePosTXL = xl, 
  eyePosTYL = yl, 
  eyePosTXR = xr, 
  eyePosTYR = yr,
  eyeLidTUL = lidUL,
  eyeLidTDL = lidDL,
  eyeLidTUR = lidUR,
  eyeLidTDR = lidDR,
  eyeScaleT = s;
  eyeLowerSquintT = squint;
  browShift = _browShift;
  browAngleT = _browAngle;
}

/**
 * Nudges a global float value towards its target value.
 */
inline void nudgeFloat (float *current, float target) { 
  const float tol = 0.02; 
  const float diff = target - *current;
  if (fabsf(diff) > tol) { 
      *current += diff * transitionFactor; 
  } else { 
      *current = target; 
  } 
}

/**
 * Nudges a global integer value towards its target value.
 */
inline void nudgeInt (int8_t *current, int8_t target) { 
  const int8_t diffLim = (int8_t)(1./transitionFactor);
  if ( *current > target ) {
      int8_t diff = *current - target;
      // Divide by 2, but ensure a minimum step of 1
      *current -= (diff > diffLim) ? (diff * transitionFactor) : 1;
  } else if ( *current < target ) {
      int8_t diff = target - *current;
      // Divide by 2, but ensure a minimum step of 1
      *current += (diff > diffLim) ? (diff * transitionFactor) : 1;
  }
}

inline void nudgeUint (uint8_t *current, uint8_t target) { 
  // Changed all internal helper variables to uint8_t
  const uint8_t diffLim = (uint8_t)(1.0f / transitionFactor);
  if ( *current > target ) {
      uint8_t diff = *current - target;
      *current -= (diff > diffLim) ? (uint8_t)(diff * transitionFactor) : 1;
  } else if ( *current < target ) {
      uint8_t diff = target - *current;
      *current += (diff > diffLim) ? (uint8_t)(diff * transitionFactor) : 1;
  }
}

/**
 * Checks if the eye transition was completed.
 */
inline bool transitionCompleted () {
  return eyePosXL == eyePosTXL && 
      eyePosYL == eyePosTYL && 
      eyePosXR == eyePosTXR && 
      eyePosYR == eyePosTYR && 
      eyeScale == eyeScaleT && 
      eyeLidDL == eyeLidTDL && 
      eyeLidDR == eyeLidTDR && 
      eyeLidUL == eyeLidTUL && 
      eyeLidUR == eyeLidTUR &&
      eyeLowerSquint == eyeLowerSquintT;
}

/**
 * @brief Animates a fluid visual transition of the eyes to a newly specified state.
 * 
 * Geometrically interpolates or updates the structural coordinates, scale, and lid positions 
 * of both eyes concurrently to morph the facial expression based on the target parameters.
 * 
 * @param xl         Target horizontal gaze offset (X-axis) for the left eye.
 * @param yl         Target vertical gaze offset (Y-axis) for the left eye.
 * @param xr         Target horizontal gaze offset (X-axis) for the right eye.
 * @param yr         Target vertical gaze offset (Y-axis) for the right eye.
 * @param scale      Overall size multiplier for the eyes. Defaults to 1.0 (normal size).
 * @param lidDL      Lower eyelid closure depth for the left eye (0 = wide open). Defaults to 0.
 * @param lidDR      Lower eyelid closure depth for the right eye (0 = wide open). Defaults to 0.
 * @param lidUL      Upper eyelid closure depth for the left eye (0 = wide open). Defaults to 0.
 * @param lidUR      Upper eyelid closure depth for the right eye (0 = wide open). Defaults to 0.
 * @param squint     Squint intensity modifier for both eyes simultaneously. Defaults to 0.
 * @param _browShift Vertical positioning offset for the eyebrows. Defaults to 0.
 * @param _browAngle Rotational angular tilt for the eyebrows to convey emotion. Defaults to 0.
 */
inline void eyesTransition (int8_t xl, 
                    int8_t yl, 
                    int8_t xr, 
                    int8_t yr, 
                    float scale=1.0, 
                    uint8_t lidDL=0, 
                    uint8_t lidDR=0, 
                    uint8_t lidUL=0, 
                    uint8_t lidUR=0, 
                    uint8_t squint=0, 
                    uint8_t _browShift=0, 
                    int8_t _browAngle=0) {
  // Set new global eye target
  setTarget(xl, yl, xr, yr, scale, lidDL, lidDR, lidUL, lidUR, squint, _browShift, _browAngle);
  // Nudge all parameters and delay frames
  while (!transitionCompleted())
  {
    // Normalize each eye position
    nudgeInt(&eyePosXL, eyePosTXL);
    nudgeInt(&eyePosXR, eyePosTXR);
    nudgeInt(&eyePosYL, eyePosTYL);
    nudgeInt(&eyePosYR, eyePosTYR);
    // Normalize scales
    nudgeFloat(&eyeScale, eyeScaleT);
    // Normalize eye lids
    nudgeUint(&eyeLidDL, eyeLidTDL);
    nudgeUint(&eyeLidDR, eyeLidTDR);
    nudgeUint(&eyeLidUL, eyeLidTUL);
    nudgeUint(&eyeLidUR, eyeLidTUR);
    // Create squint
    nudgeUint(&eyeLowerSquint, eyeLowerSquintT);
    // Move brow
    nudgeUint(&browShift, browShiftT);
    nudgeInt(&browAngle, browAngleT);
    // Draw
    display.clearDisplay();
    drawEyes();
    display.display();
    delay(FRAME_DELAY_MS);
  }
}

inline void eyesNormalize () {
  eyesTransition(0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0);
}

// Animations

inline void blink () {
  const uint8_t TDL = eyeLidTDL, 
                TDR = eyeLidTDR, 
                TUL = eyeLidTUL, 
                TUR = eyeLidTUR;
  const uint8_t trueHeightL = eyeScale * (EYE_HEIGHT - TDL - TUL), 
                trueHeightR = eyeScale * (EYE_HEIGHT - TDR - TUR);
  const uint8_t sL = trueHeightL / 2 - 2, 
                sR = trueHeightR / 2 - 2;
  eyesTransition(eyePosXL, eyePosYL, eyePosXR, eyePosYR, eyeScale, sL, sR, sL, sR);
  delay(20);
  eyesTransition(eyePosXL, eyePosYL, eyePosXR, eyePosYR, eyeScale, TDL, TDR, TUL, TUR);
}

inline void lookAround () {
  if (iid() < .05) {Serial.println("blink"); blink();}
  int8_t x = u(-15, 15);
  int8_t y = u(-12, 12);
  eyesTransition(x, y, x, y, 1);
  delay(u(1000, 4000));
}

inline void leery () {
  eyesNormalize();
  curiosity = true;
  const size_t maxTransitions = 10;
  for (size_t i = 0; i < maxTransitions; i++){
    if (iid() < .05) {Serial.println("blink"); blink();};
    int8_t x = u(-10, 10), y = u(-10, 10);
    uint8_t lids = u(6, 14);
    eyesTransition(x, y, x, y, 1, lids, lids, lids, lids, 0, u(10, 25));
    delay(u(400, 2000));
    if (iid() < 0.05) break;
  }
  eyesNormalize();
  curiosity = false;
}

inline void happy () {
  eyesNormalize();
  eyesTransition(0, 5, 0, 5, 0.8, 0, 0, 0, 0);
  eyesTransition(0, 5, 0, 5, 0.8, 0, 0, 0, 0, 20);
  // Delay 
  delay(u(1000, 3000));
  // Look around a bit
  for (size_t i = 0; i < 5; i++)
  {
    int8_t x = u(-15, 15);
    int8_t y = u(-12, 12);
    int8_t s = u(18, 22);
    eyesTransition(x, y, x, y, 0.8, 0, 0, 0, 0, s);
    delay(u(1000, 2000));
    if (iid() < .05) break;
  }
  eyesNormalize();
}

inline void sad () {
  eyesNormalize();
  eyesTransition(0, 10, 0, 10, 1.15, 0, 0, 0, 0, 0, 15, -20);
  delay(u(1000, 3000));
  uint8_t x, y;
  float scale;
  for (size_t i = 0; i < 4; i++)
  {
    x = u(-15, 15);
    y = u(10, 15);
    eyesTransition(x, y, x, y, iid() * 0.2 + 1.05, 0, 0, 0, 0, 0, u(15, 20), u(-20, -30));
    delay(u(1000, 2000));
    if (iid() < 0.05) break;
  }
}

inline void bored () {
  eyesTransition(0, 10, 0, 10, 1.15, 0, 0, 0, 0, 0, 15, 0);
  delay(u(1000, 3000));
  uint8_t x, y;
  float scale;
  for (size_t i = 0; i < 8; i++)
  {
    x = u(-10, 10);
    y = u(5, 15);
    eyesTransition(x, y, x, y, iid() * 0.1 + .95, 0, 0, 0, 0, 0, u(20, 30), 0);
    delay(u(1000, 2000));
    if (iid() < 0.05) break;
  }
}

inline void angry () {
  eyesNormalize();
  eyesTransition(0, 10, 0, 10, 1.15, 0, 0, 0, 0, 0, 15, 20);
  delay(u(1000, 3000));
  uint8_t x, y, shift; 
  int8_t  angle;
  float scale;
  for (size_t i = 0; i < 4; i++)
  {
    x = u(-10, 10);
    y = u(-10, 0);
    shift = u(25, 35);
    angle = u(25, 40);
    scale = iid() * 0.2 + 1.05;
    eyesTransition(x, y, x, y, scale, 0, 0, 0, 0, 0, shift, angle);
    delay(u(1000, 2000));
    const double _u = iid();
    // Sometimes exit early
    if (_u < 0.05) break;
    // Sometimes (but more likely) shake from rage
    else if (_u < .25) {
      // Shake from rage
      uint8_t x_direction = 2, y_direction = 1, rounds = u(6, 10);
      for (size_t j = 0; j < rounds; j++)
      {
        x_direction *= -1;
        y_direction *= -1;
        eyesTransition(x + x_direction, y + y_direction, x + x_direction, y + y_direction, scale, 0, 0, 0, 0, 0, shift, angle);
      }
    }
  }
}

inline void stunned () {
  int8_t ecc = 2;
  eyesNormalize();
  eyesTransition(ecc, 5, ecc, 5, 1.3, 0, 0, 0, 0);
  eyesTransition(-ecc, 5, -ecc, 5, 1.3, 0, 0, 0, 0);
  eyesTransition(ecc, 5, ecc, 5, 1.3, 0, 0, 0, 0);
  eyesTransition(-ecc, 5, -ecc, 5, 1.3, 0, 0, 0, 0);
  eyesTransition(ecc, 5, ecc, 5, 1.3, 0, 0, 0, 0);
  eyesTransition(-ecc, 5, -ecc, 5, 1.3, 0, 0, 0, 0);
  eyesTransition(ecc, 5, ecc, 5, 1.3, 0, 0, 0, 0);
  eyesTransition(-ecc, 5, -ecc, 5, 1.3, 0, 0, 0, 0);
  eyesTransition(ecc, 5, ecc, 5, 1.3, 0, 0, 0, 0);
  eyesTransition(-ecc, 5, -ecc, 5, 1.3, 0, 0, 0, 0);
  eyesTransition(ecc, 5, ecc, 5, 1.3, 0, 0, 0, 0);
  eyesTransition(-ecc, 5, -ecc, 5, 1.3, 0, 0, 0, 0);
  eyesNormalize();
}

// -------------------------------------------------------------------



// ------------------------- Screensavers ----------------------------
inline void screensaverCube () {
  // 1. Static 3D Cube Vertices (X, Y, Z)
  static const float vertices[8][3] = {
    {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, {-1,  1, -1},
    {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1}
  };

  // 2. Persistent Rotation Angles
  static float angleX = 0, angleY = 0, angleZ = 0;
  
  // 3. Increment angles for continuous movement
  angleX += 0.05;
  angleY += 0.04;
  angleZ += 0.03;

  // 4. Pre-calculate Trigonometric values
  float sx = sin(angleX), cx = cos(angleX);
  float sy = sin(angleY), cy = cos(angleY);
  float sz = sin(angleZ), cz = cos(angleZ);

  // Array to hold the translated 2D coordinates (X, Y)
  int proj[8][2];

  // 5. Matrix Rotation and 3D to 2D Projection
  for (int i = 0; i < 8; i++) {
    float x = vertices[i][0];
    float y = vertices[i][1];
    float z = vertices[i][2];

    // Apply combined rotation matrix
    float rx = x * cy * cz - y * cy * sz + z * sy;
    float ry = x * (sx * sy * cz + cx * sz) + y * (cx * cz - sx * sy * sz) - z * sx * cy;
    float rz = x * (sx * sz - cx * sy * cz) + y * (cx * sy * sz + sx * cz) + z * cx * cy;

    // Perspective transformation calculation
    float distance = 3.5;          // Camera distance from the cube
    float zed = rz + distance;     // Z-depth scalar
    
    // Scale and shift coordinates to fit the 128x64 display center
    proj[i][0] = (int)(64 + (rx * 60 / zed));
    proj[i][1] = (int)(32 + (ry * 60 / zed));
  }

  // 6. Draw the Frame
  // display.clearDisplay();

  // Connect lines between the vertices to form edges
  for (int i = 0; i < 4; i++) {
    // Draw front face square edges
    display.drawLine(proj[i][0], proj[i][1], proj[(i + 1) % 4][0], proj[(i + 1) % 4][1], SSD1306_WHITE);
    // Draw back face square edges
    display.drawLine(proj[i + 4][0], proj[i + 4][1], proj[4 + ((i + 1) % 4)][0], proj[4 + ((i + 1) % 4)][1], SSD1306_WHITE);
    // Draw cross-connecting parallel edges
    display.drawLine(proj[i][0], proj[i][1], proj[i + 4][0], proj[i + 4][1], SSD1306_WHITE);
  }

  // Push buffer to the actual hardware
  // display.display();
  // delay(15);
}

/**
 * A demonstration of eye movements: mainly looks around and samples random moods from time to time.
 * The function can be called in a loop for continuous flow.
 */
inline void demo () {
  double _u = iid();

  // Decide what to display
  if (iid() < .01) {
      Serial.println("sad");
      bored();
  }
  else if (iid() < .03) {
      Serial.println("blink");
      blink();
  }
  else if (iid() < .06) {
      Serial.println("happy");
      happy();
  }
  else if (iid() < .11) {
      Serial.println("bored");
      bored();
  }
  else if (iid() < .16) {
      Serial.println("leery");
      leery();
  }
  else if (iid() < .31) {
      Serial.println("blink");
      blink();
  }
  else {
      Serial.println("lookaround");
      lookAround();
  }

  // Frame delay
  delay(LOOP_DELAY_MS);
}



// ------------------------- External Initialization ----------------------------
/**
 * @brief Initializes the OptiCore system hardware components.
 * 
 * Configures and starts the Serial interface at the specified baud rate,
 * sets up the I2C bus with the provided custom SDA and SCL pins, and
 * establishes communication with the SSD1306 OLED display.
 * 
 * @param sda_pin   The GPIO pin number allocated for the I2C Data (SDA).
 * @param scl_pin   The GPIO pin number allocated for the I2C Clock (SCL).
 * @param baud_rate Optional serial communication speed in bits per second. 
 *                  Defaults to 115200 if omitted.
 */
inline void optiCoreInit(uint8_t sda_pin, uint8_t scl_pin, uint32_t baud_rate=115200) {
  // Initialize serial communication for debugging
  Serial.begin( baud_rate );
  // Initialize I2C with defined pins
  Wire.begin(sda_pin, scl_pin);
  // Initialize the OLED display. 
  // 0x3C is the most common I2C address for these screens. Change to 0x3D if it fails.
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Clear the buffer
  display.clearDisplay();
  // Set text properties
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
}