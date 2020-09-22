#include <includes.h>
#include <WiFiClient.h>
//#include <ESP8266mDNS.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>
#include <Ticker.h>
#include <time.h>

#include <espOnOffGlobals.h>
#include <HttpServerHandles.h>
#include <LocalTime.h>

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

WiFiUDP udpServer;

WiFiServer telnetServer(23);
WiFiClient telnetClient;

Ticker tick60s;
EspRelay espOnOff;
LogCircBuffer<512> logTelnetBuff;
unsigned long tiISRButtonDis = 0; //time stamp a wich the isr was disabled
uint8_t stISRButtonEna = besTRUE; //flag used to debounce the input
uint16_t tiISRDebMsec = 200; //debounce (disable isr on pin) time
uint8_t stISRButtonReq = besFALSE; //flag to process isr in main loop
File fsUploadFile; //used by fileuploadstream

String confEspHost = "espOnOff";
String confSSID;
String confPW;
byte confMACAddr[6];
uint8_t confOffOnUnknownCmd = 1;
//TODO: add config for behavior of corrupted udp message
//    make host name configurable
//    conf page to have ssid and pw visible

void ICACHE_RAM_ATTR isrButton(){
  detachInterrupt(digitalPinToInterrupt(pinButton));
  stISRButtonEna = besFALSE;
  tiISRButtonDis = millis();
  stISRButtonReq = besTRUE;
  stdOut("isrD3 - button - received");
}

void isrTick60sFunc(){
  if(WiFi.getMode() !=WIFI_STA){ //in case of electricity outage, esp will come up faster than wifi router and reset to AP mode. To avoid manual power reset device will check if the confSSID is in range and reset.
    //scan for networks, if confSSID in range, restart
    int nWifi = WiFi.scanNetworks();
    if (nWifi == 0)
      stdOut("no wifi networks found");
    else{
      for (int i = 0; i < nWifi; ++i){
        if(WiFi.SSID(i) == confSSID){ //is confSSID in range?
          stdOut(confSSID + " found, reseting esp");
          ESP.restart();
        }
      }
    }
  }
  printLocalTime();
}

void isrDebounce(){
  if ( (stISRButtonEna==besFALSE) && (abs(millis()-tiISRButtonDis)>tiISRDebMsec) ){
    attachInterrupt(digitalPinToInterrupt(pinButton), isrButton, FALLING);
    stISRButtonEna = besTRUE;
  }
}

void configPins(){
  pinMode(pinRELAY, OUTPUT);
  pinMode(pinLED, OUTPUT);
  pinMode(pinButton, INPUT_PULLUP);
  espOnOff.setRelaySt(besOFF,besRelReqNULL);
  attachInterrupt(digitalPinToInterrupt(pinButton), isrButton, FALLING);
}

void configSPIFFS(){
  if (SPIFFS.begin()) {
    stdOut(" SPIFFS File system loaded");
    File configFile = SPIFFS.open("/config.dat", "r");
    if (configFile){
      String tmpstr = configFile.readStringUntil(3); //read empty 3
      for (int i=0;i<6;++i){
        char tmpByte[2];
        tmpstr.substring(i*3,i*3+2).toCharArray(tmpByte, 3); //slice the string
        confMACAddr[i] = strtol(tmpByte, NULL, 16); //convert slice to integer
      }
      confSSID = configFile.readStringUntil(3);
      confPW = configFile.readStringUntil(3);
      confEspHost = configFile.readStringUntil(3);
      confOffOnUnknownCmd = configFile.readStringUntil(3).toInt();
      configFile.close();
      stdOut("confMACAddr = " + String(confMACAddr[0]) + String(confMACAddr[2]) + String(confMACAddr[4]));
      stdOut("confSSID = " + confSSID + "\r\n");
      stdOut("confPW = " + confPW + "\r\n");
      stdOut("confEspHost = " + confEspHost + "\r\n");
      stdOut("confOffOnUnknownCmd = " + String(confOffOnUnknownCmd) + "\r\n");
    }
    stdOut("SPIFFS size: " + String(ESP.getFlashChipSize()));
  }else{
    stdOut(" Error loading SPIFFS File system");
  }
}

void configWIFI(){
  wifi_set_macaddr(STATION_IF, confMACAddr);
  stdOut("MAC: " + WiFi.macAddress());
  stdOut("WiFi connecting to " + String(confSSID));
  WiFi.hostname(confEspHost);
  WiFi.mode(WIFI_STA);
  WiFi.begin(confSSID, confPW);
  uint8_t NrWiFiConnFail = 0;
  while ((WiFi.status() != WL_CONNECTED) && NrWiFiConnFail<10*2){ //try to connect for 10s if fails switch to ap mode
    stdOut(".");
    NrWiFiConnFail++;
    digitalWrite(pinLED,!digitalRead(pinLED));
    delay(500);
  }
  if (NrWiFiConnFail>=20){
    if (SPIFFS.exists("/wifiConnFail")){ // if exist start esp in AP mode
      WiFi.mode(WIFI_AP);
      WiFi.softAPConfig(IPAddress(192,168,0,1), IPAddress(192,168,0,1), IPAddress(255,255,255,0)) ;
      WiFi.softAP(confEspHost, "12345678"); // Start the access point
      stdOut("Access Point \"" + confEspHost + "\" started");
      stdOut("IP address:\t" + WiFi.softAPIP().toString());
    }else{
      File wifiConnFail = SPIFFS.open("/wifiConnFail", "w");
      wifiConnFail.print("Connection to " + confSSID + " failed, attempting reset");
      wifiConnFail.close();
      ESP.reset();
    }
  }
  SPIFFS.remove("/wifiConnFail");//remove file as user should have configured the ssid
  stdOut("\r\nWiFi connected");
  espOnOff.setRelaySt(besOFF,besRelReqNULL);
}

void configHttpServer(){
  /*if (MDNS.begin(confEspHost)) {              // Start the mDNS responder for esp8266.local
    stdOut("mDNS responder started");
    MDNS.addService("http", "tcp", 80);
  } else {
    stdOut("Error setting up MDNS responder!");
  }*/
  httpServer.on("/RelayPwrReq", HTTP_POST,httpServerHandlePwrReq);
  httpServer.on("/getData", HTTP_POST,httpServerHandleGetData);
  httpServer.on("/fileuploadstream", HTTP_POST, [](){
    httpServer.send(200);
    }, httpServerHandleFileUploadStream);
  httpServer.on("/saveSSID", HTTP_POST,httpServerHandleSaveSSID);
  httpServer.on("/DeviceReset", HTTP_GET,httpServerHandleDeviceReset);
  httpServer.onNotFound(httpServerHandleNotFound);

  httpUpdater.setup(&httpServer);
  httpServer.begin();
  stdOut("HTTP server started");
  stdOut("HTTPUpdateServer ready! Open http://" + confEspHost + ".local/update in browser");

  udpServer.begin(81);
  stdOut("udp server started on port 81");

  telnetServer.begin();
  telnetServer.setNoDelay(true);
  stdOut("telnet server started on port 23");
}

void ICACHE_RAM_ATTR telnetProcess(){
  if (telnetServer.hasClient()) {
    if (!telnetClient) { // equivalent to !serverClients[i].connected()
      telnetClient = telnetServer.available();
      stdOut("New telnet client");
    }
  }

  while (telnetClient.available()){
    int charRead = telnetClient.read();
    if (charRead == 100){ //=d
      charRead = telnetClient.read();
      if (charRead == 32){ //=" "
        String tmp = telnetClient.readStringUntil(13);
        stdOut("del req=\""+tmp+"\"");
        SPIFFS.remove("/" + tmp);
      }
    }
    logTelnetBuff.write(String(charRead));
  }

  if (telnetClient.availableForWrite()){
    uint16_t lenLogBuf = logTelnetBuff.getBuffDataSize();
	   for (uint16_t i=0;i<lenLogBuf;++i){
       telnetClient.write(logTelnetBuff.read());
	    }
  }
}

void setup() {
  configPins();
  Serial.begin(115200);
  delay(50);
  stdOut("\r\nSerial started");
  configSPIFFS();
  configWIFI();
  configHttpServer();

  configTime(5*3600, 1*3600, "pool.ntp.org", "time.nist.gov");

  tick60s.attach(3600,isrTick60sFunc);
}

void loop(){
  httpServer.handleClient();
  //MDNS.update();
  if (stISRButtonReq == besTRUE){
    if (espOnOff.getRelaySt() == besOFF){
      espOnOff.setRelaySt(besON,besRelReqESPButton);
    }else{
      espOnOff.setRelaySt(besOFF,besRelReqESPButton);
    }
    stISRButtonReq = besFALSE;
  }
  
  if (udpServer.parsePacket()>0){
    udpProcess();
  }

  telnetProcess();
  isrDebounce();
  yield();
}
