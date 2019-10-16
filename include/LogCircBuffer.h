#ifndef LOGCIRCBUFFER_H
#define LOGCIRCBUFFER_H

template <size_t bufferSize>
class LogCircBuffer {
	uint16_t buffSize = bufferSize;
	char chrBuff[bufferSize];
	uint16_t idxHead = 0;
	uint16_t idxTail = 0;
	uint16_t buffDataUsed = 0;
public:
	int write(String dataIn);
	char read();
	uint16_t getBuffDataSize();
};

#endif
