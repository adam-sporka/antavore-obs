#include "antavore_definitions.h"
#include "3rd-party/nayuki-dct/FastDctLee.hpp"
#include <math.h>

namespace antavore
{

////////////////////////////////////////////////////////////////
CFrame::CFrame()
{
	reset();
}

////////////////////////////////////////////////////////////////
CFrame::CFrame(const CFrame &frame)
{
	memcpy(m_Data, frame.m_Data, sizeof(TAudioSample) * m_nLength);
}

////////////////////////////////////////////////////////////////
CFrame& CFrame::operator=(const CFrame &src)
{
	memcpy(m_Data, src.m_Data, sizeof(TAudioSample) * m_nLength);
	return *this;
}

////////////////////////////////////////////////////////////////
double& CFrame::operator[] (int offset)
{
	return m_Data[offset];
}

////////////////////////////////////////////////////////////////
TAudioSample* CFrame::getData()
{
	return m_Data;
}

////////////////////////////////////////////////////////////////
void CFrame::linearMixWith(CFrame &other)
{
	for (int a = 0; a < m_nLength; a++)
	{
		m_Data[a] += other.m_Data[a];
	}
}

////////////////////////////////////////////////////////////////
void CFrame::reset()
{
	memset(m_Data, 0, sizeof(TAudioSample) * m_nLength);
}

////////////////////////////////////////////////////////////////
void CFrame::moveDataFromBuffer(CCircularBuffer &buffer)
{
	auto *wrt = m_Data;
	for (int a = 0; a < m_nLength; a++)
	{
		*wrt = buffer.getSampleFromStart(a);
		wrt++;
	}
	buffer.forget(m_nLength);
}

////////////////////////////////////////////////////////////////
void CFrame::copyDataFromBuffer(CCircularBuffer &buffer, int start)
{
	auto *wrt = m_Data;
	for (int a = 0; a < getLength(); a++)
	{
		*wrt = buffer.getSampleFromStart(a + start);
		wrt++;
	}
}

////////////////////////////////////////////////////////////////
void CFrame::setByEigenvector(TSpectrum &eigenvector)
{
	// Clear all
	memset(m_Data, 0, sizeof(TAudioSample) * m_nLength);

	// Copy data
	auto *src = eigenvector;
	auto *dst = m_Data + g_nMscLo;
	for (int a = g_nMscLo; a <= g_nMscHi; a++)
	{
		*dst = *src;
		dst++;
		src++;
	}
}

////////////////////////////////////////////////////////////////
void CFrame::applyWatermark(TSpectrum &eigenvector, double sign, double beta)
{
	CFrame w;
	w.setByEigenvector(eigenvector);
	auto g_ni = sqrt(innerProduct(*this));
	w.scaleAmplitude(beta * sign * g_ni);

	linearMixWith(w);
}

////////////////////////////////////////////////////////////////
TAudioSample CFrame::innerProduct(CFrame &other)
{
	TAudioSample sum = 0.0;
	for (int a = g_nMscLo; a <= g_nMscHi; a++)
	{
		auto p = m_Data[a] * other.m_Data[a];
		sum += p;
	}
	return sum;
}

////////////////////////////////////////////////////////////////
void CFrame::scaleAmplitude(TAudioSample factor)
{
	for (int a = 0; a < m_nLength; a++)
	{
		m_Data[a] *= factor;
	}
}

////////////////////////////////////////////////////////////////
void CFrame::dctFwd()
{
	FastDctLee::transform(m_Data, m_nLength);
}

////////////////////////////////////////////////////////////////
void CFrame::dctInv()
{
	FastDctLee::inverseTransform(m_Data, m_nLength);
	scaleAmplitude(2.0 / (double)m_nLength);
}

////////////////////////////////////////////////////////////////
void CFrame::window()
{
	for (int i = 0; i < getLength(); i++)
	{
		double multiplier = 0.5 * (1 - cos(2 * 3.14159265358979323846 * i / getLength()));
		m_Data[i] = m_Data[i] * multiplier;
	}
}

////////////////////////////////////////////////////////////////
void CFrame::setAt(int offset, TAudioSample sample)
{
	m_Data[offset] = sample;
}

////////////////////////////////////////////////////////////////
int CFrame::getLength()
{
	return g_nMscFrameLen;
}

};