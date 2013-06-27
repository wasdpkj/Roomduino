#include <EEPROM.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

#define led 3
#define key 6
#define POWER 5

//EEPROM--------------------------
#define EEPROM_write(address, p) {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) EEPROM.write(address+i, pp[i]);}
#define EEPROM_read(address, p)  {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) pp[i]=EEPROM.read(address+i);}

struct config_type
{
  int EEPROM_NUM;
};

boolean EEPROM_STA=false,EEPROM_CACHE=false;

// nRF24L01(+) radio attached using Getting Started board 
RF24 radio(8,7); //设置24L01的CE和CSN引脚
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
boolean C[4];
int tempX;
uint32_t time;

int NUM,ledNUM=0;
boolean STA,ledSTA=false;
long TIME_JS,TIME_LED;

int DELAY;

void setup(void)
{
  Serial.begin(9600);

  pinMode(key,INPUT_PULLUP);		//上拉电阻
  pinMode(led,OUTPUT);
  pinMode(POWER,OUTPUT);

  /*EEPROM读取赋值*/
  config_type config_readback;
  EEPROM_read(0, config_readback);

  NUM = config_readback.EEPROM_NUM;

  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ this_node);
  if(NUM!=0 && NUM!=1 && NUM!=2 && NUM!=3)
  {
    NUM=0;
    eeprom_write();					//写入EEPROM
  }

}

void loop(void)
{
  digitalWrite(POWER,STA);
  digitalWrite(led,ledSTA);


  //指示灯----------------------------------
  if(millis()-TIME_LED>DELAY)
  {
    ledSTA=!ledSTA;
    ledNUM++;
    if(ledNUM>(NUM*2)+1)			//亮灭
    {
      ledSTA=false;
      DELAY=2500;					//间隔
      ledNUM=0;
    }
    else
      DELAY=300;

    TIME_LED=millis();
    /*
     Serial.print(" NUM:");
     Serial.print(ledNUM);
     Serial.print(" STA:");
     Serial.println(ledSTA);
     */
  }	


  //按键判断--------------------------------
  if(boolean(digitalRead(key)))		//配置键未按下时高电平
    TIME_JS=millis();

  if(millis()-TIME_JS>2000)			//超过两秒
  {
    if(tempX!=0)					//主机是否在配置状态
    {
      NUM=tempX-1;					//第几路
      EEPROM_CACHE=true;			//EEPROM状态判断开启
      ledSTA=!ledSTA;				//闪动
      delay(80);
      TIME_LED=millis();
    }
  }
  else
    EEPROM_STA=false;				//松开按键时


  if(EEPROM_CACHE!=EEPROM_STA)		//判断是否在按键松开时
  {
    eeprom_write();					//写入EEPROM
    EEPROM_CACHE=false;				//归为
  }

  EEPROM_STA=EEPROM_CACHE;			//同步

  STA=C[NUM];		//开关赋值


  // Pump the network regularly
  network.update();
  // Is there anything ready for us?
  while ( network.available() )
  {
    // If so, grab it and print it out
    RF24NetworkHeader header;
    payload_a payloada;
    network.read(header,&payloada,sizeof(payloada));

    if(payloada.ms-time>0)
    {
      time=payloada.ms;
      C[0]=payloada.sensorDataA;
      C[1]=payloada.sensorDataB;
      C[2]=payloada.sensorDataC;
      C[3]=payloada.sensorDataD;
      tempX=payloada.sensorDataX;
    }
  }

  /*
  Serial.print("            STA:");
   Serial.print(STA);
   Serial.print(" / TempA:");
   Serial.print(C[0]);
   Serial.print("  TempB:");
   Serial.print(C[1]);
   Serial.print("  TempC:");
   Serial.print(C[2]);
   Serial.print("  TempD:");
   Serial.print(C[3]);
   Serial.print("  TempX:");
   Serial.print(tempX);
   Serial.print("  Time:");
   Serial.println(time/1000);
   */
}

void eeprom_write()
{
  config_type config;  		// 定义结构变量config，并定义config的内容
  config.EEPROM_NUM = NUM;

  EEPROM_write(0, config); 	// 变量config存储到EEPROM，地址0写入
}

