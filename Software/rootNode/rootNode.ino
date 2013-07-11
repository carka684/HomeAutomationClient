



#include <avr/pgmspace.h>
#include <nodeconfig.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <Ethernet.h>
#include <printf.h>
#define VALUECHAR '*'
#define TONODECHAR '-'
char nodeArray[10] = {
};
char seekCharArray[] = {'-','*'};
char valueArray[10] = {};
int valueCounter = 0;
int charCounter = 0;
bool foundValue = false;



int led = 0;
uint16_t node = 0;
int ledPin = 2;
bool foundMinus = false;

int i = 0;

/*
  INTERNETSAKER
 */
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
EthernetClient client;
IPAddress server(79,136,60,91);

unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
boolean lastConnected = false;                 // state of the connection last time through the main loop
const unsigned long postingInterval = 1000;  // delay between updates, in milliseconds
int time = 0;
int counterStar = 0;
int counterMinus = 0;
/*
  RADIOSAKER
 */
// nRF24L01(+) radio attached using Getting Started board 
RF24 radio(6,7);

// Network uses that radio
RF24Network network(radio);

eeprom_info_t this_node;


// Address of the other node
//const uint16_t other_node = 0;

// How often to send 'hello world to the other unit
const unsigned long interval = 100; //ms

// When did we last send?
unsigned long last_sent;

// How many have we sent already
unsigned long packets_sent;

// Structure of our payload
struct payload_t
{
  unsigned long value;
  unsigned long counter;
  char c;
};
struct message_t
{
  int toNode;
  int value;
  char c;
};

message_t message;

bool test = true;
void setup(void)
{
  pinMode(ledPin,OUTPUT);
  Serial.begin(57600);
  printf_begin();
  Serial.println("RF24Network/examples/helloworld_tx/");
  this_node = nodeconfig_read();
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ this_node.address);

  delay(1000);
  Ethernet.begin(mac);
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());

}

void loop(void)
{
  // Pump the network regularly
  network.update();
  if(network.available())
  {
    RF24NetworkHeader header;
    payload_t payload;
    network.read(header,&payload,sizeof(payload));
    readData(payload, header.from_node);
    
  }
  
  /*
  int i = 0;
  while (client.available()) {
    Serial.println(seekCharArray[i]);
    char c = client.read();
    int result = findChar(c,seekCharArray[i]);
    if(result)
    {
      
      if(c == TONODECHAR)
      {
        //message.toNode = octToDec(result);
      }
      else if(c == VALUECHAR)
      {
        //message.value = result; 
      }
      
      if(i < (sizeof(seekCharArray)-1))
      {
        i++;
      }
      else
      {
        //sendData(message.toNode,message.value);
        break;
      }
      
    }
  }
  */
   if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    Serial.println();
    client.stop();
  }
  // if you're not connected, and ten seconds have passed since
  // your last connection, then connect again and send data:
  if(!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
    //httpRequest();
    //httpPostSensorData("");
  }
  // store the state of the connection for next time through
  // the loop:
  lastConnected = client.connected();
}
// this method makes a HTTP connection to the server:
void httpPostSensorData(float value, int fromNode)
{
    Serial.println("\n post");
    if (client.connect(server, 80)) 
    {
      // send the HTTP PUT request:
      client.print("GET /postSensorData.php?");
      client.print("fromNode=");
      client.print(fromNode);
      client.print("&");
      client.print("value=");
      client.print(value/100);
      client.println(" HTTP/1.1");
      client.println("Host: www.calle.myxtreamer.net");
      client.println("User-Agent: arduino-ethernet");
      client.println("Connection: close");
      client.println();
      client.stop();
      Serial.println("Post successful");
    }  
    else
    {
     Serial.println("Post failed");
     client.stop();
     }
}
void httpRequest() {
  Serial.println("http");
  // if there's a successful connection:
  if (client.connect(server, 80)) {
    // send the HTTP PUT request:

    client.println("GET /info.php HTTP/1.1");
    client.println("Host: calle.st");
    client.println("User-Agent: arduino-ethernet");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  } 
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    Serial.println("disconnecting.");
    Serial.print("Time: ");
    int s = millis()/1000;
    int m = millis()/60000;
    int h = millis()/3600000;
    Serial.print(h);
    Serial.print(":");
    Serial.print(m - h*60);
    Serial.print(":");
    Serial.println(s - m*60);
    client.stop();
  }
}
void readData(struct payload_t payload, int fromNode)
{
    Serial.print("Received packet: #");
    Serial.print(payload.counter);
    Serial.print(" value: ");
    Serial.print(payload.value);
    Serial.print(" char: ");
    Serial.println(payload.c);
    if(payload.value == 1)
    {
      digitalWrite(ledPin, HIGH);
    }
    else
    {
       digitalWrite(ledPin, LOW);
    }
    httpPostSensorData(payload.value,fromNode);
  
}

bool sendData(uint16_t toNode, int value)
{
  Serial.print("Sending...");
  payload_t payload = {
    value , packets_sent++, 'a'         };
  RF24NetworkHeader header(/*to node*/ toNode);
  bool ok = network.write(header,&payload,sizeof(payload));
  if (ok)
    Serial.println("ok.");
  else
    Serial.println("failed.");
  return ok;
}
int findChar(char currentChar, char seekChar)
{
  if(charCounter == 3 && currentChar != seekChar)
  {
    valueArray[valueCounter++] = currentChar;
    foundValue = true;
    return false; 
  }
  
  if(currentChar == seekChar && !foundValue)
  {
    charCounter++;
    return false;
  }
  else if(currentChar == seekChar && foundValue)
  {
    int tempValue = atoi(valueArray);
    charCounter = 0;
    memset(valueArray, 0, 10);
    foundValue = false;
    return tempValue;
  }
  else if(currentChar != seekChar && foundValue)
  {
    return false;
  }
  else
  {
    charCounter = 0;
    return false;
  } 
}
/*
void writeBit(char c)
{
  star(c);
  minus(c);

}

  
void star(char c)
{
  if(counterStar == 3)
  {
    if(c == '1')
    {
      led = 1;
    }
    if(c == '0')
    {
      led = 0;
    }
  }
  if(c == '*')
  {
    counterStar++;

  }
  else
  {
    counterStar = 0;
  }  
}
void minus(char c)
{
  if(counterMinus == 3 && c != '-')
  {
    nodeArray[i++] = c;
    foundMinus = true;
  }
  if(c == '-' && !foundMinus)
  {
    counterMinus++;
  }
  else if(c == '-' && foundMinus)
  {

    node = octToDec(atoi(nodeArray));
    counterMinus = 0;
    i = 0;
    memset(nodeArray, 0, 10);
    foundMinus = false;
    Serial.println(node, OCT);
    
    while(!sendData(node, led))
      ;  


  }
  else if(c != '-' && foundMinus)
  {
    //DO NOTHING
  }
  else
  {
    counterMinus = 0;
  }
}
*/
int octToDec(int n)
{

  int result = 0;
  int length = intLength(n)-1;
  for(; length >= 0; length--)
  {
    int x = floor(n/exp(10,length));
    result = result + x*exp(8,length) ;
    n = n - x*exp(10,length);
  }
  return result;

}
int intLength (int value){
  int l=1;
  while(value>9){ 
    l++; 
    value/=10; 
  }
  return l;
}
int exp(int base, int expo)
{
  int result = 1;
  while(expo > 0)
  {
    result = result * base;
    expo--;
  }
  return result;
}







