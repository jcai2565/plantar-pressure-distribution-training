
#include "BLECStringCharacteristic.h"
#include "EString.h"
#include "RobotCommand.h"
#include <ArduinoBLE.h>
#include "FSRHandler.hpp"

//////////// BLE UUIDs ////////////
#define BLE_UUID_TEST_SERVICE "42a9cc13-e812-48e6-9dca-a4d8bc916dbe"
#define BLE_UUID_RX_STRING "9750f60b-9c9c-4158-b620-02ec9521cd99"
#define BLE_UUID_TX_FLOAT "27616294-3063-4ecc-b60b-3470ddef2938"
#define BLE_UUID_TX_STRING "f235a225-6735-4d73-94cb-ee5dfce9ba83"
//////////// BLE UUIDs ////////////

//////////// Global Variables ////////////
BLEService testService(BLE_UUID_TEST_SERVICE);
BLECStringCharacteristic rx_characteristic_string(BLE_UUID_RX_STRING, BLEWrite, MAX_MSG_SIZE);
BLEFloatCharacteristic tx_characteristic_float(BLE_UUID_TX_FLOAT, BLERead | BLENotify);
BLECStringCharacteristic tx_characteristic_string(BLE_UUID_TX_STRING, BLERead | BLENotify, MAX_MSG_SIZE);

// RX
RobotCommand robot_cmd(":|");

// TX
EString tx_estring_value;
float tx_float_value = 0.0;

long interval = 500;
static long previousMillis = 0;
unsigned long currentMillis = 0;
bool streamToInterface = false;
//////////// Global Variables ////////////

enum CommandTypes
{
    PING,
    START_STREAM,
    STOP_STREAM,
};

void
handle_command()
{   
    // Set the command string from the characteristic value
    robot_cmd.set_cmd_string(rx_characteristic_string.value(),
                             rx_characteristic_string.valueLength());

    bool success;
    int cmd_type = -1;

    // Get robot command type (an integer)
    /* NOTE: THIS SHOULD ALWAYS BE CALLED BEFORE get_next_value()
     * since it uses strtok internally (refer RobotCommand.h and 
     * https://www.cplusplus.com/reference/cstring/strtok/)
     */
    success = robot_cmd.get_command_type(cmd_type);

    // Check if the last tokenization was successful and return if failed
    if (!success) {
        return;
    }

    // Handle the command type accordingly
    switch (cmd_type) {
        /*
         * Write "PONG" on the GATT characteristic BLE_UUID_TX_STRING
         */
        case PING:
            tx_estring_value.clear();
            tx_estring_value.append("PONG");
            tx_characteristic_string.writeValue(tx_estring_value.c_str());

            Serial.print("Sent back: ");
            Serial.println(tx_estring_value.c_str());

            break;
        case START_STREAM:
        {
          streamToInterface = true;
          break;
        }
        case STOP_STREAM:
        {
          streamToInterface = false;
          break;
        }
        
        /* 
         * The default case may not capture all types of invalid commands.
         * It is safer to validate the command string on the central device (in python)
         * before writing to the characteristic.
         */
        default:
            Serial.print("Invalid Command Type: ");
            Serial.println(cmd_type);
            break;
    }
}


void
write_data()
{
    currentMillis = millis();
    if (currentMillis - previousMillis > interval) {

        tx_float_value = tx_float_value + 0.5;
        tx_characteristic_float.writeValue(tx_float_value);

        if (tx_float_value > 10000) {
            tx_float_value = 0;
            
        }

        previousMillis = currentMillis;
    }
}

void
read_data()
{
    // Query if the characteristic value has been written by another BLE device
    if (rx_characteristic_string.written()) {
        handle_command();
    }
}

// Loops through [pressureState] and sends values
void sendPressureStates(){
    tx_estring_value.clear();
    for (int i = 0; i < NUM_SENSORS; i++){
      // e.g. "p0:0.235"
      tx_estring_value.append("p");
      tx_estring_value.append(i);
      tx_estring_value.append(":");
      tx_estring_value.append(pressureState[i]);
      if (i != NUM_SENSORS - 1){
        tx_estring_value.append("|");
      }
    }
    // Expected: "p0:{}|p1:{}|...|p5:{}"
    tx_characteristic_string.writeValue(tx_estring_value.c_str());
}

// Debug
void printRawFSRReadings() {
  Serial.print("Raw FSR Values: ");
  for (int i = 0; i < 6; i++) {
    int analogVal = analogRead(fsrPins[i]);
    Serial.print("f");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(analogVal);
    if (i < 5) Serial.print(" | ");
  }
  Serial.println();
}

void executeFSR(){
  // This method reads and updates the [pressureStates] with current values.
  updatePressureState();

  // This flag is set via bluetooth command.
  if (streamToInterface){
    sendPressureStates();
  }
  // Cap at 50 Hz
  delay(20);
}

void
setup()
{
    Serial.begin(115200);
    initFSRsAndLEDs();

    BLE.begin();

    // Set advertised local name and service
    BLE.setDeviceName("Artemis BLE");
    BLE.setLocalName("Artemis BLE");
    BLE.setAdvertisedService(testService);

    // Add BLE characteristics
    testService.addCharacteristic(tx_characteristic_float);
    testService.addCharacteristic(tx_characteristic_string);
    testService.addCharacteristic(rx_characteristic_string);

    // Add BLE service
    BLE.addService(testService);

    // Initial values for characteristics
    // Set initial values to prevent errors when reading for the first time on central devices
    tx_characteristic_float.writeValue(0.0);

    // Output MAC Address
    Serial.print("Advertising BLE with MAC: ");
    Serial.println(BLE.address());

    BLE.advertise();
}

void
loop()
{
    // Listen for connections
    BLEDevice central = BLE.central();
    executeFSR();
    // printRawFSRReadings();

    // If a central is connected to the peripheral
    if (central) {
        Serial.print("Connected to: ");
        Serial.println(central.address());

        // While central is connected
        while (central.connected()) {
            // Send data
            write_data();

            // Read data
            read_data();

            executeFSR();
            // printRawFSRReadings();
        }

        Serial.println("Disconnected");
    }
}
