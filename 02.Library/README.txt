pkj是自己写的简单库
功能是限定/跳转值的上下限

ethercard库的推送方式我自己改过

大概如下：
char website[] PROGMEM = "api.yeelink.net";
char urlBuf0[] PROGMEM = "/v1.0/device/xxx/sensor/xxx/";
char apiKey[] PROGMEM = "U-ApiKey: xxx";

static void my_result_cb (byte status, word off, word len)
{
  sscanf((char *)strstr((const char *)Ethernet::buffer + off, "\"value\""), "\"value\":%d", &yee[yeenum-1]);    //采集关键字数据
}


          ether.browseUrl(urlBuf, "datapoints", website, apiKey, my_result_cb);   //采集节点

          ether.httpPost (urlBuf, "datapoints", website, apiKey, "{\"value\":0}", my_result_cb);    //把节点推到关
