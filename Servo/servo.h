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

/**
 * @brief Sets the position of a servo motor by calculating and writing the appropriate PWM duty cycle.
 * 
 * This function maps a target angle (0 to 180 degrees) to a corresponding pulse-width duration 
 * based on minimum and maximum duty cycle limits. It then converts this duration into a discrete 
 * integer value proportional to the hardware's timer resolution (`writeRange`) and updates the PWM output.
 * 
 * @param pin The servo GPIO pin number.
 * @param angle The target servo position in degrees. Clamped automatically between 0.0f and 180.0f.
 * @param writeRange The resolution/maximum value of the PWM timer (e.g., 65535 for 16-bit PWM). Defaults to 65535.
 * @param frequencyHz The operating frequency of the servo in Hertz. Defaults to 50Hz (standard 20ms period).
 * @param dutyMin The minimum pulse duration (in seconds) corresponding to 0 degrees. Defaults to 0.00105s (1.05ms).
 * @param dutyMax The maximum pulse duration (in seconds) corresponding to 180 or 360 degrees (depending on the servo). Defaults to 0.005s (5.0ms).
 * 
 * @note This function relies on a globally defined `SERVO_PIN` constant and the `analogWrite()` framework function.
 */
inline void setServoAngle (uint8_t pin, float angle, uint32_t writeRange = 65535, uint32_t frequencyHz=50, float dutyMin = 0.00105, float dutyMax = 0.005) {
  if (angle < 0.0f) angle = 0.0f;
  else if (angle > 180.0f) angle = 180.0f;
  const float T = 1.0f / (float)frequencyHz; // full period in seconds
  float dutyCycle = (dutyMax - dutyMin) * (angle / 180.0f) + dutyMin; // seconds
  // Map seconds duty cycle onto the writeRange
  int decimalValue = (int)roundf( (float)writeRange * dutyCycle / T );
  // Write the exact microsecond duration directly to the hardware
  analogWrite(pin, decimalValue);
}