#include <EEPROM.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

//EEPROM--------------------------
#define EEPROM_write(address, p) {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) EEPROM.write(address+i, pp[i]);}
#define EEPROM_read(address, p)  {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) pp[i]=EEPROM.read(address+i);}

#define POWER1 A0
#define POWER2 A1
#define POWER3 A2
#define POWER4 A3

#define led A5

struct config_type
{
  int EEPROM_NUM;
};

boolean EEPROM_STA=false,EEPROM_CACHE=false;

// nRF24L01(+) radio attached using Getting Started board 
RF24 radio(9,10);
// Network uses that radio
RF24Network network(radio);
// Address of our node
const uint16_t this_node = 0;
// Address of the other node
const uint16_t other_node = 1;
// Structure of our payload
struct payload_a
{
  uint32_t ms;
  uint32_t sensorDataA;
  uint32_t sensorDataB;
  uint32_t sensorDataC;
  uint32_t sensorDataD;
  uint32_t sensorDataX;
};

// ethernet interface mac address
boolean C[4],ledsta=false;
int tempX;
uint32_t time;

int NUM,ledNUM=0;
boolean STA,ledSTA=false;
long TIME_JS,TIME_LED;

int DELAY;

// called when the client request is complete


void setup(void)
{
  Serial.begin(9600);

  pinMode(POWER1,OUTPUT);
  pinMode(POWER2,OUTPUT);
  pinMode(POWER3,OUTPUT);
  pinMode(POWER4,OUTPUT);
  pinMode(led,OUTPUT);

  /*EEPROM读取赋值*/
  config_type config_readback;
  EEPROM_read(0, config_readback);

  NUM = config_readback.EEPROM_NUM;

  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ this_node);

}

void loop(void)
{
  digitalWrite(POWER1,C[0]);
  digitalWrite(POWER2,C[1]);
  digitalWrite(POWER3,C[2]);
  digitalWrite(POWER4,C[3]);

  digitalWrite(led,ledsta);

  if(time-TIME_LED>1000)
  {
    ledsta=!ledsta;
    TIME_LED=time;
  }
  // Pump the network regularly
  network.update();
  // Is there anything ready for us?
  while ( network.available() )
  {
    // If so, grab it and print it out
    RF24NetworkHeader header;
    payload_a payloada;
    network.read(header,&payloada,sizeof(payloada));

    time=payloada.ms;
    C[0]=payloada.sensorDataA;
    C[1]=payloada.sensorDataB;
    C[2]=payloada.sensorDataC;
    C[3]=payloada.sensorDataD;
    tempX=payloada.sensorDataX;
  }
}

void eeprom_write()
{
  config_type config;  		// 定义结构变量config，并定义config的内容
  config.EEPROM_NUM = NUM;

  EEPROM_write(0, config); 	// 变量config存储到EEPROM，地址0写入
}
