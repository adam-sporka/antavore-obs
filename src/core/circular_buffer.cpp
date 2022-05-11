#include <stdio.h>
#include <string.h>
#include <cassert>

#include "antavore_definitions.h"

namespace antavore
{

////////////////////////////////////////////////////////////////
void CCircularBuffer::reset(TAudioCursor cursor)
{
	m_nForgottenUpTo = cursor;
	m_nWrittenUpTo = cursor;
	memset(m_Data, 0, sizeof(TAudioSample) * m_nLength);
}

////////////////////////////////////////////////////////////////
void CCircularBuffer::dump()
{
	printf("FORGOTTEN UP TO %d, WRITTEN UP TO %d\n", (int)m_nForgottenUpTo, (int)m_nWrittenUpTo);
	for (int a = 0; a < m_nLength; a++)
	{
		printf("%1.1f ", m_Data[a]);
	}
	printf("\n");
}

////////////////////////////////////////////////////////////////
void CCircularBuffer::dumpData(TAudioSample* buffer, int count_samples)
{
	auto *wrt = buffer;
	for (int a = 0; a < count_samples; a++)
	{
		TAudioSample sample;
		moveFirst(*wrt);
		wrt++;
	}
}

////////////////////////////////////////////////////////////////
int CCircularBuffer::addData(TAudioSample* buffer, int count_samples)
{
	if (count_samples <= 0)
	{
		return 0;
	}

	int add_how_much = count_samples;

	if ((m_nWrittenUpTo + count_samples) > (m_nForgottenUpTo + m_nLength))
	{
		add_how_much = m_nForgottenUpTo + m_nLength - m_nWrittenUpTo;
		if (add_how_much < 0) return 0;
	}

	int i = m_nWrittenUpTo;
	for (int a = 0; a < add_how_much; a++)
	{
		int address = i & m_nAddressMask;
		m_Data[address] = *buffer;
		i++;
		buffer++;
	}

	m_nWrittenUpTo += add_how_much;
	return add_how_much;
}

////////////////////////////////////////////////////////////////
int CCircularBuffer::getNumSamples()
{
	return m_nWrittenUpTo - m_nForgottenUpTo;
}

////////////////////////////////////////////////////////////////
void CCircularBuffer::forget(int count_samples)
{
	forgetUpTo(m_nForgottenUpTo + count_samples);
}

////////////////////////////////////////////////////////////////
void CCircularBuffer::forgetUpTo(TAudioCursor cursor)
{
	assert(cursor >= m_nForgottenUpTo && cursor <= m_nWrittenUpTo);
	m_nForgottenUpTo = cursor;
}

////////////////////////////////////////////////////////////////
void CCircularBuffer::keepLast(int count_samples)
{
	auto new_forgotten_up_to = m_nWrittenUpTo - count_samples;
	forgetUpTo(new_forgotten_up_to);
}

////////////////////////////////////////////////////////////////
void CCircularBuffer::keepFirst(int count_samples)
{
	auto new_written_up_to = m_nForgottenUpTo + count_samples;
	m_nWrittenUpTo = new_written_up_to;
}

////////////////////////////////////////////////////////////////
bool CCircularBuffer::copyToFrame(CFrame &frame)
{
	if (getNumSamples() < frame.getLength())
	{
		return false;
	}

	auto *wrt = frame.getData();
	for (int a = 0; a < frame.getLength(); a++)
	{
		*wrt = getSampleFromStart(a);
		wrt++;
	}

	return true;
}

////////////////////////////////////////////////////////////////
void CCircularBuffer::mixFrameAtEnd(CFrame &frame, int offset_from_end)
{
	auto *rd = frame.getData();
	for (int a = m_nWrittenUpTo - offset_from_end; a < m_nWrittenUpTo + frame.getLength(); a++)
	{
		if (a >= m_nForgottenUpTo)
		{
			if (a >= m_nWrittenUpTo) m_Data[a & m_nAddressMask] = *rd;
			else m_Data[a & m_nAddressMask] += *rd;
		}
		rd++;
	}

	m_nWrittenUpTo = m_nWrittenUpTo - offset_from_end + frame.getLength();
}

////////////////////////////////////////////////////////////////
TAudioSample CCircularBuffer::getSampleFromStart(int offset)
{
	return operator[](offset + m_nForgottenUpTo);
}

////////////////////////////////////////////////////////////////
TAudioSample CCircularBuffer::operator[](TAudioCursor cursor)
{
	assert(cursor >= m_nForgottenUpTo && cursor <= m_nWrittenUpTo);
	int address = cursor & m_nAddressMask;
	return m_Data[address];
}

////////////////////////////////////////////////////////////////
bool CCircularBuffer::moveFirst(TAudioSample &value)
{
	if (m_nForgottenUpTo < m_nWrittenUpTo)
	{
		value = operator[](m_nForgottenUpTo);
		forgetUpTo(m_nForgottenUpTo + 1);
		return true;
	}

	value = 0.0;
	return false;
}

////////////////////////////////////////////////////////////////
void CCircularBuffer::pushBack(TAudioSample value)
{
	int address = m_nWrittenUpTo & m_nAddressMask;
	m_Data[address] = value;
	m_nWrittenUpTo++;
}

////////////////////////////////////////////////////////////////
void CCircularBuffer::moveDataFrom(CCircularBuffer &other, int count_samples)
{
	for (int a = 0; a < count_samples; a++)
	{
		TAudioSample sample;
		bool outcome = other.moveFirst(sample);
		assert(outcome);
		pushBack(sample);
	}
}

}