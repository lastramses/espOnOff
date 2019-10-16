
#include <ESP8266WiFi.h>
#include "espOnOffGlobals.h"
#include "serviceFcn.h"

int strToInt(String inStr){
  return inStr.toInt();
}

void stdOut(String logStr){
  if (stdOutTgt==1){ //serial
    Serial.println(logStr);
  }else if(stdOutTgt==2){ //logString
    logTelnetBuff.write(logStr + "\r\n");
  }else if(stdOutTgt==3){ //serial & logString
    Serial.println(logStr);
    logTelnetBuff.write(logStr + "\r\n");
  }
}
