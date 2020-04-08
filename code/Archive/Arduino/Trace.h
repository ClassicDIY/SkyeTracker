#ifndef TRACE_INC
#define TRACE_INC

#include <Arduino.h>

template <typename Generic> inline void traceln(Print* stm, Generic text)
{
	if (stm != 0) {
		stm->println(text);
	}
};

template <typename Generic> inline void tracelnHex(Print* stm, Generic text)
{
	if (stm != 0) {
		stm->println(text, HEX);
	}
};

template <typename Generic> void trace(Print* stm, Generic text)
{
	if (stm != 0) {
		stm->print(text);
	}
};

template <typename Generic> void traceHex(Print* stm, Generic text)
{
	if (stm != 0) {
		stm->print(text, HEX);
	}
};


void inline printHex(Print* stm, char X) {
	if (stm != 0) {
		if (X < 10) { trace(stm, '0'); }
		traceHex(stm, X);
	}
}

void inline printHex(Print* stm, unsigned int* ptr, int numberOfWords)
{
	if (stm != NULL) {
		trace(stm, '[');
		for (int i = 0; i < numberOfWords; i++)
		{
			int w = ptr[i];
			unsigned char l = w & 0x00ff;
			unsigned char h = w >> 8;
			trace(stm, ' ');
			printHex(stm, h);
			printHex(stm, l);
			trace(stm, ' ');
		}
		traceln(stm, ']');
	}
}

void inline printHex(Print* stm, unsigned char* ptr, int numberOfBytes)
{
	if (stm != NULL) {
		trace(stm, '[');
		for (int i = 0; i < numberOfBytes; i++)
		{
			printHex(stm, ptr[i]);
			trace(stm, ' ');
		}
		traceln(stm, ']');
	}
}




#endif
