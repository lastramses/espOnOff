#include <ESP8266WiFi.h>
#include <LogCircBuffer.h>

template <size_t bufferSize>
int LogCircBuffer<bufferSize>::write(String dataIn){
	uint16_t idxDataInStart = max((int16_t)(dataIn.length()-buffSize),(int16_t)0);
	for (uint16_t i=idxDataInStart;i<dataIn.length();++i){
		chrBuff[(idxHead + i)%buffSize]=dataIn[i];
	}
	buffDataUsed = min((uint16_t)(buffDataUsed+dataIn.length()),buffSize);
	idxHead = (idxHead + dataIn.length())%buffSize;
	if (buffSize==buffDataUsed){
		idxTail = idxHead;
	}else{
		//dont update idxTail
	}
	return 0;
}

template <size_t bufferSize>
char LogCircBuffer<bufferSize>::read(){
	if (buffDataUsed>-1){
		uint16_t idxTailOld = idxTail;
		idxTail = (idxTail + 1)%buffSize;
		buffDataUsed = (buffDataUsed - 1);
		return chrBuff[idxTailOld];
	}else{
		return -1;
	}
}

template <size_t bufferSize>
uint16_t LogCircBuffer<bufferSize>::getBuffDataSize(){
	return buffDataUsed;
}

template class LogCircBuffer<512>;
