#include <globals.h>
#include <avr/pgmspace.h>
#include <nodeconfig.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <Ethernet.h>
#include <printf.h>
#include <Xtea.h>
#include <TextFinder.h>



char nodeArray[10] = {
};
char seekCharArray[] = {
  '-','*'};
char valueArray[10] = {
};
int valueCounter = 0;
int charCounter = 0;
bool foundValue = false;

/*
 * ETHERNET SETUP
 */
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
EthernetClient client;
TextFinder finder( client,1 );
IPAddress server(79,136,60,91);

unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
boolean lastConnected = false;                 // state of the connection last time through the main loop
const unsigned long postingInterval = 200;  // delay between updates, in milliseconds
int rootNodeID = 1111;
/*
 * RF24 SETUP
 */
// nRF24L01(+) radio attached using Getting Started board 
RF24 radio(6,7);

// Network uses that radio
RF24Network network(radio);
eeprom_info_t this_node;

//Ecryption
unsigned long key[4] = {KEY0,KEY1,KEY2,KEY3};
Xtea xtea(key);

bool test = true;
void setup(void)
{
  pinMode(2, INPUT);
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
  

  if (client.available()) {
    Serial.println(millis());
    finder.find("ToNode");  
    long value = finder.getValue(); 
    Serial.println(value);
    finder.find("Value"); 
    long value1 = finder.getValue(); 
    Serial.println(value1); 
    client.stop();
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
    httpRequest();
  }
  // store the state of the connection for next time through
  // the loop:
  lastConnected = client.connected();
}
// this method makes a HTTP connection to the server:
void httpPostSensorData(float value,int sensorID, int fromNode)
{
  Serial.println("\n post");
  if (client.connect(server, 80)) 
  {
    
    // send the HTTP PUT request:
   
    client.print("GET /postSensorData.php?");
    client.print("rootNodeID=");
    client.print(rootNodeID);
    client.print("&");
    client.print("fromNode=");
    client.print(fromNode);
    client.print("&");
    client.print("value=");
    client.print(value/100);
    client.print("&");
    client.print("sensorID=");
    client.print(sensorID);
    
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
  unsigned long message[2] = {payload.value,payload.sensorID};
  xtea.decrypt(message);
  Serial.print(" Value: ");
  Serial.print(message[0]);
  Serial.print(" SensorID: ");
  Serial.println(message[1]);

  //httpPostSensorData(value,sensorID,fromNode);

}
bool sendKey()
{
  Serial.print("Sending key...");
  unsigned long toNode = 5;
  payload_t payload = 
  { 
    key[0],
    key[1],
    key[2],
    key[3]
  };
  RF24NetworkHeader header(/*to node*/ toNode,SEND_KEY_CHAR );
  bool ok = network.write(header,&key,sizeof(key));
  if (ok)
    Serial.println("ok.");
  else
    Serial.println("failed.");
  return ok;
}
bool sendData(uint16_t toNode, unsigned long command,unsigned long value)
{
  Serial.print("Sending...");
  unsigned long message[2] = {command,value};
  xtea.encrypt(message);
  payload_t payload = 
  { 
    message[0],
    message[1],
    NO_COMMAND,
    NO_RESERV
  };
  RF24NetworkHeader header(/*to node*/ toNode, SEND_KEY_CHAR);
  bool ok = network.write(header,&payload,sizeof(payload));
  if (ok)
    Serial.println("ok.");
  else
    Serial.println("failed.");
  return ok;
}
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










