/**
 * ============================================================================================
 * Servo motor code.
 * ============================================================================================
 * This library is intended to steer an ultrasonic sensor for precise distance measurements.
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

#include <Arduino.h>

class Servo
{
private:
  /* data */
  uint8_t _pin;
  uint16_t _angleRange;
  uint32_t _freq, _writeRange;
  float _period, _minDuty, _maxDuty;
public:
  /**
   * @brief Initializes the Servo object and configures ESP32 hardware PWM settings.
   * 
   * Calculates the operational duty cycle parameters, sets up bit-shifted resolution boundaries, 
   * and configures the hardware timer frequencies directly for the target GPIO pin.
   * 
   * @param pin           GPIO pin number assigned to the servo motor.
   * @param frequencyHz   PWM signal frequency in Hertz (default: 50 Hz for standard servos).
   * @param resolution    Hardware bit depth for duty cycle steps (default: 10-bit / 0-1023 steps).
   * @param minDuty       Minimum pulse width duration in seconds (default: 0.001s / 1ms for 0°).
   * @param maxDuty       Maximum pulse width duration in seconds (default: 0.005s / 5ms for max angle).
   * @param angleRange    Total physical mechanical movement spectrum in degrees (default: 180°).
   */
  Servo(uint8_t pin, uint32_t frequencyHz=50, uint32_t resolution=10, float minDuty=0.001, float maxDuty = 0.005, uint16_t angleRange=180) {
    _pin = pin;
    _freq = frequencyHz;
    _period = 1.0f / (float)_freq; // full period in seconds
    _minDuty = minDuty;
    _maxDuty = maxDuty;
    _angleRange = angleRange;
    _writeRange = (1 << resolution) - 1; // bit shifts for power calculus
    // Main methods to set hardware PWM resolution
    analogWriteFrequency(_freq);
    analogWriteResolution(resolution); // resolution of 10 is 2^10 -> 0-1023
    // Some ESPs and RP2040 compilers use this alternative naming below 
    // (uncomment if needed and comment out the upper two lines):
    // analogWriteRange((1 << resolution) - 1);
    // analogWriteFreq(PWM_FREQ_HZ);
  };
  /**
   * @brief Sets the position of a servo motor by calculating and writing the appropriate PWM duty cycle.
   * 
   * This function maps a target angle (0 to 180 degrees) to a corresponding pulse-width duration 
   * based on minimum and maximum duty cycle limits. It then converts this duration into a discrete 
   * integer value proportional to the hardware's timer resolution (`writeRange`) and updates the PWM output.
   * 
   * @param angle The target servo position in degrees. Clamped automatically between 0.0f and 180.0f.
   * @note This function relies on a globally defined `SERVO_PIN` constant and the `analogWrite()` framework function.
   */
  void set (float angle) {
    if (angle < 0.0f) angle = 0.0f;
    else if (angle > _angleRange) angle = _angleRange;
    const float T = 1.0f / (float)_freq; // full period in seconds
    float dutyCycle = (_maxDuty - _minDuty) * (angle / _angleRange) + _minDuty; // seconds
    // Map seconds duty cycle onto the writeRange
    int decimalValue = (int)roundf( (float)_writeRange * dutyCycle / T );
    // Write the exact microsecond duration directly to the hardware
    analogWrite(_pin, decimalValue);
  }
};