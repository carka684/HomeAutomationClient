#include <globals.h>
#include <avr/pgmspace.h>
#include <nodeconfig.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <printf.h>
#include <avr/io.h> 
#include <avr/wdt.h>

// nRF24L01(+) radio attached using Getting Started board 
RF24 radio(CE_PIN,CS_PIN);

// Network uses that radio
RF24Network network(radio);

// Address of our node
eeprom_info_t this_node;
// Structure of our payload


void setup(void)
{
 
  pinMode(RELAY_PIN,OUTPUT);
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
    int command = payload.command;
    int value = payload.value;
    
    Serial.print("Received packet: #");
    Serial.print(payload.command);
    Serial.print(" value: ");
    Serial.print(payload.value);
    
    switch (command) 
    {
      case CHANGEADRESS_COMMAND:
        changeAddress(value);
        break;
      case CHANGERELAYSTATE_COMMAND:
        changeRelayState(value);
        break;
    }
  
}
void changeAddress(int newAddress)
{
  eepromChangeAddress(newAddress);
  wdt_enable(WDTO_15MS);
  while(1);  
}
void changeRelayState(int newState)
{
  digitalWrite(RELAY_PIN, newState); 
}
