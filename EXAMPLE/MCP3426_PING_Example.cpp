#include <Arduino.h>
#include <Wire.h>
#include <MCP3426_PING.h>

void delaySafeMillis(unsigned long timeToWait) {
	unsigned long start_time = millis();
	while (millis() - start_time <= timeToWait) {}
}

void setup() {
  Wire.begin(); // Wire communication begin
  
  Serial.begin(57600);
  while (!Serial); // Waiting for Serial Monitor
  
  Serial.println(F("[INFO] Welcome MCP3426 Communication Utility. Sending Initialization Packet..."));

  delayMicroseconds(300); // datasheet for MCP3426 says 300us for settle time
  if(!initializeMCP3426()) for(;;); // couldn't init MCP3426 so we're done here
  //if(!initializeMCP3426(2)) for(;;); // TEST FOR CHANNEL 2 INITIALIZATION

  delaySafeMillis(100); // give MCP3426 some time to settle
}

void channel_default_init_test() {
  while(!checkMCP3426ChannelDataReady()) { /* wait for data to become available */ }
  int16_t def_full_reading = readMCP3426CurrentBits();
  double  def_voltage      = readMCP3426CurrentVoltage();
  uint8_t def_config       = readMCP3426Config();
  Serial.println(F("*************** DEFAULT INITIALIZATION ***************"));
  Serial.print(F("[INFO] Config = 0B"));
  Serial.print(def_config, BIN);
  Serial.print(F(" // Full Reading = 0x"));
  Serial.print(def_full_reading, HEX);
  Serial.print(F(" // Voltage = "));
  Serial.print(def_voltage);
  Serial.println(F(" Volts"));
}

void channel_1_test() {
  unsigned long start_time = micros();
  while(!checkMCP3426ChannelDataReady(1)) { /* wait for data to become available */ }
  unsigned long finish_time = micros();
  unsigned long delta = finish_time - start_time;
  int16_t ch1_full_reading = readMCP3426CurrentBits(1);
  double  ch1_voltage      = readMCP3426CurrentVoltage(1);
  uint8_t ch1_config       = readMCP3426Config();
  Serial.print(F("*************** CHANNEL 1 ("));
  Serial.print(delta, DEC);
  Serial.println(F(" microseconds) ***************"));
  Serial.print(F("[INFO] Config = 0B"));
  Serial.print(ch1_config, BIN);
  Serial.print(F(" // Full Reading = 0x"));
  Serial.print(ch1_full_reading, HEX);
  Serial.print(F(" // Voltage = "));
  Serial.print(ch1_voltage);
  Serial.println(F(" Volts"));
}

void channel_2_test() {
  unsigned long start_time = micros();
  while(!checkMCP3426ChannelDataReady(2)) { /* wait for data to become available */ }
  unsigned long end_time = micros();
  int16_t ch2_full_reading = readMCP3426CurrentBits(2);
  double  ch2_voltage      = readMCP3426CurrentVoltage(2);
  uint8_t ch2_config       = readMCP3426Config();
  unsigned long delta = end_time - start_time;
  Serial.print(F("*************** CHANNEL 2 ("));
  Serial.print(delta, DEC);
  Serial.println(F(" microseconds) ***************"));

  Serial.print(F("[INFO] Config = 0B"));
  Serial.print(ch2_config, BIN);
  Serial.print(F(" // Full Reading = 0x"));
  Serial.print(ch2_full_reading, HEX);
  Serial.print(F(" // Voltage = "));
  Serial.print(ch2_voltage);
  Serial.println(F(" Volts"));
}

void print_current_config_nice() {
  uint8_t config_reading = readMCP3426Config();
  Serial.print(F("  -->  CURRENT CONFIG REGISTER: 0B"));
  Serial.println(config_reading, BIN);
}

void config_test() {
  Serial.println(F("CONFIG @ START OF CONFIG TEST ... "));
  print_current_config_nice();

  //
  Serial.println();
  Serial.println(F("Set MCP3426_PGA_MODE::TIMES_2 ... "));
  setMCP3426PGA(MCP3426_PGA_MODE::TIMES_2);
  print_current_config_nice();

  Serial.println(F("Set MCP3426_PGA_MODE::TIMES_4 ... "));
  setMCP3426PGA(MCP3426_PGA_MODE::TIMES_4);
  print_current_config_nice();

  Serial.println(F("Set MCP3426_PGA_MODE::TIMES_8 ... "));
  setMCP3426PGA(MCP3426_PGA_MODE::TIMES_8);
  print_current_config_nice();

  Serial.println(F("Set MCP3426_PGA_MODE::TIMES_1 ... "));
  setMCP3426PGA(MCP3426_PGA_MODE::TIMES_1);
  print_current_config_nice();

  //
  Serial.println();
  Serial.println(F("Set MCP3426_CONVERSION_MODE::ONESHOT ... "));
  setMCP3426ConversionMode(MCP3426_CONVERSION_MODE::ONESHOT);
  print_current_config_nice();

  Serial.println(F("Set MCP3426_CONVERSION_MODE::CONTINUOUS ... "));
  setMCP3426ConversionMode(MCP3426_CONVERSION_MODE::CONTINUOUS);
  print_current_config_nice();

  //

  Serial.println();
  Serial.println(F("Set MCP3426_RESOLUTION_MODE::TWELVE_BIT ... "));
  setMCP3426Resolution(MCP3426_RESOLUTION_MODE::TWELVE_BIT);
  print_current_config_nice();

  Serial.println(F("Set MCP3426_RESOLUTION_MODE::FOURTEEN_BIT ... "));
  setMCP3426Resolution(MCP3426_RESOLUTION_MODE::FOURTEEN_BIT);
  print_current_config_nice();

  Serial.println(F("Set MCP3426_RESOLUTION_MODE::SIXTEEN_BIT ... "));
  setMCP3426Resolution(MCP3426_RESOLUTION_MODE::SIXTEEN_BIT);
  print_current_config_nice();

  //

  Serial.println();
  Serial.println(F("Set MCP3426 Channel 1 ... "));
  setMCP3426ActiveChannel(1);
  print_current_config_nice();

  Serial.println(F("Set MCP3426 Channel 2 ... "));
  setMCP3426ActiveChannel(2);
  print_current_config_nice();

  // now reset config
  Serial.println();
  Serial.println(F("Resetting MCP3426 Config Register to Default ... "));
  initializeMCP3426();
  print_current_config_nice();
}

int main_delay = 2000;
void loop() {
  channel_1_test();
  delaySafeMillis(main_delay);
  Serial.println();
  channel_2_test();
  delaySafeMillis(main_delay);
  Serial.println();
  config_test();
  delaySafeMillis(1000);
  Serial.println();
}