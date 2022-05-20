#include <Arduino.h>
#include <Wire.h> //include Wire.h library
#include <MCP3426_PING.h>

void delaySafeMillis(unsigned long timeToWait) {
	unsigned long start_time = millis();
	while (millis() - start_time <= timeToWait) {}
}

void setup() {
  Wire.begin(); // Wire communication begin
  
  Serial.begin(57600);
  while (!Serial); // Waiting for Serial Monitor
  
  Serial.println(F("[INFO] Welcome MCP3426 Communication Utility. Sending Config Packet..."));
  if(!initializeMCP3426()) for(;;) // couldn't init MCP3426 so we're done here
	                           // NOTE that calling initializeMCP3426() with no parameters 
	                           // sets channel 1 as default read channel
	                           // call initalizeMCP3426(1); to set config to auto read channel 1
	                           // call initalizeMCP3426(2); to set config to auto read channel 2
	                           // - you don't NEED to initialize though, readMCP3426([channel here]) writes config packet
  
  delaySafeMillis(100); // give MCP3426 some time to settle
}

void loop() {
  // READ CHANNEL 1 AND PRINT
  Serial.println("*********************************");
  Serial.println("*********************************");

  int16_t full_reading_ch1 = readMCP3426CurrentBits(1);
  double voltage_ch1 = readMCP3426CurrentVoltage(1);
  uint8_t config_ch1 = readMCP3426Config();
  Serial.print(F("[INFO] CH1 Config = 0x"));
  Serial.print(config_ch1, HEX);
  Serial.print(F(" (0B"));
  Serial.print(config_ch1, BIN);
  Serial.print(F(") // CH1 Full Reading = 0x"));
  Serial.print(full_reading_ch1, HEX);
  Serial.print(F(" // CH1 Voltage = "));
  Serial.print(voltage_ch1);
  Serial.println(F(" Volts"));

  // READ CHANNEL 2 AND PRINT
  delaySafeMillis(50); // don't need this

  int16_t full_reading_ch2 = readMCP3426CurrentBits(2);
  double voltage_ch2 = readMCP3426CurrentVoltage(2);
  uint8_t config_ch2 = readMCP3426Config();

  Serial.print(F("[INFO] CH2 Config = 0x"));
  Serial.print(config_ch2, HEX);
  Serial.print(F(" (0B"));
  Serial.print(config_ch2, BIN);
  Serial.print(F(") // CH2 Full Reading = 0x"));
  Serial.print(full_reading_ch2, HEX);
  Serial.print(F(" // CH2 Voltage = "));
  Serial.print(voltage_ch2);
  Serial.println(F(" Volts"));

  delaySafeMillis(1000); // don't blow up our serial monitor
}
