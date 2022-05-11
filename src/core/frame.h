#pragma once

#include "antavore_definitions.h"

namespace antavore
{

////////////////////////////////////////////////////////////////
using TSpectrum = TAudioSample[g_nMscBandCount];

////////////////////////////////////////////////////////////////
class CFrame
{
private:
	static const int m_nLength = g_nMscFrameLen;
	TAudioSample m_Data[m_nLength];
	
public:
	CFrame();
	CFrame(const CFrame &frame);
	CFrame& operator=(const CFrame &src);
	double& operator[] (int offset);

	TAudioSample* getData();

	void linearMixWith(CFrame &other);
	void reset();

	void moveDataFromBuffer(CCircularBuffer &buffer);
	void copyDataFromBuffer(CCircularBuffer &buffer, int start);
	void setByEigenvector(TSpectrum &eigenvector);
	TAudioSample innerProduct(CFrame &other);
	void scaleAmplitude(TAudioSample factor);
	void dctFwd();
	void dctInv();
	void window();
	void setAt(int offset, TAudioSample sample);
	static int getLength();

	void applyWatermark(TSpectrum &eigenvector, double sign, double beta);
};

};