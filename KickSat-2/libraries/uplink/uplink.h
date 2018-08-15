/**
Uplink handler for KickSat, uplink.h
Purpose: Library for handling the reading and processing of uplink to the satellite
@author Ralen Toledo
@version 1.0 08/12/18
*/

#ifndef UPLINK_h
#define UPLINK_h

#include <KickSatLog.h>
#include <ChecksumHandler.h>
// #include <ax25.h>
// #include <RadioHead.h>
#define NUM_COMMANDS 8
#define COMMAND_WIDTH 3


ChecksumHandler checksumHandler;
bool armingMode;

const byte commandDict[NUM_COMMANDS][COMMAND_WIDTH] = {/* REDACTED */};

bool parseUplink(char *buf);
void processUplink(char *buf);

///////////////////////
// Listen for uplink //
//////////////////////////////////////////////////////////////////////////////
bool listenForUplink(char *buf, uint8_t duration) {
  timeout.start(duration); // Start timeout timer for the listening duration
  while(1) { // Wait for uplink, retreive from buffer
    // if (radio.available()) { // TODO: change if radio is available, not serial
    if (SerialUSB.available()) { // If data is in buffer to be read
      if (parseUplink(buf)) { // Read from buffer and validate uplink
        return true;
      }
    }
    if (timeout.triggered()) { // Checks time for timeout
      SerialUSB.println("Uplink Timeout");
      return false;
    }
  // sleepTimer.sleep(); // Go into sleep mode until next interrupt
  }
}

bool parseUplink(char *buf) {
  ////////////////////////////////////////////////////////
  // TODO: change from Serial.read to radio.read(char *buf)
  uint32_t i = 0;
  // while(radio.available() > 0) { // Read until the entire buffer has been read // TODO: This function doesn't exist but should
  //   buf[i] = radio.read(); // Reads the available char (ASCII) // TODO: This function doesn't exist but should
  while(SerialUSB.available() > 0) { // Read until the entire buffer has been read
    buf[i] = SerialUSB.read(); // Reads the available char (ASCII)
    i++;
  }
  buf[i] = '\0';
  ////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////
  // if (checksum(buf)) // Verifies checksum of read uplink // TODO: Incorporate real checksum check here for input
  if (checksumHandler.evaluateChecksum(buf, i)) { // If uplink is valid, to be replaced by checksum validation
    SerialUSB.print("ACK:"); // If checksum is valid, respond with ACK
    SerialUSB.println(buf);
    // radio.send("ACK") // If checksum is valid, respond with ACK // TODO: This function doesn't exist but should
    return true; // Return successfull uplink
  } else {
    SerialUSB.print("NACK:");
    SerialUSB.println(buf);
    // radio.send("NACK") // If checksum is not valid, respond with NACK // TODO: This function doesn't exist but should
    return false; // Return unsuccessfull uplink
  }
  ////////////////////////////////////////////////////////
}

////////////////////
// Process uplink //
//////////////////////////////////////////////////////////////////////////////
uint8_t extractCommand(char* buf) {
  for (int i = 0; i < NUM_COMMANDS; i++) {
    bool flag = true;
    for (int n = 0; n < COMMAND_WIDTH; n++) {
      if (commandDict[i][n] != buf[n]) {
        flag = false;
        break;
      }
    }
    if (flag) {
      return i;
    }
  }
}

void processUplink(char *buf) {

  uint8_t command = extractCommand(buf);

  // respond to the command
  switch(command)
  {
    // Burn wires
    case 0:
    SerialUSB.println("Command: Burn Wire");
    // send_sensor_data();
    break;
    // Send down Sensor data
    case 2:
    SerialUSB.println("Command: Downlink Sensors");
    // Get some user input for the number of logs to send down
    // Normally this o
    // SerialUSB.println("How many logs shall we send down?");
    // logs_to_send = return_selection();
    // SerialUSB.print("Fetching the last ");
    // SerialUSB.print(logs_to_send);
    // SerialUSB.println(" logs");

    // TODO: @emma read the last logs_to_send logs into the multiple_logs

    // serial_transmit(multiple_logs);

    break;

    // Downlink last x number of logs, depends on user input
    case 3:
    SerialUSB.println("Command: Downlink Logs");
    // TODO: make a function that does this...
    break;

    // Rewrite the sensor config files @connor @max
    case 4:
    // TODO: make a function that does this...
    SerialUSB.println("Command: Uplink Sensor Config");
    break;

    // Send the mission config files
    case 5:
    // TODO: @emma
    // read the mission config files into config_string =
    // serial_transmit(config_string);
    SerialUSB.println("Command: Downlink Mission Status");
    break;

    // Enter arming mode, exit standby mode
    case 6:
    SerialUSB.println("Command: Enter Arming Mode");
    // Set arming flag
    armingMode = true;


    // TODO: enter arming mode
    // send: "Entered arming mode"
    // This is a transition condition in the more general state diagram
    // we will exit standby mode here
    break;

    // Enter End of Life mode
    case 7:
    SerialUSB.println("Command: Enter End of Life Mode");
    // Send: ACK, are you sure you want to explode all of our hard work into pixie dust?
    // serial_transmit("Are you sure you want to kill KickSat II? (y/n)");
    // wait for response: if (response != "yes explode" ) --> go back into standby mode
    // command = return_selection();
    // SerialUSB.print("case 7, command: ");
    // SerialUSB.println(command);
    // if((command == 41) || (command == 73))
    // {
    //   // go into end of life mode
    //   // this is a state change into end of life mode
    //   SerialUSB.println("Killing the goddamn 3u that never deserved to live MUAHAHAHAHA!");
    // }
    // else
    // {
    //   // do not kill the satelite
    //
    //   command = 0;
    // }

    break;

    // No command --> go back to sleep and go through another standby mode loop
    case 8:
    //
    SerialUSB.println("Doing no command... going to sleep");
    break;

    // No command --> go back to sleep and go through another standby mode loop
    default:
    SerialUSB.println("Doing no command... going to sleep");
    break;
  }

}

#endif
