=================================
本作品采用知识共享 署名-非商业性使用-相同方式共享 3.0 未本地化版本 许可协议进行许可。
访问 http://creativecommons.org/licenses/by-nc-sa/3.0/ 查看该许可协议。
=================================

硬件：
主机：MCU：MEGA644PA，网络芯片：ENC28J60，RJ45网口：HR911105A，2.4G无线：nRF24L01，显示屏：NOKIA5110屏，红外接收头
副机：MCU：MEGA328P或者168PA，2.4G无线：nRF24L01，可控硅：BT136，光耦：MOC3041
{副机可以自行选择设计成节点式（既一个arduino+24L01控制一个开关节点）、还是单MCU多路式（既一个arduino+24L01控制四个开关）}

除了以上条件，你还需要一个路由器，一个YEELINK账号，和一个红外遥控器。


========================================================
软件：

配置：
比如我的路由器网关是192.168.1，那么主机程序段里设置成如下即可，第一个是主机的IP地址，这个要和本地网页中的IP地址匹配

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

然后YEELINK的ID：
urlBuf0[]是YEELINK控制开关地址，剩下的是四个节点，然后填好你的API就可以

char website[] PROGMEM = "api.yeelink.net";

char urlBuf0[] PROGMEM = "/v1.0/device/xxx/sensor/xxx/";

char urlBuf1[] PROGMEM = "/v1.0/device/xxx/sensor/xxx/";
char urlBuf2[] PROGMEM = "/v1.0/device/xxx/sensor/xxx/";
char urlBuf3[] PROGMEM = "/v1.0/device/xxx/sensor/xxx/";
char urlBuf4[] PROGMEM = "/v1.0/device/xxx/sensor/xxx/";

char apiKey[] PROGMEM = "U-ApiKey: xxx";

================================================

说说红外遥控的使用：
开机进入系统后：
按“1”“2”“3”“4”可以开关四路节点；
按“PLAY”也可以实现系统模式切换（YEELINK万维网控制还是本地手动控制）；
按“CH+”“CH-”“CH”可以选择相应节点并且执行倒计时功能。

按“EQ”即可进入设置，此时“0”为确认，“+”和“-”上下移动菜单，再次按“EQ”结束设置：
设置的第一项“CONFIG MOD”是切换系统模式，这个在主界面按PLAY也可以实现切换。
设置的第二项“CONFIG TIME”是预约开启的设置，选择对应节点后，第一项是否定时开启，第二项与第三项是定时开启的时间设置，第四项是定时开启的世界设置（单位分钟）。
设置的第三项“CONFIG DS”是倒计时的设置，选择对应节点后，即可设置倒计时时间（单位分钟），此时“+”“-”“NEXT”“PREV”分别是加1，减1，加10，减10。
设置的第四项“CONFIG CON”是配置节点用的，选择所需配置节点后，系统会提示按下你所需节点的配置按键2秒以上，此时你能看到节点上的状态灯快速闪烁的话，就配置成功了；对了，工作模式下，节点上的灯闪动次数对应着第几路。
设置的第五项“CONFIG INFO”是系统信息和about

================================================

关于本地局域网控制的web网页：
你可以用编辑器打开index.html，里面的“http://192.168.1.121”就是当前arduino主机对应的IP地址；
另外，由于jquery的安全限制，现在仅支持PC端谷歌浏览器和ios端的谷歌浏览器的正常使用。

