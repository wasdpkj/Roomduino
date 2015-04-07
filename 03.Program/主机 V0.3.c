
/*LCD
 4
 5
 6
 7
 10
 3.3
 VCC
 GND
 */

/*
RX=0;
 TX=1;
 */

/*RF
 nRF24L01=A5;
 nRF24L01=A4;
 */

/*NET
 
 SCK = 13;
 MISO = 12;
 MOSI = 11;
 RS(CS) = 3;
 */

/*OTHER
 A3：红外接收
 18:DHT11
 20:SDA
 21:SCL
 */

#include <PKJ.h>

#include <IRremote.h>
#include <Wire.h>
#include <EtherCard.h>
#define NUM 4

#define jsEEPROM 1000   //EEPROM检查

#define OUT

PKJ pkj;

/*-----LCD------*/
#include "U8glib.h"
U8GLIB_PCD8544 u8g(10, 7, 5, 6, 4);		// SPI Com: SCK = 13, MOSI = 11, CS = 10, A0 = 9, Reset = 8

#define lcd_delay 5
int lcd_show=0;
/*-----MENU-----*/
boolean  KEY_MENU_enter = false, KEY_MENU_down1x = false, KEY_MENU_up1x = false, KEY_MENU_down10x = false, KEY_MENU_up10x = false;

int MENU_xuan0, MENU_xuan1, MENU_xuan2, MENU_xuan3;
boolean MENU_xuan0sta, MENU_xuan1sta, MENU_xuan2sta, MENU_xuan3sta;
int MENUnum0, MENUnum2;
int MENUval = 0;

boolean setmod = false;
boolean MOD = false, MOD_TS;
long MOD_TSmil;

int tk=0;

/*----EEPROM---*/
#include <EEPROM.h>
#define EEPROM_write(address, p) {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) EEPROM.write(address+i, pp[i]);}
#define EEPROM_read(address, p)  {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) pp[i]=EEPROM.read(address+i);}

struct config_type
{
  int EEP_C_timesetshi[NUM];
  int EEP_C_timesetfen[NUM];
  int EEP_C_timesetdelay[NUM];
  int EEP_C_dsset[NUM];
  boolean EEP_C_timesta[NUM];
};

/*-------net------*/
static byte mymac[] =
{
  0x2C, 0x81, 0x58, 0x3C, 0xC5, 0x60
};
static byte myip[] =
{
  192, 168, 1, 121
};
static byte gwip[] =
{
  192, 168, 1, 1
};
static byte dnsip[] =
{
  192, 168, 1, 1
};

byte Ethernet::buffer[900];


//YEELINK---------------------------------------------
#define NET_YEE_request 1400 // milliseconds

char website[] PROGMEM = "api.yeelink.net";

char urlBuf0[] PROGMEM = "/v1.0/device/xx/sensor/xx/";  //mod-switch

char urlBuf1[] PROGMEM = "/v1.0/device/xx/sensor/xx/";
char urlBuf2[] PROGMEM = "/v1.0/device/xx/sensor/xx/";
char urlBuf3[] PROGMEM = "/v1.0/device/xx/sensor/xx/";
char urlBuf4[] PROGMEM = "/v1.0/device/xx/sensor/xx/";

char urlBuf5[] PROGMEM = "/v1.0/device/xx/sensor/xx/";  //hum
char urlBuf6[] PROGMEM = "/v1.0/device/xx/sensor/xx/";  //tem

char apiKey[] PROGMEM = "U-ApiKey: xx";

boolean NET_REPLY = false;
boolean NET_YEE_sw = false;

int yee[NUM+1];
int yeenum = 0;

static long NET_YEE_mil;


/*---SEND---*/
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

//* nRF24L01(+) radio attached using Getting Started board *//
RF24 radio(A5, A4);
// Network uses that radio
RF24Network network(radio);
// Address of our node
const uint16_t this_node = 1;
// Address of the other node
const uint16_t other_node = 0;
// How often to send 'hello world to the other unit
const unsigned long interval = 300; //ms
// When did we last send?
unsigned long last_sent;

struct send_a
{
  uint32_t ms;
  uint32_t sensorDataA;
  uint32_t sensorDataB;
  uint32_t sensorDataC;
  uint32_t sensorDataD;
  uint32_t sensorDataX;
};

/*-------time------*/
#define SECONDS_IN_DAY          86400
#define START_YEAR              1900
#define TIME_ZONE               +8
#define TIME_OUT               10000
#define TIME_REFRESH           300

int timenian, timeyue, timeri, timeshi, timefen, timemiao, timezhou;  //时钟

static int days_in_month[] =
{
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static byte ntpServer[] =
{
  193, 204, 114, 232
};
static byte srcPort = 0;

uint32_t timeStamp;
boolean NET_TIME_reply;
boolean NET_WEBSITE_sta;

#define NET_TIME_INTERVAL                1000
#define NET_CS           60000
#define NET_RP           300000
unsigned long TIME_cache = 0, TIME, NET_TIME_cs, NET_TIME_rp;

/*-------timing----*/
boolean KEY_CH_up =  false, KEY_CH_down = false , KEY_CH_ok =  false; //定时2路

long TIME_loop0,TIME_loop1;

/*-------dht11----*/
#include <dht11.h>
dht11 DHT11;
#define DHT11PIN 18 //DHT11 PIN

#define dht11_delay 100
int tem,hum;
int dht11_read=0;
boolean dht11_send=false;

/*-------ir------*/
#define RECV_PIN  A3    //红外输入

IRrecv irrecv(RECV_PIN);
decode_results results;

//===========VOL
boolean C[NUM];
int C_SW;
//-------------------
int C_timesetshi[NUM];
int C_timesetfen[NUM];
int C_timesetdelay[NUM];
boolean C_timesta[NUM];

boolean C_timedelaysta[NUM];
long C_timemil[NUM];
//-------------------
int C_dsset[NUM];

boolean C_dssta[NUM];
boolean C_dsgo[NUM];
int C_dslast[NUM];
long C_dsmil[NUM];

int C_dssw = 0;
//-------------------
int light;      //风扇档位
int mod[4];      //风扇档位


void setup()
{
  Serial.begin(9600);

  digitalWrite(12, 1);
  delay(10);

  u8g.setRot180();

  volcdsetup("Via PKJ  2013.6.2",10,15);
  delay(2000);

  //EEPROM-------------------------------------------------  
  volcdsetup("Load EEPROM",10,15);
  delay(1000);

  config_type config_readback;
  EEPROM_read(0, config_readback);

  for(int a = 0; a < NUM; a++) //设置时间上下限
  {
    C_timesetshi[a] = config_readback.EEP_C_timesetshi[a];
    C_timesetfen[a] = config_readback.EEP_C_timesetfen[a];
    C_timesetdelay[a] = config_readback.EEP_C_timesetdelay[a];
    C_dsset[a] = config_readback.EEP_C_dsset[a];
    C_timesta[a] = config_readback.EEP_C_timesta[a];
  }

  //NET------------------------------------------------------
  volcdsetup("Load NETWORK",10,15);
  delay(1000);

  ether.begin(sizeof Ethernet::buffer, mymac, 3);

  ether.staticSetup(myip, gwip, dnsip);

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);
  ether.printIp("DNS: ", ether.dnsip);

  if(ether.dnsLookup(website))
  {
    volcdsetup("Yeelink OK",10,15);
    delay(1000);

    NET_WEBSITE_sta = true;
  }
  else
  {
    volcdsetup("Yeelink ERROR",10,15);
    delay(1000);

    NET_WEBSITE_sta = false;
  }

  volcdsetup("Load Net Time",10,15);
  delay(1000);

  NET_TIME_reply = true;
  vonettime();

  if(NET_TIME_reply)
  {
    volcdsetup("Net Time ERROR",10,15);
    delay(1000);
  }
  else
  {
    volcdsetup("Net Time OK",10,15);
    delay(1000);
  }
  /*
      xx while (ether.clientWaitingGw())
   ether.packetLoop(ether.packetReceive());
   */
  volcdsetup("Load nRF",10,15);
  delay(1000);

  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ this_node);

  NET_YEE_mil = millis(); // start timing out right away


  volcdsetup("Load IR",10,15);
  delay(1000);

  irrecv.enableIRIn();        //开始采集

  volcdsetup("WELCOME!",10,15);
  delay(1000);

}


void loop()
{
  TIME_loop0=millis();

  KEY_MENU_enter = false;
  KEY_MENU_down1x = false;
  KEY_MENU_up1x = false;
  KEY_MENU_down10x = false;
  KEY_MENU_up10x = false;

  KEY_CH_ok =  false;
  KEY_CH_up =  false;
  KEY_CH_down = false;

  MOD_TS = false;

  /*==================Time======================*/
  if (millis() - TIME_cache > TIME_REFRESH)        //设置TIME刷新频率
  {
    TIME_cache = millis();
    printDate(uint32_t (timeStamp + 3600 * TIME_ZONE + ((millis() - TIME) / 1000)));
  }

  /*=================end-Time==================*/


  /*=================红外控制=================*/
  if (irrecv.decode(&results))
  {
    voir();
  }

  /*==============end-红外控制=================*/


  /*====================DHT11================*/
  dht11_read++;
  dht11_read=pkj.VOLtiaobian(dht11_read, 0, dht11_delay);
  if(dht11_read==dht11_delay)
  {
    int chk = DHT11.read(DHT11PIN);
    hum=(int)DHT11.humidity;
    tem=(int)DHT11.temperature;
  }
  /*====================end-DHT11================*/

  /*================重连服务======================*/
  if(!setmod)
  {
    if(NET_TIME_reply == true && NET_REPLY == false)    //如果setup里时间没连接成功，先重连时间
    {
      if(millis() - NET_TIME_cs > NET_CS)
      {
        vonettime();                      //同步时间

        NET_TIME_cs = millis();
        NET_REPLY = true;                        //切换到另一个
      }
    }

    if(NET_WEBSITE_sta == false && NET_REPLY == true)     //如果setup里yeelink服务器没连接上，重连
    {

      if(millis() - NET_TIME_cs > NET_CS)
      {
        if(ether.dnsLookup(website))      //连接
        {
          yeenum = 0;
          NET_WEBSITE_sta = true;            //搞定
        }
        else
          NET_WEBSITE_sta = false;           //没搞定

        NET_TIME_cs = millis();
        NET_REPLY = false;                       //切换到另一个
      }
    }

    if(millis() - NET_TIME_rp > NET_RP)
    {
      if(ether.dnsLookup(website))      //连接
      {
        yeenum = 0;
        NET_WEBSITE_sta = true;            //搞定
      }
      else
        NET_WEBSITE_sta = false;           //没搞定

      NET_TIME_rp = millis();
    }
  }
  /*===============end-重连服务================*/


  /*==================net=====================*/
  ether.packetLoop(ether.packetReceive());
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);

  if(pos)
  {
    if(strstr((char *)Ethernet::buffer + pos, "POST") != 0)
    {
      sscanf((char *)strstr((char *)Ethernet::buffer + pos, "lightstatus"), "lightstatus=%d", &light);
      sscanf((char *)strstr((char *)Ethernet::buffer + pos, "mod"), "mod=%d", &mod[0]);
    }


    switch(mod[0])
    {
    case 00:
      MOD=false;
      NET_YEE_sw = true;
      yeenum = 0;
      break;
    case 01:
      MOD=true;
      NET_YEE_sw = true;
      yeenum = 0;
      break;
    }

    if(!MOD)
    {
      if((light%10))
        C_dssta[int(light/10)-1]=false;

      if(light!=0)
      {
        switch(light)
        {
        case 10:
          if(!MOD)
            C[0] = false;
          break;
        case 11:
          if(!MOD)
            C[0] = true;
          break;
        case 20:
          if(!MOD)
            C[1] = false;
          break;
        case 21:
          if(!MOD)
            C[1] = true;
          break;
        case 30:
          if(!MOD)
            C[2] = false;
          break;
        case 31:
          if(!MOD)
            C[2] = true;
          break;
        case 40:
          if(!MOD)
            C[3] = false;
          break;
        case 41:
          if(!MOD)
            C[3] = true;
          break;
        }
      }
      light=0;
    }
    else
      MOD_TS=true;

    BufferFiller bfill = ether.tcpOffset();
    bfill.emit_p(PSTR("HTTP/1.1 200 OK\r\n"
      "Content-Type: text/html\r\n"
      "Pragma: no-cache\r\n\r\n"
      "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"/>"
      "<title></title><style type=\"text/css\"> body {background-color:black;} .STYLE0 {background-color:#666666;}.STYLE1 {background-color:#0000FF;} </style></head>"
      "<body><table width=\"500\" border=\"0\"><tr>"
      "<td width=\"102\" height=\"80\" class=\"STYLE$D\"></td><td width=\"102\" height=\"80\" class=\"STYLE$D\"></td>"
      "<td width=\"102\" height=\"80\" class=\"STYLE$D\"></td><td width=\"102\" height=\"80\" class=\"STYLE$D\"></td>"
      "</tr></table>"
      "<table width=\"500\" border=\"0\"><tr><td height=\"10\" class=\"STYLE$D\"></td></tr></table>"
      "</body></html>"), C[0], C[1], C[2], C[3],MOD);
    ether.httpServerReply(bfill.position());
  }
  //==================endnet===================//

  //----------------------------------------------------------------
  if(NET_WEBSITE_sta)
  {
    if (millis() - NET_YEE_mil > NET_YEE_request)    //采集节点时间
    {
      ether.printIp("Server: ", ether.hisip);   //服务器握手

      yeenum++;   //顺序递增
      if(yeenum > 5)
        yeenum = 0;

      switch(yeenum)                            //顺序采集
      {
      case 0:
        if(!NET_YEE_sw)     //网络模式判断未激活时，先执行这个
        {
          ether.browseUrl(urlBuf0, "datapoints", website, apiKey, my_result_cb);  //取得状态
          if(MOD != yee[0])   //如果网络模式改变，归零
          {
            yee[1] = 0, yee[2] = 0, yee[3] = 0, yee[4] = 0;
            C[0] = false, C[1] = false, C[2] = false, C[3] = false;
          }
          MOD = yee[0];   //采集网络模式状态
        }
        else    //网络模式判断激活时，先执行这个，把网络模式状态推出去
        {
          if(MOD != yee[0])
          {
            if(!MOD)
              ether.httpPost (urlBuf0, "datapoints", website, apiKey, "{\"value\":0}", my_result_cb);
            else
              ether.httpPost (urlBuf0, "datapoints", website, apiKey, "{\"value\":1}", my_result_cb);
          }
          NET_YEE_sw = false;   //执行完毕
        }
        break;

      case 1:
        if(MOD)
          ether.browseUrl(urlBuf1, "datapoints", website, apiKey, my_result_cb);    //采集节点
        else
          ether.httpPost (urlBuf1, "datapoints", website, apiKey, "{\"value\":0}", my_result_cb);    //把节点推到关
        break;
      case 2:
        if(MOD)
          ether.browseUrl(urlBuf2, "datapoints", website, apiKey, my_result_cb);   //采集节点
        else
          ether.httpPost (urlBuf2, "datapoints", website, apiKey, "{\"value\":0}", my_result_cb);    //把节点推到关
        break;
      case 3:
        if(MOD)
          ether.browseUrl(urlBuf3, "datapoints", website, apiKey, my_result_cb);    //采集节点
        else
          ether.httpPost (urlBuf3, "datapoints", website, apiKey, "{\"value\":0}", my_result_cb);    //把节点推到关
        break;
      case 4:
        if(MOD)
          ether.browseUrl(urlBuf4, "datapoints", website, apiKey, my_result_cb);    //采集节点
        else
          ether.httpPost (urlBuf4, "datapoints", website, apiKey, "{\"value\":0}", my_result_cb);    //把节点推到关
        break;
      case 5:
        dht11_send=!dht11_send;
        static char buf[20];
        get_send_string(buf);
        if(dht11_send)
          ether.httpPost (urlBuf5, "datapoints", website, apiKey, buf, my_result_cb);    //把节点推到关
        else
          ether.httpPost (urlBuf6, "datapoints", website, apiKey, buf, my_result_cb);    //把节点推到关
        break;
      }

      NET_YEE_mil = millis();
    }

    if(MOD)      //开启网络模式时
    {
      C[0] = yee[1];
      C[1] = yee[2];
      C[2] = yee[3];
      C[3] = yee[4];
    }

  }
  /*===================MOD===================*/
  if(MOD_TS)
    MOD_TSmil = millis();

  /*=================end-MOD=================*/

  if(!MOD)
  {
    /*================定时开启===================*/
    for(int a = 0; a < NUM; a++) //设置时间上下限
    {
      C_timesetshi[a] = pkj.VOLtiaobian(C_timesetshi[a], 0, 23);
      C_timesetfen[a] = pkj.VOLtiaobian(C_timesetfen[a], 0, 59);
      C_timesetdelay[a] = pkj.VOLtiaobian(C_timesetdelay[a], 1, 360);
    }

    for(int a = 0; a < NUM; a++) //触发
    {
      if(!C_dssta[a] && C_timesta[a] == true && timeshi == C_timesetshi[a] && timefen == C_timesetfen[a] && timemiao == 1)		//没有在定时时，可触发
      {
        C[a] = true;

        C_timedelaysta[a] = true; //持续函数状态
        C_timemil[a] = millis(); //记下开始时间
      }
    }

    for(int a = 0; a < NUM; a++) //被触发时
    {
      if(C_timedelaysta[a])
      {
        if((millis() - C_timemil[a]) / 1000 / 60 > C_timesetdelay[a])			//超过延时时关闭
        {
          C[a] = false;

          C_timedelaysta[a] == false; //持续函数状态
        }
      }
    }

    //如果某动作需要打断定时，使C_timedelaysta[]函数为假即可

    /*==============end-定时开启================*/


    /*================倒计时===================*/
    if (KEY_CH_up)     //控制数值上升
    {
      C_dssw++;
    }
    if (KEY_CH_down)    //控制数值下降
    {
      C_dssw--;
    }

    C_dssw = pkj.VOLtiaobian(C_dssw, 0, 3);

    for(int a = 0; a < NUM; a++) //设置时间上下限
    {
      C_dsset[a] = pkj.VOLtiaobian(C_dsset[a], 1, 90);
      C_dsgo[a] = false;
    }

    if(KEY_CH_ok)
    {
      C_dsgo[C_dssw] = !C_dsgo[C_dssw];
      C_timedelaysta[C_dssw] = false;					//如果开启倒计时，则打断相应定时
      //C_timesta[C_dssw]=false
    }

    for(int a = 0; a < NUM; a++) //倒计时
    {
      if (C_dsgo[a] == true && (C_dssta[a] == false && C[a] == false))    //C[]为零时可触发计时
      {
        C_dsmil[a] = millis();
        C[a] = true;          //开始执行
        C_dssta[a] = true;    //开始计时
      }

      else if (C_dsgo[a] == true && (C_dssta[a] == true || C[a] == true))   //C[]为1时或计时开始后，再次触发则停止计时、
      {
        C[a] = false;
        C_dssta[a] = false;   //停止计时
      }

      if((millis() - C_dsmil[a]) / 1000 > C_dsset[a] * 60 && C_dssta[a] == true) //大于设置
      {
        C[a] = false;         //停止执行
        C_dssta[a] = false;   //停止计时
      }

      C_dslast[a] = (C_dsset[a] * 60 - (millis() - C_dsmil[a]) / 1000);
    }
    /*=============end-倒计时===================*/
  }

  /*=====================MENU===================*/
  vomenu();

  /*====================end-MENU================*/

  /*=====================LCD===================*/
  lcd_show++;
  lcd_show=pkj.VOLtiaobian(lcd_show, 0, lcd_delay);
  if(lcd_show==lcd_delay)
  {

    u8g.firstPage();
    do
    {
      if(setmod)
        menu();
      else
        draw();
    }
    while( u8g.nextPage() );
  }

  /*===================end-LCD================*/


  /*==================SEND======================*/
  vosend();

  /*================end-SEND=====================*/

  TIME_loop1=millis()-TIME_loop0;
}

void get_send_string(OUT char *p)
{
  int cache;

  switch(dht11_send)
  {
  case false:
    cache = hum;
    break;
  case true:
    cache = tem;
    break;
  }

  sprintf(p, "{\"value\":%d}", cache);
}

void voir()
{
  unsigned long remote = results.value;   //设红外信号为remote

    if(remote == (-1))    //溢出时
  {
    //        Serial.print(results.value, HEX);   //输出红外线解码结果（十六进制）
  }
  switch (remote)
  {
  case 0xFF629D:      //CH
    if(!MOD)
      KEY_CH_ok = true;
    else
      MOD_TS = true;
    break;

  case 0xFFE21D:      //CH+
    if(!MOD)
      KEY_CH_up = true;
    else
      MOD_TS = true;
    break;

  case 0xFFA25D:      //CH-
    if(!MOD)
      KEY_CH_down = true;
    else
      MOD_TS = true;
    break;

  case 0xFFC23D:      //PLAY
    MOD = !MOD;
    NET_YEE_sw = true;
    break;

  case 0xFF906F:      //EQ
    setmod = !setmod;
    MENU_xuan0sta = true;
    MENU_xuan1sta = false;
    MENU_xuan2sta = false;
    MENU_xuan3sta = false;
    MENU_xuan0 = 0;
    C_SW=0;
    if(!setmod)
      eeprom_test();
    NET_TIME_cs = millis();
    break;

  case 0xFF6897:      //0
    KEY_MENU_enter = true;
    break;

  case 0xFFE01F:      //-
    KEY_MENU_down1x = true;
    break;

  case 0xFFA857:      //+
    KEY_MENU_up1x = true;
    break;

  case 0xFF22DD:      //-10
    KEY_MENU_down10x = true;
    break;

  case 0xFF02FD:      //+10
    KEY_MENU_up10x = true;
    break;

  case 0xFF30CF:      //1
    if(!MOD)
    {
      C[0] = !C[0];
      if(!C[0])
        C_dssta[0]=false;
    }
    else
      MOD_TS = true;
    break;

  case 0xFF18E7:      //2
    if(!MOD)
    {
      C[1] = !C[1];
      if(!C[1])
        C_dssta[1]=false;
    }
    else
      MOD_TS = true;
    break;

  case 0xFF7A85:      //3
    if(!MOD)
    {
      C[2] = !C[2];
      if(!C[2])
        C_dssta[2]=false;
    }
    else
      MOD_TS = true;
    break;

  case 0xFF10EF:      //4
    if(!MOD)
    {
      C[3] = !C[3];
      if(!C[3])
        C_dssta[3]=false;
    }
    else
      MOD_TS = true;
    break;
  }


  irrecv.resume();    // 返回值
}


void eeprom_test()
{
  config_type config;     // 定义结构变量config，并定义config的内容

  for(int a = 0; a < NUM; a++)
  {
    config.EEP_C_timesetshi[a] = C_timesetshi[a];
    config.EEP_C_timesetfen[a] = C_timesetfen[a];
    config.EEP_C_timesetdelay[a] = C_timesetdelay[a];
    config.EEP_C_dsset[a] = C_dsset[a];
    config.EEP_C_timesta[a] = C_timesta[a];
  }
  EEPROM_write(0, config);  // 变量config存储到EEPROM，地址0写入
}


void voadd(int add)
{
  if(add < 10)
  {
    u8g.print("0");
  }
}


void printDate(uint32_t timeStamp)
{
  unsigned int year = START_YEAR;
  while(1)
  {
    uint32_t seconds;
    if(isLeapYear(year)) seconds = SECONDS_IN_DAY * 366;
    else seconds = SECONDS_IN_DAY * 365;
    if(timeStamp >= seconds)
    {
      timeStamp -= seconds;
      year++;
    }
    else break;
  }

  unsigned int month = 0;
  while(1)
  {
    uint32_t seconds = SECONDS_IN_DAY * days_in_month[month];
    if(isLeapYear(year) && month == 1) seconds = SECONDS_IN_DAY * 29;
    if(timeStamp >= seconds)
    {
      timeStamp -= seconds;
      month++;
    }
    else break;
  }
  month++;

  unsigned int day = 1;
  while(1)
  {
    if(timeStamp >= SECONDS_IN_DAY)
    {
      timeStamp -= SECONDS_IN_DAY;
      day++;
    }
    else break;
  }

  unsigned int hour = timeStamp / 3600;
  unsigned int minute = (timeStamp - (uint32_t)hour * 3600) / 60;
  unsigned int second = (timeStamp - (uint32_t)hour * 3600) - minute * 60;

  timenian = year;
  timeyue = month;
  timeri = day;
  timeshi = hour;
  timefen = minute;
  timemiao = second;
}

boolean isLeapYear(unsigned int year)
{
  return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

void vonettime()
{
  long timeout = millis();    //超时时间
  while(NET_TIME_reply == true && millis() - timeout < TIME_OUT)   //采集到或者超时时跳出
  {
    ether.packetLoop(ether.packetReceive());
    if(millis() - TIME_cache > NET_TIME_INTERVAL)     //发送请求间隔
    {
      ether.ntpRequest(ntpServer, srcPort);
      TIME_cache = millis();
      NET_TIME_reply = true;   //标记发送
    }

    if(NET_TIME_reply && ether.ntpProcessAnswer(&timeStamp, srcPort))    //发送后等待应答
    {
      printDate(timeStamp + 3600 * TIME_ZONE);    //开始计算
      TIME_cache = millis();
      TIME = millis();        //送到主函数参与时间运算
      NET_TIME_reply = false;    //接收到了
    }
  }
}


static void my_result_cb (byte status, word off, word len)
{
  if(yeenum!=5)
  sscanf((char *)strstr((const char *)Ethernet::buffer + off, "\"value\""), "\"value\":%d", &yee[yeenum]);    //采集关键字数据
}


void draw(void)
{
  u8g.setFont(u8g_font_chikitar);
  //u8g.setFont(u8g_font_fixed_v0r);
  //u8g.setFont(u8g_font_7x13);
  //u8g.setFont(u8g_font_osb21);
  //u8g.drawStr( 0, 18, "Hello!");

  u8g.drawLine(37, 1,   37, 39);
  u8g.drawLine(47, 1,   47, 39);

  u8g.drawLine(0, 41,   83, 41);


  for(int a = 0; a < NUM; a++) //设置时间上下限
  {
    u8g.setPrintPos(2, 8 + 10 * a);
    switch(a)
    {
    case 0:
      u8g.print("C1:");
      break;
    case 1:
      u8g.print("C2:");
      break;
    case 2:
      u8g.print("C3:");
      break;
    case 3:
      u8g.print("C4:");
      break;
    }

    u8g.setPrintPos(18, 8 + 10 * a);
    if(C[a])
      u8g.print("ON");
    else
      u8g.print("OFF");

    if(C_timesta[a])
      u8g.drawBox(39, 2 + 10 * a, 7,6);
    else
    {
      u8g.drawFrame(39, 2 + 10 * a, 7,6);
    }
  }

  u8g.drawFrame(0, 0 + 10 * C_dssw, 83, 10);

  for(int a = 0; a < NUM; a++) //设置时间上下限
  {
    u8g.setPrintPos(50, 8 + 10 * a);
    if(C_dssta[a])
    {
      voadd(C_dslast[a] / 60);
      u8g.print(C_dslast[a] / 60);
      u8g.print(":");
      voadd(C_dslast[a] % 60);
      u8g.print(C_dslast[a] % 60);
    }
    else
    {
      u8g.print(C_dsset[a]);
      u8g.print("min");
    }
  }

  u8g.setFont(u8g_font_chikitar);
  u8g.setPrintPos(0, 48);
  if(millis() - MOD_TSmil < 3000)
  {
    if(millis() / 800 % 2 != 0)
      u8g.print("MOD:");
    else
      u8g.print("XXX:");
  }
  else
    u8g.print("MOD:");
  switch(MOD)
  {
  case false:
    u8g.print("Def.");
    break;
  case true:
    u8g.print("Yee.");
    break;
  }

  u8g.setPrintPos(56, 48);

voadd(timeshi);
  u8g.print(timeshi);    //hor
  if((millis() / 1000) % 2 == 0)
    u8g.print(":");
  else
    u8g.print(".");
  voadd(timefen);
  u8g.print(timefen);    //min
}

void menu(void)
{
  u8g.setFont(u8g_font_chikitar);
  //  u8g.setFont(u8g_font_fixed_v0r);
  //  u8g.setFont(u8g_font_7x13);
  //  u8g.setFont(u8g_font_osb21);
  //  u8g.drawStr( 0, 18, "Hello!");

  //    u8g.drawFrame(0,0,83,48);

  //-----------------------------------------------
  if(MENU_xuan0sta)								//进入初级菜单后
  {
    u8g.drawStr(30, 6, "MENU");

    u8g.drawFrame(0, 8 + (8 * MENU_xuan0), 83, 8);		//选框

    u8g.setPrintPos(5, 15 + 8 * 0);
    u8g.print("CONFIG MOD");
    u8g.setPrintPos(5, 15 + 8 * 1);
    u8g.print("CONFIG TIME");
    u8g.setPrintPos(5, 15 + 8 * 2);
    u8g.print("CONFIG DS");
    u8g.setPrintPos(5, 15 + 8 * 3);
    u8g.print("CONFIG CON");
    u8g.setPrintPos(5, 15 + 8 * 4);
    u8g.print("CONFIG INFO");
  }

  //-----------------------------------------------
  if(MENU_xuan1sta)								//进入一级菜单后
  {
    u8g.drawFrame(0, 0 + (8 * MENU_xuan1), 83, 8);		//选框

    u8g.setPrintPos(5, 7 + 8 * 0);
    u8g.print("...UP^");

    switch(MENU_xuan0)							//读取上级选中菜单
    {
    case 0:
      MENU_xuan1 = pkj.VOLtiaobian(MENU_xuan1, 0, 1);
      u8g.setPrintPos(5, 7 + 8 * 1);
      u8g.print("SYSTEM^MOD" );

      MENUnum0 = 10;								//标记本层菜单
      break;
    case 1:
      MENU_xuan1 = pkj.VOLtiaobian(MENU_xuan1, 0, 4);
      u8g.setPrintPos(5, 7 + 8 * 1);
      u8g.print("TIME^C1");
      u8g.setPrintPos(5, 7 + 8 * 2);
      u8g.print("TIME^C2");
      u8g.setPrintPos(5, 7 + 8 * 3);
      u8g.print("TIME^C3");
      u8g.setPrintPos(5, 7 + 8 * 4);
      u8g.print("TIME^C4");

      MENUnum0 = 20;								//标记本层菜单
      break;
    case 2:
      MENU_xuan1 = pkj.VOLtiaobian(MENU_xuan1, 0, 4);
      u8g.setPrintPos(5, 7 + 8 * 1);
      u8g.print("DS^C1");
      u8g.setPrintPos(5, 7 + 8 * 2);
      u8g.print("DS^C2");
      u8g.setPrintPos(5, 7 + 8 * 3);
      u8g.print("DS^C3");
      u8g.setPrintPos(5, 7 + 8 * 4);
      u8g.print("DS^C4");

      MENUnum0 = 30;								//标记本层菜单
      break;
    case 3:
      MENU_xuan1 = pkj.VOLtiaobian(MENU_xuan1, 0, 4);
      u8g.setPrintPos(5, 7 + 8 * 1);
      u8g.print("CON^C1");
      u8g.setPrintPos(5, 7 + 8 * 2);
      u8g.print("CON^C2");
      u8g.setPrintPos(5, 7 + 8 * 3);
      u8g.print("CON^C3");
      u8g.setPrintPos(5, 7 + 8 * 4);
      u8g.print("CON^C4");

      MENUnum0 = 40;								//标记本层菜单
      break;
    case 4:
      MENU_xuan1 = pkj.VOLtiaobian(MENU_xuan1, 0, 2);
      u8g.setPrintPos(5, 7 + 8 * 1);
      u8g.print("About");
      u8g.setPrintPos(5, 7 + 8 * 2);
      u8g.print("SYS^INFO");

      MENUnum0 = 50;								//标记本层菜单
      break;
    }

  }

  //-----------------------------------------------
  if(MENU_xuan2sta)								//进入二级菜单后
  {
    u8g.drawFrame(0, 0 + (8 * MENU_xuan2), 83, 8);

    u8g.setPrintPos(5, 7 + 8 * 0);
    u8g.print("...UP^");

    switch(MENUnum0)								//读取上级选中菜单
    {
    case 10:									//模式
      MENU_xuan2 = pkj.VOLtiaobian(MENU_xuan2, 0, 2);
      u8g.setPrintPos(5, 7 + 8 * 1);
      u8g.print("Default");
      u8g.setPrintPos(5, 7 + 8 * 2);
      u8g.print("Yeelink");
      switch(MENU_xuan2)						//标记本层菜单
      {
      case 0:
        MENUnum2 = 10;
        break;
      case 1:
        MENUnum2 = 11;
        break;
      case 2:
        MENUnum2 = 12;
        break;
      };
      break;

    case 20:									//定时
      MENU_xuan2 = pkj.VOLtiaobian(MENU_xuan2, 0, 4);
      u8g.setPrintPos(5, 7 + 8 * 1);
      u8g.print("SWITCH:");
      if(C_timesta[MENU_xuan1-1])
        u8g.print("ON");
      else
        u8g.print("OFF");

      u8g.setPrintPos(5, 7 + 8 * 2);
      u8g.print("HOUR:");
      u8g.print(C_timesetshi[MENU_xuan1-1]);
      u8g.setPrintPos(5, 7 + 8 * 3);
      u8g.print("MIN:");
      u8g.print(C_timesetfen[MENU_xuan1-1]);
      u8g.setPrintPos(5, 7 + 8 * 4);
      u8g.print("DELAY:");
      u8g.print(C_timesetdelay[MENU_xuan1-1]);

      switch(MENU_xuan2)						//标记本层菜单
      {
      case 0:
        MENUnum2 = 20;
        break;
      case 1:
        MENUnum2 = 21;
        break;
      case 2:
        MENUnum2 = 22;
        break;
      case 3:
        MENUnum2 = 23;
        break;
      case 4:
        MENUnum2 = 24;
        break;
      };

      break;

    case 30:									//倒计时
      MENU_xuan2 = pkj.VOLtiaobian(MENU_xuan2, 0, 1);
      u8g.setPrintPos(5, 7 + 8 * 1);
      u8g.print("TIMING:");
      u8g.print(C_dsset[MENU_xuan1-1]);

      switch(MENU_xuan2)						//标记本层菜单
      {
      case 0:
        MENUnum2 = 30;
        break;
      case 1:
        MENUnum2 = 31;
        break;
      };

      break;

    case 40:
      //      MENU_xuan2 = pkj.VOLtiaobian(MENU_xuan2, 0, 1);
      MENU_xuan2=0;
      u8g.setPrintPos(5, 7 + 8 * 1);
      u8g.print("Push ");
      u8g.print("NO.");
      u8g.print(MENU_xuan1);
      u8g.print("  2s");

      switch(MENU_xuan1)						//标记本层菜单
      {
      case 0:
        C_SW = 0;
        break;
      case 1:
        C_SW = 1;
        break;
      case 2:
        C_SW = 2;
        break;
      case 3:
        C_SW = 3;
        break;
      case 4:
        C_SW = 4;
        break;
      };

      break;

    case 50:									//倒计时
      MENU_xuan2 = pkj.VOLtiaobian(MENU_xuan2, 0, 1);
      u8g.setPrintPos(5, 7 + 8 * 1);
      switch(MENU_xuan1)
      {
      case 1: 
        {
          MENU_xuan2=0;

          int p=(millis() / 1000) % 2;

          u8g.print("Via PKJ 2013.05")  ;
          u8g.setPrintPos(5, 7 + 8 * 2);  
          u8g.print("Version 1.0")  ;
          u8g.setPrintPos(5, 7 + 8 * 3);  
          u8g.print("Thanks:")  ;
          u8g.setPrintPos(5, 7 + 8 * 4);  
          switch(tk)
          {
          case 0:
            u8g.print("    Olikraus")  ;
            break;
          case 1:
            u8g.print("    James Coliz")  ;
            break;
          case 2:
            u8g.print("    Guido Socher")  ;
            break;
          case 3:
            u8g.print("    Andrew Lindsay")  ;
            break;
          case 4:
            u8g.print("    Pascal Stang")  ;
            break;
          case 5:
            u8g.print("    zcb")  ;
            break;
          case 6:
            u8g.print("    yeelink")  ;
            break;
          }
          if((millis() / 1000) % 2 != p)
            tk++;
          if(tk>6)
            tk=0;
        }
        ;
        break;
      case 2: 
        MENU_xuan2=0;
        //        char* ip=(char*)ether.myip;
        //        char* dns=(char*)ether.dnsip;
        //        char* service=(char*)ether.hisip;
        u8g.print("FreeRAM: ");
        u8g.print(freeRam());
        u8g.setPrintPos(5, 7 + 8 * 2);  
        u8g.print("Loop Time: ");
        u8g.print(TIME_loop1);
        u8g.print("ms");
        //        u8g.setPrintPos(5, 7 + 8 * 3);
        //        u8g.print("DNS:");
        //        u8g.print(dns);
        //        u8g.setPrintPos(5, 7 + 8 * 4);
        //        u8g.print("Service:");
        //        u8g.print(service);
        u8g.setPrintPos(5, 7 + 8 * 3);
        u8g.print("Yeelink: ");
        if(NET_WEBSITE_sta)
          u8g.print(" OK ");
        else
          u8g.print("ERROR");
        u8g.setPrintPos(5, 7 + 8 * 4);
        u8g.print("Hum: ");
        u8g.print(hum);
        u8g.print(",Tem: ");
        u8g.print(tem);
        u8g.setPrintPos(5, 7 + 8 * 5);
        u8g.print("RUN: ");
        voadd(millis()/86400000);
        u8g.print(millis()/86400000);
        u8g.print("D");
        voadd((millis()%86400000)/3600000);  
        u8g.print((millis()%86400000)/3600000);
        u8g.print("H");
        voadd(((millis()%86400000)%3600000)/60000);
        u8g.print(((millis()%86400000)%3600000)/60000);
        u8g.print("M");

        break;
      }
      /*
      switch(MENU_xuan2)						//标记本层菜单
       {
       case 0:
       MENUnum2 = 50;
       break;
       case 1:
       MENUnum2 = 51;
       break;
       case 2:
       MENUnum2 = 52;
       break;
       case 3:
       MENUnum2 = 53;
       break;
       case 4:
       MENUnum2 = 54;
       break;
       }
       */
      break;      
    }

  }


  //-------------------------------
  if(MENU_xuan3sta)								//进入三级菜单后
  {
    u8g.setPrintPos(5, 7 + 8 * 0);
    if(millis() / 800 % 2 != 0)
      u8g.print("..PLEASE INPUT..");

    switch(MENUnum2)							//读取上级选中菜单
    {

      //    case 21:
      //      u8g.setPrintPos(5, 7+8*1);
      //      u8g.print("SWITCH:");
      //      u8g.print(C_timesta[MENU_xuan1-1]);
      //      break;
    case 22:
      u8g.setPrintPos(5, 7 + 8 * 2);
      u8g.print("HOUR:");
      u8g.print(C_timesetshi[MENU_xuan1-1]);
      break;
    case 23:
      u8g.setPrintPos(5, 7 + 8 * 3);
      u8g.print("MIN:");
      u8g.print(C_timesetfen[MENU_xuan1-1]);
      break;
    case 24:
      u8g.setPrintPos(5, 7 + 8 * 4);
      u8g.print("DELAY:");
      u8g.print(C_timesetdelay[MENU_xuan1-1]);
      break;

    case 31:
      u8g.setPrintPos(5, 7 + 8 * 1);
      u8g.print("TIMING:");
      u8g.print(C_dsset[MENU_xuan1-1]);
      break;
    }

  }

}


void vomenu()
{
  int enter_cache = 0;						//缓存enter状态 防止连续触发

  if(MENU_xuan0sta)							//进入初级菜单后
  {
    if(KEY_MENU_up1x)
      MENU_xuan0++;
    if(KEY_MENU_down1x)
      MENU_xuan0--;

    MENU_xuan0 = pkj.VOLtiaobian(MENU_xuan0, 0, 4);

    if(KEY_MENU_enter)								//选中时
    {
      MENU_xuan0sta = false;					//关闭初级菜单
      MENU_xuan1sta = true;					//一级菜单激活
      MENU_xuan1 = 1;							//指到第一栏

      enter_cache = 1;						//标记enter状态
    }
  }
  //-------------------------------
  if(MENU_xuan1sta)							//进入一级菜单后
  {
    if(KEY_MENU_up1x)
      MENU_xuan1++;
    if(KEY_MENU_down1x)
      MENU_xuan1--;

    if(enter_cache != 1)						//判断上面enter是否触发过
    {
      if(KEY_MENU_enter)								//选中时
      {
        enter_cache = 1;						//标记enter状态

        if(MENU_xuan1 == 0)					//返回上一级
        {
          MENU_xuan1sta = false;				//关闭一级菜单显示
          MENU_xuan0sta = true;				//开启初级菜单显示
        }
        else
        {
          MENU_xuan1sta = false;				//关闭一级菜单
          MENU_xuan2sta = true;				//开启二级菜单
          MENU_xuan2 = 1;						//指到第一栏
        }
      }
    }
  }

  //-------------------------------

  if(MENU_xuan2sta)							//进入二级菜单后
  {
    if(KEY_MENU_up1x)
      MENU_xuan2++;
    if(KEY_MENU_down1x)
      MENU_xuan2--;

    if(enter_cache != 1)						//判断上面enter是否触发过
    {
      if(KEY_MENU_enter)
      {
        enter_cache = 1;						//标记enter状态

        if(MENUnum2 == 21)
        {
          C_timesta[MENU_xuan1-1] = !C_timesta[MENU_xuan1-1];
        }

        else
        {
          if(MENU_xuan2 == 0 || MENUnum0 == 10 || MENUnum0==40 || (MENUnum0==50 && (MENU_xuan1 == 1 || MENU_xuan1 == 2 || MENU_xuan1 == 3 || MENU_xuan1 == 4)))	//跳转上一级，第二个条件：完成选择设置后
          {
            MENU_xuan2sta = false;				//关闭二级菜单
            MENU_xuan1sta = true;				//开启一级菜单
            switch(MENUnum2)					//第二个条件：完成选择设置后
            {
            case 11:
              MOD = false;
              break;
            case 12:
              MOD = true;
              break;
            }
            C_SW=0;
          }
          else
          {
            MENU_xuan2sta = false;				//关闭二级菜单
            MENU_xuan3sta = true;				//开启三级菜单
            MENU_xuan3 = 1;						//指到第一栏
          }
        }
      }
    }
  }

  //-------------------------------

  if(MENU_xuan3sta)							//进入三级菜单后
  {
    switch(MENUnum2)						//判断选中菜单做出响应
    {
      //    case 21:
      //      if(KEY_MENU_up1x)
      //        C_timesta[MENU_xuan1-1]=!C_timesta[MENU_xuan1-1];
      //      if(KEY_MENU_down1x)
      //        C_timesta[MENU_xuan1-1]=!C_timesta[MENU_xuan1-1];
      //      break;
    case 22:
      if(KEY_MENU_up1x)
        C_timesetshi[MENU_xuan1-1]++;
      if(KEY_MENU_down1x)
        C_timesetshi[MENU_xuan1-1]--;
      break;
    case 23:
      if(KEY_MENU_up1x)
        C_timesetfen[MENU_xuan1-1]++;
      if(KEY_MENU_down1x)
        C_timesetfen[MENU_xuan1-1]--;
      break;
    case 24:
      if(KEY_MENU_up1x)
        C_timesetdelay[MENU_xuan1-1]++;
      if(KEY_MENU_up10x)
        C_timesetdelay[MENU_xuan1-1] += 10;
      if(KEY_MENU_down1x)
        C_timesetdelay[MENU_xuan1-1]--;
      if(KEY_MENU_down10x)
        C_timesetdelay[MENU_xuan1-1] -= 10;
      break;

    case 31:
      if(KEY_MENU_up1x)
        C_dsset[MENU_xuan1-1]++;
      if(KEY_MENU_up10x)
        C_dsset[MENU_xuan1-1] += 10;
      if(KEY_MENU_down1x)
        C_dsset[MENU_xuan1-1]--;
      if(KEY_MENU_down10x)
        C_dsset[MENU_xuan1-1] -= 10;
      break;
    }

    if(enter_cache != 1)						//判断上面enter是否触发过
    {
      if(KEY_MENU_enter)
      {
        MENU_xuan3sta = false;
        MENU_xuan2sta = true;
      }
    }
  }

}

void vosend()
{
  /*========nRF24L01=======*/
  network.update();

  unsigned long now = millis();
  if ( now - last_sent >= interval  )
  {
    last_sent = now;
    /*
            Serial.print("Sending...");
     */

    send_a payloada = { 
      millis(), C[0], C[1], C[2], C[3], C_SW                                                                                                                                        };

    RF24NetworkHeader header(/*to node*/ other_node);
    bool oka = network.write(header, &payloada, sizeof(payloada));
    /*
    if (oka)
     {
     }
     */
  }
}  

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


void volcdsetup(char* zi,int x,int y)
{
  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_chikitar);
    u8g.setPrintPos(x, y); 
    u8g.print(zi);
  } 
  while( u8g.nextPage() );
}