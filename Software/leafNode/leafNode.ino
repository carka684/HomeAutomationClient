#include "LowPower.h"
#include <globals.h>
#include <avr/pgmspace.h>
#include <nodeconfig.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <printf.h>
#include <avr/io.h> 
#include <avr/wdt.h>
#include <Xtea.h>

int ledPin = 2;
int tempC;
int reading;
int tempPin = 0;
int voltPin = 1;
int transPin = 8;
int sleepCycles = 8;
// nRF24L01(+) radio attached using Getting Started board 
RF24 radio(6,7);

// Network uses that radio
RF24Network network(radio);
unsigned long lastSent = 0;
bool lastValue = 0; 
unsigned long sendInterval = 5000;
int packets_sent = 0;
// Address of our node
eeprom_info_t this_node;
// Structure of our payload
//Ecryption
unsigned long key[4] = {KEY0,KEY1,KEY2,KEY3};
Xtea xtea(key);
 
unsigned long counter = 0;

void setup(void)
{
  analogReference(INTERNAL);
  pinMode(voltPin,INPUT);
  pinMode(tempPin,INPUT);
  pinMode(transPin,OUTPUT);
  
  Serial.begin(57600);
  //printf_begin();
  Serial.println("RF24Network/examples/helloworld_rx/");
  this_node = nodeconfig_read();
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ this_node.address);
  

}

void loop(void)
{
  
  if(sleepCycles > 7)
  {
    network.update();
    int time = millis();
    while(!sendData(ROOTNODE, 100*getVoltage(), VOLT_SENSOR));
    while(!sendData(ROOTNODE, 100*getTemp(), TEMP_SENSOR));
    sleepCycles = 0;
  }
  sleepCycles++;
  
  radio.powerDown();
  delay(500);
  //LowPower.powerDown(SLEEP_500MS, ADC_OFF, BOD_OFF);
    
}
bool sendData(unsigned long toNode, unsigned long value, unsigned long sensorID)
{
  unsigned long message[2] = {value,sensorID};
  xtea.encrypt(message);

  payload_t payload = 
  {
    NO_COMMAND, 
    message[0],
    message[1],
    NO_RESERV
  };
  RF24NetworkHeader header(/*to node*/ toNode);
  bool ok = network.write(header,&payload,sizeof(payload));
  if (ok)
    Serial.println("ok.");
  else
    Serial.println("failed.");
  return ok;
}
void readKey(struct payload_t payload)
{
  unsigned long key[4] = {payload.command,payload.value,payload.sensorID,payload.reserved};
}

float getTemp()
{
  digitalWrite(transPin,HIGH);
  delay(1);
  reading = analogRead(tempPin);
  digitalWrite(transPin,LOW);
  return reading/9.81;
}
float getVoltage()
{
  reading = analogRead(voltPin);
  return (reading*2.818*1.1)/1023;
}
void changeAddress(int newAddress)
{
  eepromChangeAddress(newAddress);
  wdt_enable(WDTO_15MS);
  while(1);  
}


