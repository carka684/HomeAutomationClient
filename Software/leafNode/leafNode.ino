

/*
 Copyright (C) 2012 James Coliz, Jr. <maniacbug@ymail.com>
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Simplest possible example of using RF24Network,
 *
 * RECEIVER NODE
 * Listens for messages from the transmitter and prints them out.
 */
#include <avr/pgmspace.h>
#include <nodeconfig.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <printf.h>
#include <sleep.h>
int ledPin = 2;
// nRF24L01(+) radio attached using Getting Started board 
RF24 radio(6,7);

// Network uses that radio
RF24Network network(radio);
const int rootNode = 0;
int lastSent = 0;
bool lastValue = 0; 
unsigned long sendInterval = 500;
int packets_sent = 0;
// Address of our node
eeprom_info_t this_node;
// Structure of our payload
struct payload_t
{
  unsigned long led;
  unsigned long counter;
  char c;
};

void setup(void)
{
  pinMode(ledPin,OUTPUT);
  Serial.begin(57600);
  printf_begin();
  Serial.println("RF24Network/examples/helloworld_rx/");
  this_node = nodeconfig_read();
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ this_node.address);
}

void loop(void)
{
  // Pump the network regularly
  network.update();
  if(millis() - lastSent> sendInterval)
  {
    while(!sendData(rootNode, 1))
      ;    
    lastSent = millis();
    packets_sent++;
    radio.powerDown();
    
  }

  // Is there anything ready for us?
  if ( network.available() )
  {
    // If so, grab it and print it out
    RF24NetworkHeader header;
    payload_t payload;
    network.read(header,&payload,sizeof(payload));
    readData(payload);
  }
}
bool sendData(uint16_t toNode, int value)
{
  Serial.print("Sending...");
  payload_t payload = {
    value , packets_sent, 'a'           };
  RF24NetworkHeader header(/*to node*/ toNode);
  bool ok = network.write(header,&payload,sizeof(payload));
  if (ok)
    Serial.println("ok.");
  else
    Serial.println("failed.");
  return ok;
}
void readData(struct payload_t payload)
{
  Serial.print("Received packet: #");
  Serial.print(payload.counter);
  Serial.print(" value: ");
  Serial.print(payload.led);
  Serial.print(" char: ");
  Serial.println(payload.c);
  if(payload.led == 1)
  {
    digitalWrite(ledPin, HIGH);
  }
  else
  {
    digitalWrite(ledPin, LOW);
  }

}
// vim:ai:cin:sts=2 sw=2 ft=cpp
