#include <Wire.h>
#include <Arduino.h>
#include "MCP3426_PING.h"

/*
* Quick Start:
1. call initializeMCP3426() [or initalizeMCP3426(channel)] in setup 
	- default initialization: CH1, 16 bit resolution, continuous reading, x1 PGA
2. read current channel value with readMCP3426CurrentBits(channel) or readMCP3426CurrentVoltage(channel)
	- if no channel is given in the arguments, CH1 is default read

* Optional:
3. change the configuration register at any time with the following functions:
	- setMCP3426PGA
	- setMCP3426Resolution
	- setMCP3426ConversionMode
	- setMCP3426ActiveChannel
*/



// Initializer
//
// every time the MCP3426 is power cycled it loses
// its configuration, so we have to re-configure it
// every time we start up
//
// default initializer, chooses channel 1 if no channel provided
// [ch1, 16 bit, continuous, x1 gain]
bool initializeMCP3426() {
    bool ret = false;
    Wire.beginTransmission(MCP3426_I2C_ADDRESS);
	Wire.write((byte)MCP3426_CONFIG_DEFAULT); // default to channel 1
    byte error = Wire.endTransmission();
    ret = error == 0; // return FALSE if failed to initialize
    return ret;
}

// Initializer
//
// if channel is provided, initialize with user-defined channel
// [16 bit, continuous, x1 gain]
bool initializeMCP3426(int channel) {
    bool ret = false;
    Wire.beginTransmission(MCP3426_I2C_ADDRESS);
	
    if (channel == 2) {
        Wire.write((byte)MCP3426_CONFIG_CH2);
    }
    else if (channel == 1) {
        Wire.write((byte)MCP3426_CONFIG_CH1); // default to channel 1
    }
	
    byte error = Wire.endTransmission();
    ret = error == 0; // return FALSE if failed to initialize
    return ret;
}

// helper function to update config after user-triggered updates
bool updateMCP3426Config(byte new_config) {
	bool ret = false;
    Wire.beginTransmission(MCP3426_I2C_ADDRESS);
	Wire.write(new_config);
	byte error = Wire.endTransmission();
    ret = error == 0; // return FALSE if failed to initialize
    return ret;
}

/*
CONFIG BYTE DEFINITION:
[MSB]  7 6 5 4 3 2 1 0  [LSB]
7   = ~RDY  = Data Ready Flag
6,5 = C1,C0 = Channel Selection (00 = CH1, 01 = CH2)
4   = ~O/C  = Conversion Mode (1 = Continuous, 0 = One-Shot)
3,2 = S1,S0 = Sample Rate Selection (10 = 16 bit)
1,0 = G1,G0 = Gain selection (00=x1, 01=x2, 10=x4, 11=x8)  
*/
uint8_t readMCP3426Config() {
    int16_t  full_reading   = 0xDEAD;
    uint8_t  config_reading = 0xFF;
    uint8_t  sign           = 0x00;
	
    Wire.requestFrom(MCP3426_I2C_ADDRESS, (uint8_t)3);

    if(Wire.available()) {
        byte count = 0x0;
        while(Wire.available()) {
            uint8_t temp_8_bits = Wire.read(); // read 1 byte at a time until 3 bytes are read
            if (count == 0x0) {
                full_reading = (int16_t)temp_8_bits;
                full_reading = full_reading << 8; // left shift this first 8 bits to make room for lower 8 bits
            }
            else if (count == 0x1) {
                full_reading |= (int16_t)temp_8_bits; // fill in the last 8 bits of the 16 bit ADC reading
            }
            else {
                config_reading = temp_8_bits;
            }
            count += 0x1;
        }
    }
    else {
        //Serial.println(F("[ERROR] MCP3426 DID NOT RESPOND TO REQUEST."));
    }
    
    return config_reading; // 0xFF returned means there was a problem
}

/*
	============================================================================
	==== this bad boy is the most important/useful function of this library ====
	~~~~~~~~~~~~~~you will use only this function 95% of the time~~~~~~~~~~~~~~~
	============================================================================
	
	data is transmitted in 3 bytes, first 2 bytes are 16 bits of ADC data (WITH SIGN)
	last byte is the 8 bit config packet
*/
int16_t readMCP3426CurrentBits(int channel) {
    int16_t  full_reading   = 0xDEAD; // check for full_reading = 0xDEAD (0b1101 1110 1010 1101 // DEC 57,005)
									  // to determine if a failure has occured during read operation
    uint8_t  config_reading = 0x00;
    uint8_t  sign           = 0x00;
	
	if(!setMCP3426ActiveChannel(channel)) return full_reading; // 0xDEAD

    Wire.requestFrom(MCP3426_I2C_ADDRESS, (uint8_t)3); // request 3 bytes from the MCP3426 (16 bit value + config)

    if(Wire.available()) {
        byte count = 0x0;
        while(Wire.available()) {
            uint8_t temp_8_bits = Wire.read(); // read 1 byte at a time until 3 bytes are read
            if (count == 0x0) {
                full_reading = (int16_t)temp_8_bits;
                full_reading = full_reading << 8; // left shift this first 8 bits to make room for lower 8 bits
            }
            else if (count == 0x1) {
                full_reading |= (int16_t)temp_8_bits; // fill in the last 8 bits of the 16 bit ADC reading
            }
            else {
                config_reading = temp_8_bits;
            }
            count += 0x1;
        }
    }
    else {
        //Serial.println(F("[ERROR] MCP3426 DID NOT RESPOND TO REQUEST."));
    }
    return full_reading; // full_reading will = 0xDEAD if a read failure occured
}

double readMCP3426CurrentVoltage(int channel) {
    int16_t  full_reading   = 0xDEAD;
    uint8_t  config_reading = 0x00;
    uint8_t  sign           = 0x00;
    double   voltage        = -999; // can check for -999 returned to mean status = FAIL
	
    full_reading = readMCP3426CurrentBits(channel);
    
    sign = full_reading >> 15;
    if (sign == 0x1) { /* voltage returned will be -999 if we somehow measure a negative voltage on the ADC */ }
    else {
        voltage = (double)full_reading * 0.0000625; // 62.5uV per division @ gain = 1x
    }
    return voltage; // can check for -999 returned to mean status = FAIL
}


bool setMCP3426PGA(MCP3426_PGA_MODE mode) {
	// 00 = x1
	// 01 = x2
	// 10 = x4
	// 11 = x8
	// config mask: 00000011
	uint8_t mask = 0B11111100;
	uint8_t current_config = readMCP3426Config();
	
	if(current_config == 0xFF) return false; // config read failed
	
	current_config &= mask; // use switch() to OR in correct bits now
	
	switch(mode) {
		case(TIMES_1):
			current_config |= 0B00000000;
			break;
		case(TIMES_2):
			current_config |= 0B00000001;
			break;
		case(TIMES_4):
			current_config |= 0B00000010;
			break;
		case(TIMES_8):
			current_config |= 0B00000011;
			break;
		default:
		    return false;
			break;
	}
	
	updateMCP3426Config(current_config);
	return true;
}

bool setMCP3426Resolution(MCP3426_RESOLUTION_MODE mode) {
	// AKA RATE
	// 00 = 12 bit
	// 01 = 14 bit
	// 10 = 16 bit
	// config mask: 00001100
	uint8_t mask = 0B11110011;
	uint8_t current_config = readMCP3426Config();
	
	if(current_config == 0xFF) return false; // config read failed
	
	current_config &= mask; // use switch() to OR in correct bits now
	
	switch(mode) {
		case(TWELVE_BIT):
			current_config |= 0B00000000;
			break;
		case(FOURTEEN_BIT):
			current_config |= 0B00000100;
			break;
		case(SIXTEEN_BIT):
			current_config |= 0B00001000;
			break;
		default:
			return false;
			break;
	}
	updateMCP3426Config(current_config);
	return true;
}

bool setMCP3426ConversionMode(MCP3426_CONVERSION_MODE mode) {
	// continuous or oneshot
	// 1 = Continuous
	// 0 = One Shot
	// config mask: 00010000
	uint8_t mask = 0B11101111;
	uint8_t current_config = readMCP3426Config();
	
	if(current_config == 0xFF) return false; // config read failed
	
	current_config &= mask; // use switch() to OR in correct bits now
	
	switch(mode) {
		case(CONTINUOUS):
			current_config |= 0B00010000;
			break;
		case(ONESHOT):
			current_config |= 0B00000000;
			break;
		default:
			return false;
			break;
	}
	updateMCP3426Config(current_config);
	return true;
}

bool setMCP3426ActiveChannel(int channel) {
	// CH1 or CH2
	// 11 = CH1
	// 00 = CH2
	// config mask: 01100000
	uint8_t mask = 0B10011111;
	uint8_t current_config = readMCP3426Config();
	if(current_config == 0xFF) return false; // config read failed
	current_config &= mask; // use switch() to OR in correct bits now
	switch(channel) {
		case(1):
			current_config |= 0B01100000;
			break;
		case(2):
			current_config |= 0B00000000;
			break;
		default:
			return false;
			break;
	}
	updateMCP3426Config(current_config);
	return true;
}