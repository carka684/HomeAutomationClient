//Board = Arduino Uno
#define __AVR_ATmega328P__
#define ARDUINO 105
#define F_CPU 16000000L
#define __AVR__
extern "C" void __cxa_pure_virtual() {;}

void setup(void);
void loop(void);
void httpRequest();
void readData(struct payload_t payload);
bool sendData(uint16_t toNode, int value);
void writeBit(char c);
void star(char c);
void minus(char c);
int octToDec(int n);
int intLength (int value);
int exp(int base, int expo);

#include "C:\Program Files (x86)\Arduino\hardware\arduino\variants\standard\pins_arduino.h" 
#include "C:\Program Files (x86)\Arduino\hardware\arduino\cores\arduino\arduino.h"
#include "C:\Users\Calle\Documents\Arduino\RF24Network-master\examples\Calle leker\HomeAutomation\rootNode\rootNode.ino"
