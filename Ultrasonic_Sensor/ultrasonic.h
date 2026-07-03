/**
 * ============================================================================================
 * Ultrasonic Sensor Source Code
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
#include <math.h>

class UltraSonicSensor {

    public:
  
        // Member variables (Publicly accessible fields)
        int triggerPin;
        int echoPin;

        // Physics constants defined explicitly in strict SI units
        const float M_air = 0.0289647f; // Molar mass of dry air in kg/mol (SI Base unit)
        const float kappa = 1.4000000f; // Adiabatic index (dimensionless)
        const float R     = 8.3144626f; // Universal Gas Constant in J/(mol*K)

        /**
         * @brief The constructor initialization.
         * 
         * @param trig The hardware pin for the ultrasonic trigger.
         * @param echo The hardware pin for the ultrasonic echo.
         * @param temp The baseline environment temperature in Celsius.
         */
        UltraSonicSensor(int trig, int echo, float tempC=20) {
            // Assign parameters to member variables
            triggerPin = trig;
            echoPin = echo;

            // Configure hardware pins immediately at startup
            pinMode(triggerPin, OUTPUT);
            pinMode(echoPin, INPUT);

            // Compute physical parameters
            speedOfSound = sqrtf( kappa * R * (tempC + 273.15) / M_air );
        }
        /**
         * @brief Measures the flight time of an ultrasonic pulse.
         * 
         * Sends a 10-microsecond trigger pulse to the ultrasonic sensor and
         * measures the duration of the returning echo pulse using hardware timing.
         * 
         * @param timeout The maximum time to wait for the echo pulse in microseconds. 
         *                Defaults to 30000 µs (approx. 5 meters max distance).
         * @return `unsigned long` The pulse duration in microseconds, or 0 if a timeout occurs.
         */
        unsigned long ping(unsigned long timeout = 30000) {
            // Ensure clean trigger pulse
            digitalWrite(triggerPin, LOW);
            delayMicroseconds(2);
            
            // Trigger the sensor with a 10 microsecond pulse
            digitalWrite(triggerPin, HIGH);
            delayMicroseconds(10);
            digitalWrite(triggerPin, LOW);
            
            // Measure pulse duration in microseconds (returns 0 if timeout)
            return pulseIn(echoPin, HIGH, timeout);
        }
        /**
         * Converts a traveling time in microseconds to the corresponding distance in millimeters.
         */
        unsigned long timeToDistance(unsigned long microseconds) {
            return (unsigned long)(microseconds * speedOfSound / 2000.0f); // Returns distance in millimeters
        }
        /**
         * Makes a single distance measurement.
         * Returns distance in millimeters.
         */
        unsigned long distance() {
            return timeToDistance(ping());
        }
        /**
         * Returns the maximum distance out of multiple iterations.
         */
        unsigned long maxDistance(size_t iterations=100) {
            unsigned long current, value;
            for (size_t i = 0; i < iterations; i++)
            {
                current = ping();
                if (current > value) value = current;
            }
            return timeToDistance(value);
        }
};