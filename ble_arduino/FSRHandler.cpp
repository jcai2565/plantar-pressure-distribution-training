#include "FSRHandler.hpp"

// === CONFIG ===
const float Vcc = 3.3; // volts
const float Rm = 10000.0; // ohms

// According to the numberings on the diagram in paper
const int fsrPins[NUM_SENSORS] = {4, 2, 35, 34, 32, 15};
const int redLEDs[NUM_SENSORS] = {0, 0, 0, 0, 0, 0}; // unused
const int greenLEDs[NUM_SENSORS] = {0, 0, 0, 0, 0, 0}; // set later!
float pressureState[NUM_SENSORS] = {0, 0, 0, 0, 0, 0}; // Initialized to 0

void initFSRsAndLEDs()
{
  for (int i = 0; i < NUM_SENSORS; i++)
  {
    pinMode(fsrPins[i], INPUT);
    pinMode(redLEDs[i], OUTPUT);
    pinMode(greenLEDs[i], OUTPUT);
  }
}

// Using the real datasheet, fit a power law
float resistanceToForce(float fsrResistance_kOhm) {
  float grams = pow(fsrResistance_kOhm / 153.18, -1.431);
  return grams / 1000.0 * 9.81; // convert to Newtons
}

// Given input analogPin, returns a force in Newtons
float readForce(int analogPin)
{
  int analogVal = analogRead(analogPin);
  float voltage = analogVal * (Vcc / 4095.0);
  if (voltage <= 0.0 || voltage >= Vcc)
    return 0;

  float fsrResistance_kOhm = ((Vcc - voltage) * Rm) / voltage / 1000.0;
  return resistanceToForce(fsrResistance_kOhm);
}

void updatePressureState()
{
  float rawForces[NUM_SENSORS];
  float totalForce = 0.0;

  for (int i = 0; i < NUM_SENSORS; i++)
  {
    rawForces[i] = readForce(fsrPins[i]);
    totalForce += rawForces[i];
  }

  for (int i = 0; i < NUM_SENSORS; i++)
  {
    if (totalForce > 0.0)
      pressureState[i] = rawForces[i] / totalForce;
    else
      pressureState[i] = 0.0;
  }
}

void updateLEDs(float force_N, int redPin, int greenPin)
{
  if (force_N < 3.5)
  {
    digitalWrite(greenPin, HIGH);
    digitalWrite(redPin, LOW);
  }
  else if (force_N > 6.5)
  {
    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, HIGH);
  }
  else
  {
    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, LOW);
  }
}

