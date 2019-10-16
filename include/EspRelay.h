#ifndef ESPRELAY_H
#define ESPRELAY_H

class EspRelay {
  uint8_t StRlyMdl = 0; //0-Unknown, 1-On, 2-Off
  //uint16_t NrTrunOffDebounce = 0;
  //uint16_t TiExecRate = 50; //ms
  //uint16_t TiTurnOffDebThd1 = 750; //ms
  //uint16_t TiTurnOffDebThd2 = 2000; //ms
  uint8_t lastOnReqSrc = 0;
  uint8_t lastOffReqSrc = 0;
  uint8_t error = 0;
public:
  bool setRelaySt(uint8_t cmd, uint8_t cmdSrc);
  uint8_t getRelaySt();
  //bool patWatDog(uint8_t src);
  //uint8_t isWatDogExp();
  //uint16_t incrTurnOffDebCtr();
  uint8_t getLastOnReqSrc();
  uint8_t verifyGPIOConsistency();
};
#endif
