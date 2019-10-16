//#include <ESP8266WiFi.h>
#include <espOnOffGlobals.h>
#include <EspRelay.h>

bool EspRelay::setRelaySt(uint8_t cmd, uint8_t cmdSrc){
  if (cmd==besON){
    digitalWrite(pinRELAY,HIGH); //Relay
    digitalWrite(pinLED,LOW); //Led
    lastOnReqSrc = cmdSrc;
  }else{
    digitalWrite(pinRELAY,LOW);
    digitalWrite(pinLED,HIGH);
    lastOffReqSrc = cmdSrc;
  }
  return 0;
}

uint8_t EspRelay::getRelaySt(){
  if (digitalRead(pinRELAY)==HIGH){
    return besON; //relay on
  }else{
    return besOFF; //relay off
  }
}
/*
bool EspRelay::patWatDog(uint8_t src){
  NrTrunOffDebounce = 0;
  lastOnReqSrc = src; //this function is used by periodic sources
  return 0;
}

uint8_t EspRelay::isWatDogExp(){
  if ((NrTrunOffDebounce*TiExecRate)>TiTurnOffDebThd2){
    return 2;
  }else if ((NrTrunOffDebounce*TiExecRate)>TiTurnOffDebThd1){
    return 1;
  }else{
    return 0;
  }
}

uint16_t EspRelay::incrTurnOffDebCtr(){
  NrTrunOffDebounce++;
  return NrTrunOffDebounce;
}
*/
uint8_t EspRelay::getLastOnReqSrc(){
  return lastOnReqSrc;
}
