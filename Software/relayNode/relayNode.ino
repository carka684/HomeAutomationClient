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
// nRF24L01(+) radio attached using Getting Started board 
RF24 radio(CE_PIN,CS_PIN);

// Network uses that radio
RF24Network network(radio);

// Address of our node
eeprom_info_t this_node;
// Structure of our payload

//Ecryption
unsigned long key[4] = {KEY0,KEY1,KEY2,KEY3};
Xtea xtea(key);

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
  changeAddress(3);
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
    /*
    if(header.type == SEND_KEY_CHAR)
    {
      readKey(payload);
    }
    else
    {
      readData(payload);
    }
    */
  }
}
void readKey(struct payload_t payload)
{
  
  unsigned long key[4] = {payload.value,payload.command,payload.sensorID,payload.reserved};
  Serial.println(key[0],HEX);
  Serial.println(key[1],HEX);
  Serial.println(key[2],HEX);
  Serial.println(key[3],HEX);
  Serial.println("");
}
void readData(struct payload_t payload)
{
  unsigned long message[2] = {payload.value,payload.command};
  xtea.decrypt(message);
  unsigned long command = message[0];
  unsigned long value = message[1];
  Serial.print(" Command: ");
  Serial.print(command);
  Serial.print(" Value: ");
  Serial.println(value);
    
  switch (command) 
  {
    case CHANGEADRESS_COMMAND:
      changeAddress(value);
      break;
    case CHANGERELAYSTATE_COMMAND:
      Serial.println("changeRelay");
      changeRelayState(value);
      break;
  }
  
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
void changeAddress(int newAddress)
{
  if(this_node.address != newAddress)
  {
    eepromChangeAddress(newAddress);
    wdt_enable(WDTO_15MS);
    while(1);  
  }
}
void changeRelayState(int newState)
{
  digitalWrite(RELAY_PIN, newState); 
}
