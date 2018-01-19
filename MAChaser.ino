
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266Ping.h>
#include<lwip/etharp.h>
#include <ESP8266HTTPClient.h>

#include "urlencode.h"

ESP8266WiFiMulti WiFiMulti;

unsigned int sinceTimer(const unsigned int millis_value)
{
  return millis()-millis_value;
}

void resetTimer(unsigned int &millis_value)
{
  millis_value = millis();
}

unsigned int timer;

void setup() {
Serial.begin(115200);
WiFi.mode(WIFI_STA);
WiFiMulti.addAP("OBFUCKSCATED1", "OBFUCKSCATED1");
WiFiMulti.addAP("OBFUCKSCATED2", "OBFUCKSCATED2");
Serial.println();
Serial.println();
Serial.print("Waiting for WiFi");

while(WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
}

Serial.println("");
Serial.print("WiFi connected: ");
Serial.println(WiFi.SSID());
Serial.print("IP address: ");
Serial.println(WiFi.localIP());
    
etharp_init();
timer = 0;
}


void loop() {

  
  
  yield();
  if (sinceTimer(timer)/1000 >= 120 || !timer){
  IPAddress subnet = WiFi.subnetMask();
  IPAddress myip = WiFi.localIP();
  IPAddress gwip = WiFi.gatewayIP();

  uint32_t subnetSize = (256-subnet[0])*(256-subnet[1])*(256-subnet[2])*(256-subnet[3])-2;

  IPAddress startIP = WiFi.localIP();
  startIP[0] &= subnet[0];
  startIP[1] &= subnet[1];
  startIP[2] &= subnet[2];
  startIP[3] &= subnet[3];
  startIP[3] += 1;
  
  String payload;
  payload +="{ \"devices\": [";
  
  uint16_t count=0;
  for (int i=0; i<subnetSize; i++){
    
    IPAddress IPtoCheck = startIP;
    IPtoCheck[0] += (i/255/255/255)%256;
    IPtoCheck[1] += (i/255/255)%256;
    IPtoCheck[2] += (i/255)%256;
    IPtoCheck[3] += i%256;
    if (IPtoCheck == myip || IPtoCheck == gwip) continue;

    ip4_addr_t toCheck;
    IP4_ADDR(&toCheck,IPtoCheck[0],IPtoCheck[1],IPtoCheck[2],IPtoCheck[3]);
    
    etharp_request(netif_default, &toCheck);
//    Serial.printf("Scanning for: %s\n", IPtoCheck.toString().c_str());
    delay(20);
  }
  delay(2000);
  for (int i=0; i<subnetSize; i++){

    IPAddress IPtoCheck = startIP;
    IPtoCheck[0] += (i/255/255/255)%256;
    IPtoCheck[1] += (i/255/255)%256;
    IPtoCheck[2] += (i/255)%256;
    IPtoCheck[3] += i%256;
    if (IPtoCheck == myip || IPtoCheck == gwip) continue;
        
    ip4_addr_t toCheck;
    IP4_ADDR(&toCheck,IPtoCheck[0],IPtoCheck[1],IPtoCheck[2],IPtoCheck[3]);
  
    struct eth_addr *ret_eth_addr = NULL;
    struct ip4_addr const *ret_ip_addr = NULL;
    
    int arp_find = etharp_find_addr(netif_default, &toCheck, &ret_eth_addr, &ret_ip_addr);

//    Serial.printf("Lookup: %s | %d\n", IPtoCheck.toString().c_str(), arp_find );
    
    if(arp_find != -1 && ret_eth_addr != NULL && ( Ping.ping(IPtoCheck, 1) || Ping.ping(IPtoCheck, 1) )) {
        count++;
        char mac[32];
        sprintf(mac, "{ \"mac\": \"%02x:%02x:%02x:%02x:%02x:%02x\"", ret_eth_addr->addr[0], ret_eth_addr->addr[1], ret_eth_addr->addr[2], ret_eth_addr->addr[3], ret_eth_addr->addr[4], ret_eth_addr->addr[5]);
        payload += mac;
        payload += ", \"ip\": \"" + IPtoCheck.toString() + "\"}, ";
    }
    delay(20);
  }
  
//  Serial.printf("Found [%02d] devices\n\n", count);
  if (payload[payload.length()-2] == ',') payload.remove(payload.length()-2);
  payload += "] }";
  Serial.println(payload);
  HTTPClient http;
  http.setUserAgent("SPFEIT presence tracker by github.com/Crypter");
  http.addHeader("Content-Type", "application/json");
  String URL;
  URL += "http://ob.fuck.scated/index.php";
  http.begin(URL);
  http.POST(payload);

  
  resetTimer(timer);
  }
}
