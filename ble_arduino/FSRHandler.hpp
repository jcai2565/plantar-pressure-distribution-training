#ifndef FSR_HANDLER_HPP
#define FSR_HANDLER_HPP

#include <Arduino.h>

// Number of sensors
const int NUM_SENSORS = 6;

// Global constants
extern const float Vcc;
extern const float Rm;

// Pin arrays (set your GPIOs here)
extern const int fsrPins[NUM_SENSORS];
extern const int redLEDs[NUM_SENSORS];
extern const int greenLEDs[NUM_SENSORS];

extern float pressureState[NUM_SENSORS]; // Stores normalized pressure values
void updatePressureState();              // Updates pressureState array

// Setup functions
void initFSRsAndLEDs();

// Read and convert force
float resistanceToForce(float fsrResistance_kOhm);
float readForce(int analogPin);

// LED state handler
void updateLEDs(float force_N, int redPin, int greenPin);

#endif
