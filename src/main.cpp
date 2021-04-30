#include <Arduino.h>

// NAPT example released to public domain


#define STASSID "ssid_wifi"
#define STAPSK  "pwd_wifi"


#include <WiFi.h>
#include "esp_wifi.h"


#if !IP_NAPT
  #error "IP_NAPT must be defined"
#else
  #include "lwip/lwip_napt.h"
#endif

#include "dhcpserver/dhcpserver.h"
#include "dhcpserver/dhcpserver_options.h"


uint8_t AP_clients = 0;
uint8_t AP_clients_last = AP_clients;




void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nNAPT Range extender\n");
  Serial.printf("Heap on start: %d\n", ESP.getFreeHeap());


  
  //** First, connect to STA so we can get a proper local DNS server
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




  //** Give DNS servers to AP side (Added after softAP setup)
  //**
  //***********************************
  // For ESP8266 example:
  // dhcpSoftAP.dhcps_set_dns(0, WiFi.dnsIP(0));
  // dhcpSoftAP.dhcps_set_dns(1, WiFi.dnsIP(1));

  // For esp32:
  #ifdef __DHCPS_H__
    Serial.println("all should works");           // But it does not so I have commented.
    // Unfortunatelly,it appears the errors while linking as explained here: https://github.com/espressif/arduino-esp32/issues/5117

    // Enable DNS (offer) for dhcp server:

  // dhcps_offer_t dhcps_dns_value = OFFER_DNS;
  // dhcps_set_option_info(6, &dhcps_dns_value, sizeof(dhcps_dns_value));

    // Set custom dns server address for dhcp server
  // ip_addr_t dnsserver;
    // dnsserver.u_addr.ip4.addr = ipaddr_addr(DEFAULT_DNS);;
    // dnsserver.u_addr.ip4.addr = ipaddr_addr("8.8.8.8");
  // dnsserver.type = IPADDR_TYPE_V4;
  // dhcps_dns_setserver(&dnsserver);

  // tcpip_adapter_get_dns_info(TCPIP_ADAPTER_IF_AP, TCPIP_ADAPTER_DNS_MAIN, &dnsinfo);
  // Serial.printf("DNS IP:" IPSTR, IP2STR(&dnsinfo.ip.u_addr.ip4));
  #else
  Serial.println("DHCPS_H not included");
  #endif


  // Other way that does not work:
  // WiFi.softAPConfig(  // enable AP, with android-compatible google domain
  //   IPAddress(172, 217, 28, 254),
  //   IPAddress(172, 217, 28, 254),
  //   IPAddress(255, 255, 255, 0));



  // SOLUTION: From topic https://esp32.com/viewtopic.php?t=3761#p17169
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

  Serial.printf("\nAP: %s\n", WiFi.softAPIP().toString().c_str());




  //** Give DNS servers to AP side:
  //**
  //***********************************
  esp_err_t err;
  tcpip_adapter_dns_info_t ip_dns;


  err = tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
  Serial.printf("\ntcpip_adapter_dhcps_stop: err %s", esp_err_to_name(err)) ;

  err = tcpip_adapter_get_dns_info(TCPIP_ADAPTER_IF_STA, ESP_NETIF_DNS_MAIN, &ip_dns);
  Serial.printf("\ntcpip_adapter_get_dns_info: err %s . ip_dns: " IPSTR, esp_err_to_name(err), IP2STR(&ip_dns.ip.u_addr.ip4)) ;

  // err = tcpip_adapter_set_dns_info(TCPIP_ADAPTER_IF_STA, ESP_NETIF_DNS_FALLBACK, &ip_dns);
  // Serial.printf("\tcpip_adapter_set_dns_info ESP_NETIF_DNS_FALLBACK: err %s . ip_dns:" IPSTR, esp_err_to_name(err), IP2STR(&ip_dns.ip.u_addr.ip4)) ;
 
  err = tcpip_adapter_set_dns_info(TCPIP_ADAPTER_IF_AP, ESP_NETIF_DNS_MAIN, &ip_dns);
  Serial.printf("\tcpip_adapter_set_dns_info ESP_NETIF_DNS_MAIN: err %s . ip_dns:" IPSTR, esp_err_to_name(err), IP2STR(&ip_dns.ip.u_addr.ip4)) ;
  
  // ip_dns.ip.u_addr.ip4.addr = ipaddr_addr("8.8.8.8");
  // ip_dns.ip.type = IPADDR_TYPE_V4;
  //  err = tcpip_adapter_set_dns_info(TCPIP_ADAPTER_IF_STA, ESP_NETIF_DNS_BACKUP, &ip_dns);
  // Serial.printf("\tcpip_adapter_set_dns_info ESP_NETIF_DNS_BACKUP: err %s . ip_dns:" IPSTR, esp_err_to_name(err), IP2STR(&ip_dns.ip.u_addr.ip4)) ;
 

  dhcps_offer_t opt_val = OFFER_DNS; // supply a dns server via dhcps
  tcpip_adapter_dhcps_option(ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER, &opt_val, 1);

  err = tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP);
  Serial.printf("\ntcpip_adapter_dhcps_start: err %s\n", esp_err_to_name(err)) ;




  //**  Enable NAT:
  //**
  //***********************************

  // For esp32:
  #if IP_NAPT
  // u32_t napt_netif_ip = 0xC0A80401; // Set to ip address of softAP netif (Default is 192.168.4.1)
  // ip_napt_enable(htonl(napt_netif_ip), 1);
  // ip_napt_enable_no(WiFi.softAPNetworkID(), 1);
  // ip_napt_enable_no(ESP_IF_WIFI_AP, 1);
  // ip_napt_enable(IPAddress(192, 168, 4, 1), 1);

  ip_napt_enable(WiFi.softAPIP(), 1);

  #endif


  Serial.printf("Heap before: %d\n", ESP.getFreeHeap());
  Serial.println();
}


void loop() {

  AP_clients = WiFi.softAPgetStationNum();

  if (AP_clients_last != AP_clients){
    Serial.printf("Stations connected to AP: %d\n", AP_clients);
    AP_clients_last = AP_clients;

    wifi_sta_list_t wifi_sta_list;
    tcpip_adapter_sta_list_t adapter_sta_list;
  
    memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
    memset(&adapter_sta_list, 0, sizeof(adapter_sta_list));

    delay(500);   // To give time to AP to provide IP to the new station
    esp_wifi_ap_get_sta_list(&wifi_sta_list);
    tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list);
  
    for (int i = 0; i < adapter_sta_list.num; i++) {
      tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];
      Serial.printf("\t - Station %d MAC: ", i);
      for(int i = 0; i< 6; i++){
        Serial.printf("%02X", station.mac[i]);  
        if(i<5)Serial.print(":");
      }
      Serial.printf("  IP: " IPSTR, IP2STR(&station.ip));
      Serial.println();
    }
  }
}