#include <defines.h>
#include <avr/pgmspace.h>
#include <nodeconfig.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <printf.h>
#include <avr/io.h> 
#include <avr/wdt.h>
int ledPin = 2;
// nRF24L01(+) radio attached using Getting Started board 
RF24 radio(6,7);

// Network uses that radio
RF24Network network(radio);

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
  //test();
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
void changeAddress(int newAddress)
{
  eepromChangeAddress(newAddress);
  wdt_enable(WDTO_15MS);
  while(1);  
}
