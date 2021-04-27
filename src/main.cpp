#include <Arduino.h>

// NAPT example released to public domain


#define STASSID "ssid_wifi"
#define STAPSK  "pwd_wifi"

// #define NAPT 1000
// #define NAPT_PORT 10

// #include <ESP8266WiFi.h>
// #include <lwip/napt.h>
// #include <lwip/dns.h>
// #include <LwipDhcpServer.h>

#include <WiFi.h>
#if !IP_NAPT
  #error "IP_NAPT must be defined"
#else
  #include "lwip/lwip_napt.h"
#endif
#include <lwip/dns.h>
// #include <LwipDhcpServer.h>
// #include "dhcpserver/dhcpserver.h"
#include "dhcpserver/dhcpserver.h"
#include "dhcpserver/dhcpserver_options.h"


#define MY_DNS_IP_ADDR 0x08080808 // 8.8.8.8




void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nNAPT Range extender\n");
  Serial.printf("Heap on start: %d\n", ESP.getFreeHeap());


  
  //** first, connect to STA so we can get a proper local DNS server
  //**
  //***********************************
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.printf("\nSTA: %s (dns: %s / %s)\n",
                WiFi.localIP().toString().c_str(),
                WiFi.dnsIP(0).toString().c_str(),
                WiFi.dnsIP(1).toString().c_str());




  //** give DNS servers to AP side
  //**
  //***********************************
  // For ESP8266 example:
  // dhcpSoftAP.dhcps_set_dns(0, WiFi.dnsIP(0));
  // dhcpSoftAP.dhcps_set_dns(1, WiFi.dnsIP(1));

  // For esp32:
  // // Enable DNS (offer) for dhcp server
  // dhcps_offer_t dhcps_dns_value = OFFER_DNS;
  // dhcps_set_option_info(6, &dhcps_dns_value, sizeof(dhcps_dns_value));

  // ip_addr_t dnsserver;
  // // Set custom dns server address for dhcp server
  // dnsserver.u_addr.ip4.addr = htonl(MY_DNS_IP_ADDR);
  // dnsserver.u_addr.ip4.addr = esp_ip4addr_aton("8.8.8.8");
  // dnsserver.type = IPADDR_TYPE_V4;
  // dhcps_dns_setserver(&dnsserver);

  // tcpip_adapter_get_dns_info(TCPIP_ADAPTER_IF_AP, TCPIP_ADAPTER_DNS_MAIN, &dnsinfo);
  // Serial.printf("DNS IP:" IPSTR, IP2STR(&dnsinfo.ip.u_addr.ip4));



  // Other way that does not work:
  // WiFi.softAPConfig(  // enable AP, with android-compatible google domain
  //   IPAddress(172, 217, 28, 254),
  //   IPAddress(172, 217, 28, 254),
  //   IPAddress(255, 255, 255, 0));

  // WiFi.softAPConfig(  // enable AP, with android-compatible google domain
  //   IPAddress(192, 168, 4, 1),
  //   IPAddress(192, 168, 4, 1),
  //   IPAddress(255, 255, 255, 0),
  //   WiFi.dnsIP(0),
  //   WiFi.dnsIP(1)
  //   );



  // From topic https://esp32.com/viewtopic.php?t=3761#p17169
  // tcpip_adapter_ip_info_t info = {0};
  // tcpip_adapter_dns_info_t dns_info = {0};
  // memset (&dns_info, 8, sizeof(dns_info));
  // IP4_ADDR(&info.ip, 192, 168, 4, 1);
  // IP4_ADDR(&info.gw, 192, 168, 4, 1);
  // IP4_ADDR(&info.netmask, 255, 255, 255, 0);
  // IP_ADDR4(&dns_info.ip, 192, 168, 1, 230);
  
  // tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
  // tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &info);
  // tcpip_adapter_set_dns_info(TCPIP_ADAPTER_IF_AP, ESP_NETIF_DNS_MAIN, &dns_info);
  // dhcps_offer_t opt_val = OFFER_DNS; // supply a dns server via dhcps
  // tcpip_adapter_dhcps_option(TCPIP_ADAPTER_OP_SET, TCPIP_ADAPTER_DOMAIN_NAME_SERVER, &opt_val, 1);
  // tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP);



  //**  Enable wifi AP:
  //**
  //***********************************
  WiFi.softAP(STASSID "extender2", STAPSK);
  Serial.printf("AP: %s\n", WiFi.softAPIP().toString().c_str());

  Serial.printf("Heap before: %d\n", ESP.getFreeHeap());


  //**  Enable NAT:
  //**
  //***********************************
  // For ESP8266 example:
  // err_t ret = ip_napt_init(NAPT, NAPT_PORT);
  // err_t ret = 0;
  // Serial.printf("ip_napt_init(%d,%d): ret=%d (OK=%d)\n", NAPT, NAPT_PORT, (int)ret, (int)ERR_OK);
  
  // if (ret == ERR_OK) {
  //   // ret = ip_napt_enable_no(SOFTAP_IF, 1);
  //   uint32_t my_ap_ip = ipaddr_addr("192.168.4.1");
  //   ip_napt_enable(my_ap_ip, 1);
  //   Serial.printf("ip_napt_enable_no(SOFTAP_IF): ret=%d (OK=%d)\n", (int)ret, (int)ERR_OK);
  //   if (ret == ERR_OK) {
  //     Serial.printf("WiFi Network '%s' with same password is now NATed behind '%s'\n", STASSID "extender", STASSID);
  //   }
  // }
  // Serial.printf("Heap after napt init: %d\n", ESP.getFreeHeap());
  // if (ret != ERR_OK) {
  //   Serial.printf("NAPT initialization failed\n");
  // }

  // For esp32:
  #if IP_NAPT
  // u32_t napt_netif_ip = 0xC0A80401; // Set to ip address of softAP netif (Default is 192.168.4.1)
  // ip_napt_enable(htonl(napt_netif_ip), 1);
  // ip_napt_enable_no(WiFi.softAPNetworkID(), 1);
  ip_napt_enable_no(ESP_IF_WIFI_AP, 1);
  // ip_napt_enable(IPAddress(192, 168, 4, 1), 1);
  ip_napt_enable(WiFi.softAPIP(), 1);

  #endif


}


void loop() {
}