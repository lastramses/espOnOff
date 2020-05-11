#ifndef ESPONOFFGLOBALSDEFINE_H
#define ESPONOFFGLOBALSDEFINE_H

#include <includes.h>

#define pinButton 0 //D3
#define pinD4 2 //D4
#define pinRELAY 12 // D6
#define pinLED 13 // D7

#define besTRUE 0xAA
#define besFALSE 0x55
#define besON 0xAA //used by relay st
#define besOFF 0x55
#define besRelReqNULL 0x0 //used by relaySrc
#define besRelReqHTTP 0x1 //phone or espswt
#define besRelReqUDP 0x2 //espswt
#define besRelReqESPButton 0x2 //onboard button

#define stdOutTgt 3//0=none,1=serial,2=logString,3=serial&LogString
extern EspRelay espOnOff;
extern ESP8266WebServer httpServer;
extern WiFiUDP udpServer;
extern LogCircBuffer<512> logTelnetBuff;
extern File fsUploadFile;
//extern String espHost;
extern String confEspHost;
extern byte confMACAddr[6];
extern String confSSID;
extern String confPW;
extern uint8_t confOffOnUnknownCmd;

#endif
