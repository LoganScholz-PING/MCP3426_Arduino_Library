#include <Wire.h>
#include <Arduino.h>
#include "MCP3426_PING.h"

uint8_t MCP3426_CONFIG_CH1  = 0B01111000;
uint8_t MCP3426_CONFIG_CH2  = 0B00011000;
uint8_t MCP3426_I2C_ADDRESS = 0x69; // 0110 1001

// every time the MCP3426 is power cycled it loses
// its configuration, so we have to re-configure it
// every time we start up
//
// you don't need to call this function, but you can
// in your setup() loop and use it to determine if the
// MCP3426 is connected and working correctly
bool initializeMCP3426(int channel=1) {
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
int16_t readMCP3426CurrentBits(int channel=1) {
    int16_t  full_reading   = 0xDEAD; // check for full_reading = 0xDEAD (0b1101 1110 1010 1101 // DEC 57,005)
									  // to determine if a failure has occured during read operation
    uint8_t  config_reading = 0x00;
    uint8_t  sign           = 0x00;
	
    // set channel to read from (default is channel 1)
    if (channel == 1) {
        if (!initializeMCP3426(1)) {
            return full_reading; // 0xDEAD
        }
    }
    else if (channel == 2) {
        if (!initializeMCP3426(2)) {
            return full_reading;
        }
    }

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

double readMCP3426CurrentVoltage(int channel=1) {
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
