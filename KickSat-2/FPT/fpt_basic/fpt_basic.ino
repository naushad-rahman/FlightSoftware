/*
 * Basic FPT - goes through each of the components in KMB and checks whether they are all responsive
 * 
 * Components to test: WDT, SD Card, Radio, IMU, Battery readings, Gyro, Relays, Sensors
 * 
 * Last updated: 23-9-2018
 *           By: Andrea 
 */

//Includes
#include <BattHandler.h>
#include <GyroHandler.h>
#include <IMUHandler.h>
#include <KickSat_Sensor.h>
#include <RH_RF22.h>  
#include <RTCCounter.h>
#include <SdFat.h>
#include <SPI.h> 

//Global objects
BattHandle power;
Counter watchdogTimer;
IMUHandle IMU;
GyroHandle gyroscope;
SdFat SD;

KickSat_Sensor kSensor1(SPI_CS_XTB1, XTB_RESET, SPI_CS_SD, "sc1.txt", SD);
KickSat_Sensor kSensor2(SPI_CS_XTB2, XTB_RESET, SPI_CS_SD, "sc2.txt", SD);
KickSat_Sensor kSensor3(SPI_CS_XTB3, XTB_RESET, SPI_CS_SD, "sc3.txt", SD);

//Radio 
RHHardwareSPI spi;
RH_RF22 radio(SPI_CS_RFM, RF_NIRQ, spi);
RH_RF22::ModemConfig FSK1k2 = {
  0x2B, //reg_1c
  0x03, //reg_1f
  0x41, //reg_20
  0x60, //reg_21
  0x27, //reg_22
  0x52, //reg_23
  0x00, //reg_24
  0x9F, //reg_25
  0x2C, //reg_2c - Only matters for OOK mode
  0x11, //reg_2d - Only matters for OOK mode
  0x2A, //reg_2e - Only matters for OOK mode
  0x80, //reg_58
  0x60, //reg_69
  0x09, //reg_6e
  0xD5, //reg_6f
  0x24, //reg_70
  0x22, //reg_71
  0x01  //reg_72
};
                   
uint8_t packet[] = {
  0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111,
  0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111,
  0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111,
  0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111,
  0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111,
  0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111,
  0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111,
  0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111,
  0b01110101, 0b10010110, 0b01010100, 0b10101011, 0b01010100, 0b10101011, 0b01010111, 0b01110010,
  0b01011010, 0b01001000, 0b10110101, 0b10101101, 0b10001010, 0b00101000, 0b11101010, 0b10101111,
  0b10110011, 0b00110111, 0b00010000, 0b10110000, 0b11011000, 0b10000111, 0b00001000, 0b10101001,
  0b00101110, 0b10010001, 0b00010001, 0b00000011, 0b01111101, 0b11000000, 0b01111111
};

//Global variables
int SDWrites = 0;
int WDTCounter = 1;
unsigned int radioPackets = 1;
File filetest;

//Flags
bool LEDSTATE = false; // to toggle LED, helps us test whether the board is alive

void setup() {
  // Define pin modes
  setPins();

  // Begin timers
  watchdogTimer.init(1000, watchdog); // timer delay, milliseconds

  // Initialize Serial
  SerialUSB.begin(115200); // Restart SerialUSB
  while(!SerialUSB); // Wait for SerialUSB USB port to open
  SerialUSB.println("SerialUSB Initialized");
  delay(2500); // Provides user with time to open SerialUSB Monitor or upload before sleep

  //Need a delay before turning on radio
  //so that power supply can stabilize
  digitalWrite(RF_SDN, HIGH);
  delay(2000);
  digitalWrite(RF_SDN, LOW);
  delay(500);

  SPI.begin(); //for sensors

  BattHandle(); //initializes battery pins

  if(IMU.begin()){ // Initialize IMU
    SerialUSB.println("IMU Intialized");
  } else {
    SerialUSB.println("IMU Could Not Be Intialized");
  }

  if(gyroscope.begin()){ // Initialize gyro
    SerialUSB.println("Gyro Intialized");
  } else {
    SerialUSB.println("Gyro Could Not Be Intialized");
  }

  if(SD.begin(SPI_CS_SD)) { //Initialize SD Card
    SerialUSB.println("SD initialized");
  } else {
    SerialUSB.println("SD not initialized");
  }

  if(!radio.init()) { //Setup radio
    SerialUSB.println("Radio couldnt initialize");
    SerialUSB.println(radio.statusRead(), HEX);
  } else {
    SerialUSB.println("Radio initialized");
  }
  radio.setFrequency(437.505, 0.1);
  radio.setModemRegisters(&FSK1k2);
  radio.setTxPower(RH_RF22_RF23BP_TXPOW_30DBM);

}

void loop() { //check all parts of the board

  printMenu();
  
  while(SerialUSB.available() == 0); //wait for user input
  int option = SerialUSB.read();
  delay(200);

  if(option == '1') {
    SerialUSB.println("******Testing the Power Readings******");
    for(int i = 0; i < 3; i++) {
      checkBattHandler();
      delay(100);
    }
  } else if (option == '2') {
    SerialUSB.println("******Testing the IMU******");
    for(int i = 0; i < 3; i++) {
      checkIMUHandler();
      delay(100);
    }
  } else if (option == '3') {
    SerialUSB.println("******Testing the Gyro******");
    for(int i = 0; i < 5; i++) {
      checkGyroHandler();
      delay(100);
    }
  } else if (option == '4') {   
    checkSDCard();
  } else if (option == '5') {
    //Check radio chriping
    SerialUSB.println("******Testing the Radio******");
    for(int i = 0; i < 5; i++) {
      checkRadio();
      delay(750);
    }       
  } else if (option == '6') {
    //remove file from SD card
    removeFile();
  } else if (option == '7') {
    //toggle relays
    checkRelays(); 
  } else if (option == '8') {
    //test XTBs
    checkXTB();
  } else if (option == '9') {
    //burn a burn wire
    burnWire();
  } else {
    SerialUSB.println("Invalid option");
  }
  
//  kSensor1.resetADC();
//  kSensor1.startADC();
//  delay(200);
//  kSensor1.regReadout();

  delay(1000);
}

void setPins() {
  pinMode(LED_BUILTIN, OUTPUT); // Defines builtin LED pin mode to output
  pinMode(WDT_WDI, OUTPUT); // Set watchdog pin mode to output
  pinMode(RF_SDN, OUTPUT);
  pinMode(BURN_RELAY_A, OUTPUT);
  pinMode(BURN_RELAY_B, OUTPUT);
  digitalWrite(WDT_WDI, LOW);
  //turn off all SPI devices
  pinMode(SPI_CS_RFM, OUTPUT);
  digitalWrite(SPI_CS_RFM, HIGH);
  pinMode(SPI_CS_XTB1, OUTPUT);
  digitalWrite(SPI_CS_XTB1, HIGH);
  pinMode(SPI_CS_XTB2, OUTPUT);
  digitalWrite(SPI_CS_XTB2, HIGH);
  pinMode(SPI_CS_XTB3, OUTPUT);
  digitalWrite(SPI_CS_XTB3, HIGH);
  pinMode(SPI_CS_XTB4, OUTPUT);
  digitalWrite(SPI_CS_XTB4, HIGH);
  pinMode(SPI_CS_SD, OUTPUT);
  digitalWrite(SPI_CS_SD, HIGH);
  pinMode(SPI_CS_MRAM, OUTPUT);
  digitalWrite(SPI_CS_MRAM, HIGH);

}

void printMenu() {
  SerialUSB.println("******Please enter a number to choose a test: ******");
  SerialUSB.println("1: Battery test");
  SerialUSB.println("2: IMU test");
  SerialUSB.println("3: Gyro test");
  SerialUSB.println("4: SD read/write test");
  SerialUSB.println("5: Radio test");
  SerialUSB.println("6: SD remove test file");
  SerialUSB.println("7: Relay test");
  SerialUSB.println("8: Sensor test");
  SerialUSB.println("9: Burn wire test (Not ready yet)");
}

void checkBattHandler() { //read battery current draw, voltage, and charging current
  float battCurr = power.readBattCurrent();
  SerialUSB.print("Current Draw:\t\t");
  SerialUSB.print(battCurr);
  SerialUSB.println(" mA");
  delay(500);
  float battery = power.readBattVoltage();
  SerialUSB.print("Battery Voltage:\t");
  SerialUSB.print(battery,3);
  SerialUSB.println(" V");
  delay(500);
  float battChrg = power.readBattChargeCurrent();
  SerialUSB.print("Charging Current:\t");
  SerialUSB.print(battChrg);
  SerialUSB.println(" mA");
  SerialUSB.println("");
}

void checkIMUHandler() { //read accelerometer, magnometer, and gyroscope in imu
  float imuData[9];
  IMU.read(imuData); // Read IMU data
  for(int i = 0; i < 9; i++) { // Print to Serial IMU data
    SerialUSB.print(i);SerialUSB.print(": ");SerialUSB.print(imuData[i]);SerialUSB.print("\t");
  }
  SerialUSB.println("\n");
}

void checkGyroHandler() { //gets data from the gyro
  float gyroData[3];
  gyroscope.read(gyroData); // Read gyro data
  SerialUSB.print("X: "); SerialUSB.print(gyroData[0]); SerialUSB.print("  ");
  SerialUSB.print("Y: "); SerialUSB.print(gyroData[1]); SerialUSB.print("  ");
  SerialUSB.print("Z: "); SerialUSB.print(gyroData[2]); SerialUSB.print("  ");
  SerialUSB.println("rad/s ");
  SerialUSB.println("\n");
}

void checkSDCard() { //read and writes to the SD Card
  SerialUSB.println("******Testing the SD Card******");
  filetest = SD.open("TestSD.txt", FILE_WRITE); //first open the file to write
  if(filetest) { //if the file exists, print one new line on the file
    SerialUSB.print("Writing to SD card...");
    filetest.print(SDWrites);
    filetest.println(": Hi KickSat Team! The SD Card is working correctly.");
    filetest.close();
    SDWrites++;
    SerialUSB.println("done");
  } else {
    SerialUSB.println("Couldnt open TestSD.txt");
  }

  filetest = SD.open("TestSD.txt", FILE_READ); //reopen the file to read
  if(filetest) { //if the file opened, read all its contents
    SerialUSB.println("SDFile contents: ");
    while(filetest.available()) {
      SerialUSB.print((char)filetest.read());
    }
    SerialUSB.println("");
    filetest.close();
  } else {
    SerialUSB.println("Couldnt open TestSD.txt");
  }
}

void checkRadio() { //sends a radio packet 
  spi.begin();
  SerialUSB.print("Status byte: 0x");
  SerialUSB.print(radio.statusRead(), HEX);
  SerialUSB.print("\t Packet ");
  radio.send(packet, 95);
  radio.waitPacketSent(500);
  SerialUSB.println(radioPackets++);
  SerialUSB.println("");
}

void removeFile() { //removes the test file (to delete any other file, replace the text file name in lines 268, 269 to the file youd like to remove)
  SerialUSB.println("******Removing TestSD.txt******");
  if(SD.exists("TestSD.txt")) {
    if(SD.remove("TestSD.txt")) {
      SerialUSB.println("Test file removed");
      SDWrites = 0; //reset line counter for test file
    } else { 
      SerialUSB.println("Couldn't remove test file");
    }
  } else {
    SerialUSB.println("TestSD.txt file doesnt exist");
  }
  SerialUSB.println("");
}

void checkRelays() { //toggles both relays to check if they are clicking or not
  SerialUSB.println("******Testing Relays******");
  SerialUSB.println("Toggling relay A (used for antenna deployment)");
  for(int i = 0; i < 3; i++) {
    digitalWrite(BURN_RELAY_A, HIGH);
    delay(1000);
    digitalWrite(BURN_RELAY_A, LOW);
    delay(1000);
  }
  SerialUSB.println("Relay A was toggled on and off 3 times");
  SerialUSB.println("Toggling relay B (used for sprite deployment)");
  for(int i = 0; i < 3; i++) {
    digitalWrite(BURN_RELAY_B, HIGH);
    delay(1000);
    digitalWrite(BURN_RELAY_B, LOW);
    delay(1000);
  }
  SerialUSB.println("Relay B was toggled on and off 3 times");
  SerialUSB.println("");
}

void checkXTB() {
  SerialUSB.println("******Testing XTBs******");
  SerialUSB.println("Testing XTB1");
  kSensor1.resetADC();
  delay(200);
  kSensor1.startADC();
  delay(200);
  kSensor1.regReadout();
  delay(500);
  SerialUSB.println("Testing XTB2");
  kSensor2.resetADC();
  delay(200);
  kSensor2.startADC();
  delay(200);
  kSensor2.regReadout();
  delay(500);
  SerialUSB.println("Testing XTB3");
  kSensor3.resetADC();
  delay(200);
  kSensor3.startADC();
  delay(200);
  kSensor3.regReadout();
  delay(500);
  SerialUSB.println("");
}

void burnWire() { //burn a burn wire
  SerialUSB.println("Are you sure you want to burn a wire? (Enter y to confirm)");
  while(SerialUSB.available() == 0); //wait for user input
  int choice = SerialUSB.read();
  delay(200);
  if (choice != 'y') return; //exit if user doesnt confirm burn command

  SerialUSB.println("******Testing Burnwire******");
  int relay; int enable; //relay and enable to be chosen by user
  SerialUSB.println("Choose a relay, enter A for antenna relay or B for sprite relay");
  while(1) {
    while(SerialUSB.available() == 0); //wait for user input
    choice = SerialUSB.read();
    delay(200);
    if (choice == 'A') {
      SerialUSB.println("You chose relay A");
      relay = BURN_RELAY_A;
      break;
    } else if (choice == 'B') {
      SerialUSB.println("You chose relay B");
      relay = BURN_RELAY_B;
      break;
    } else {
      SerialUSB.println("Not valid input, try again");
    }
  }
  
  SerialUSB.println("Choose an enable, enter 1 for antenna 1, 2 for antenna 2, 3 for deploy 1, 4 for deploy 2, or 5 for deploy 3");
  while(1) {
    while(SerialUSB.available() == 0); //wait for user input
    choice = SerialUSB.read();
    delay(200);
    if (choice == '1') {
      SerialUSB.println("You chose antenna 1");
      enable = ENAB_BURN2;
      break;
    } else if (choice == '2') {
      SerialUSB.println("You chose antenna 2");
      enable = ENAB_BURN1;
      break;
    } else if (choice == '3') {
      SerialUSB.println("You chose deploy 1");
      enable = ENAB_BURN3;
      break;
    } else if (choice == '4') {
      SerialUSB.println("You chose deploy 2");
      enable = ENAB_BURN5;
      break;
    } else if (choice == '5') {
      SerialUSB.println("You chose deploy 3");
      enable = ENAB_BURN4;
      break;
    } else {
      SerialUSB.println("Not valid input, try again");
    }
  }
  //TODO: Finish this test, need to know desired duty cycle, frequency, and duration of burn
}

void watchdog() { // Function that runs every time watchdog timer triggers
  digitalWrite(WDT_WDI, HIGH);
  delayMicroseconds(2);
  digitalWrite(WDT_WDI, LOW);
  if(LEDSTATE) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
  LEDSTATE = !LEDSTATE;
  SerialUSB.print("Toggled WDT "); SerialUSB.print(WDTCounter); SerialUSB.println(" time(s)");
  WDTCounter++;
}
