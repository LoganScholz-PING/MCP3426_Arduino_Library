#ifndef MCP3426_PING_H
#define MCP3426_PING_H

//#include <Arduino.h> // i think we need this for the typedefs

bool    initializeMCP3426(int channel = 1);
int16_t readMCP3426CurrentBits(int channel = 1);
uint8_t readMCP3426Config();
double  readMCP3426CurrentVoltage(int channel = 1);

#endif