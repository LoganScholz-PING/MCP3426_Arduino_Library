#ifndef MCP3426_PING_H
#define MCP3426_PING_H

//#include <Arduino.h> // i think we need this for the typedefs

enum MCP3426_CONVERSION_MODE {
	CONTINUOUS,
	ONESHOT
};

enum MCP3426_RESOLUTION_MODE {
	TWELVE_BIT,
	FOURTEEN_BIT,
	SIXTEEN_BIT
};

enum MCP3426_PGA_MODE {
	TIMES_1,
	TIMES_2,
	TIMES_4,
	TIMES_8
};

bool    initializeMCP3426(); // default init to CH1
bool    initializeMCP3426(int channel); // init to user defined channel
bool    updateMCP3426Config(byte new_config);
int16_t readMCP3426CurrentBits(int channel = 1);
uint8_t readMCP3426Config();
double  readMCP3426CurrentVoltage(int channel = 1);
bool    setMCP3426PGA(MCP3426_PGA_MODE mode);
bool    setMCP3426Resolution(MCP3426_RESOLUTION_MODE mode);
bool    setMCP3426ConversionMode(MCP3426_CONVERSION_MODE mode);
bool    setMCP3426ActiveChannel(int channel);

#endif


