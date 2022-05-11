#pragma once
#include <map>
#include <vector>

#include "antavore_definitions.h"

namespace antavore
{
class CFrame;

////////////////////////////////////////////////////////////////
class CCircularBuffer
{
public:
	void reset(TAudioCursor cursor);
	int addData(TAudioSample* buffer, int count_samples);
	void dumpData(TAudioSample* buffer, int count_samples);
	void dump();

	TAudioSample operator[](TAudioCursor cursor);
	bool moveFirst(TAudioSample &value);
	void pushBack(TAudioSample value);
	int getNumSamples();
	void forget(int count_samples);
	void forgetUpTo(TAudioCursor cursor);
	void keepLast(int count_samples);
	void keepFirst(int count_samples);

	bool copyToFrame(CFrame &frame);
	void mixFrameAtEnd(CFrame &frame, int offset_from_end);

	TAudioCursor getStartAbs() { return m_nForgottenUpTo; };
	TAudioSample getSampleFromStart(int offset);

	void moveDataFrom(CCircularBuffer &other, int count_samples);

private:

#ifdef JavaScript
	static const TAudioCursor m_nLength = 0x80000;
	static const TAudioCursor m_nAddressMask = 0x7ffff;
#else
	static const TAudioCursor m_nLength = 0x400000;
	static const TAudioCursor m_nAddressMask = 0x3fffff;
#endif

	TAudioCursor m_nForgottenUpTo = 0;
	TAudioCursor m_nWrittenUpTo = 0;

	TAudioSample m_Data[m_nLength];
};

}