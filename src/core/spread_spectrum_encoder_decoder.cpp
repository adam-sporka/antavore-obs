#include "antavore_definitions.h"
#include <assert.h>
#include <math.h>

namespace antavore {

#include "watermarks.inl"

////////////////////////////////////////////////////////////////
CSpreadSpectrumEncoderDecoder::CSpreadSpectrumEncoderDecoder()
{
	m_pInput = new CCircularBuffer();
	m_pTempBuffer = new CCircularBuffer();
	m_pOutput = new CCircularBuffer();
}

////////////////////////////////////////////////////////////////
CSpreadSpectrumEncoderDecoder::~CSpreadSpectrumEncoderDecoder()
{
	delete m_pInput;
	delete m_pTempBuffer;
	delete m_pOutput;
}

////////////////////////////////////////////////////////////////
void CSpreadSpectrumEncoderDecoder::initializeRandomSequence(const char *sequence)
{
	m_PlusMinusSequence.clear();
	for (const char *c = sequence; *c != 0; c++)
	{
		if (*c == '+') m_PlusMinusSequence.push_back(1.0);
		if (*c == '-') m_PlusMinusSequence.push_back(-1.0);
	}
}

////////////////////////////////////////////////////////////////
void CSpreadSpectrumEncoderDecoder::initializeWatermarkSequence(int repetition, std::vector<int> order)
{
	m_SequenceRepetition = repetition;
	m_SequenceLength = order.size();
	for (int a = 0; a < repetition; a++)
	{
		for (auto n : order)
		{
			m_WatermarkSequence.push_back(n);
		};
	}
}

////////////////////////////////////////////////////////////////
void CSpreadSpectrumEncoderDecoder::initializeSpectrum()
{
	for (int i = 0; i < g_nMscBandCount; i++)
	{
		for (int j = 0; j < g_nMscBandCount; j++)
		{
			m_ppWatermarkSpectrum[i][j] = ((double)(rand() % 1025) / 512.0) - 1.0;
		}
	}
}

////////////////////////////////////////////////////////////////
void CSpreadSpectrumEncoderDecoder::saveSpectrum(const char *filename)
{
#ifndef JavaScript
	FILE *file;
	if (!fopen_s(&file, filename, "wb"))
	{
		fwrite(m_ppWatermarkSpectrum, sizeof(TAudioSample), g_nMscBandCount * g_nMscBandCount, file);
		fclose(file);
	}
#endif
}

////////////////////////////////////////////////////////////////
void CSpreadSpectrumEncoderDecoder::loadSpectrum(const char *filename)
{
#ifndef JavaScript
	FILE *file;
	if (!fopen_s(&file, filename, "rb"))
	{
		fread(m_ppWatermarkSpectrum, sizeof(TAudioSample), g_nMscBandCount * g_nMscBandCount, file);
		fclose(file);
	}
#endif
}

////////////////////////////////////////////////////////////////
void CSpreadSpectrumEncoderDecoder::fetchSpectrumFromArray()
{
	memcpy(m_ppWatermarkSpectrum, watermarks, 32768);
}

////////////////////////////////////////////////////////////////
bool CSpreadSpectrumEncoderDecoder::encode(CFrame &output, bool apply_watermark)
{
	if (m_pInput->getNumSamples() < CFrame::getLength())
	{
		return false;
	}

	output.moveDataFromBuffer(*m_pInput);
	output.dctFwd();

	auto &eigenvector = m_ppWatermarkSpectrum[m_WatermarkSequence[m_RunningIndex % m_WatermarkSequence.size()]];
	auto sign = m_PlusMinusSequence[m_RunningIndex % m_WatermarkSequence.size()];
	m_RunningIndex++;

	if (apply_watermark)
	{
		output.applyWatermark(eigenvector, sign, m_Strength);
	}

	output.dctInv();
	return true;
}

////////////////////////////////////////////////////////////////
int CSpreadSpectrumEncoderDecoder::calcMinAmountData()
{
	return m_PlusMinusSequence.size() * CFrame::getLength();
}

////////////////////////////////////////////////////////////////
bool CSpreadSpectrumEncoderDecoder::decodeWithRecovery(double &corr)
{
	corr = 0.0;
	bool full_recalc = false;
	int min_data = calcMinAmountData();

	// Fetch data

	switch (m_State)
	{
		////////
		case INITIALIZED:
		{
			if (m_pInput->getNumSamples() < min_data)
			{
				return false;
			}

			m_pTempBuffer->moveDataFrom(*m_pInput, min_data);

			m_State = ENOUGH_DATA;
			full_recalc = true;
			break;
		}

		////////
		case ENOUGH_DATA:
		{
			// Fetch 32 samples + recalc
			// Exit if not enough
			if (m_pInput->getNumSamples() < 64)
			{
				return false;
			}

			m_pTempBuffer->forget(64);
			m_pTempBuffer->moveDataFrom(*m_pInput, 64);
			full_recalc = true;
			break;
		}

		////////
		case ENOUGH_DATA_WATERMARK_DETECTED:
		{
			if (m_pInput->getNumSamples() < CFrame::getLength())
			{
				return false;
			}

			m_pTempBuffer->forget(CFrame::getLength());
			m_pTempBuffer->moveDataFrom(*m_pInput, CFrame::getLength());
			full_recalc = false;
			break;
		}

		////////
		default:
			assert(false);
	}

	// Do a full recalculation of the stored parameters
	if (full_recalc)
	{
		m_RunningIndex = 0;
		m_PastFrames.clear();
		m_PastSign.clear();
		m_PastH.clear();

		for (int a = 0; a < m_WatermarkSequence.size(); a++)
		{
			CFrame frame;
			frame.copyDataFromBuffer(*m_pTempBuffer, a * CFrame::getLength());
			frame.dctFwd();

			auto H = sqrt(frame.innerProduct(frame));
			auto sign = m_PlusMinusSequence[m_RunningIndex % m_PlusMinusSequence.size()];
			m_RunningIndex++;

			m_PastFrames.push_back(frame);
			m_PastH.push_back(H);
			m_PastSign.push_back(sign);
		}
	}

	// Do a partial recalculation of the stored parameters
	else
	{
		m_PastFrames.erase(m_PastFrames.begin());
		m_PastH.erase(m_PastH.begin());
		m_PastSign.erase(m_PastSign.begin());

		CFrame frame;
		frame.copyDataFromBuffer(*m_pTempBuffer, CFrame::getLength() * (m_WatermarkSequence.size() - 1));
		frame.dctFwd();

		auto H = sqrt(frame.innerProduct(frame));
		auto sign = m_PlusMinusSequence[m_RunningIndex % m_PlusMinusSequence.size()];
		m_RunningIndex++;

		m_PastFrames.push_back(frame);
		m_PastH.push_back(H);
		m_PastSign.push_back(sign);
	}

	// Calculate the correlation
	for (int i = 0; i < m_SequenceLength; i++)
	{
		for (int n = 0; n < m_SequenceRepetition - 1; n++)
		{
			for (int m = n + 1; m < m_SequenceRepetition; m++)
			{
				int mi = n * m_SequenceLength + i;
				int ni = m * m_SequenceLength + i;

				auto sign = m_PastSign[mi] * m_PastSign[ni];
				auto ycorr = m_PastFrames[mi].innerProduct(m_PastFrames[ni]);
				auto h = m_PastH[mi] * m_PastH[ni];

				if (h != 0)
				{
					corr += sign * ycorr / h;
				}
			}
		}
	}

	m_LastCorrelation = corr;

	if (corr > 2.0)
	{
		m_nHysteresis	= 3;
		m_State = ENOUGH_DATA_WATERMARK_DETECTED;
	}
	else
	{
		m_nHysteresis--;
		if (m_nHysteresis <= 0)
		{
			m_State = ENOUGH_DATA;
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////
bool CSpreadSpectrumEncoderDecoder::decodeWithoutRecovery(double &corr)
{
	corr = 0.0;
	if (m_pInput->getNumSamples() < CFrame::getLength())
	{
		return false;
	}

	CFrame output;
	output.moveDataFromBuffer(*m_pInput);
	output.dctFwd();

	auto H = sqrt(output.innerProduct(output));
	auto sign = m_PlusMinusSequence[m_RunningIndex % m_PlusMinusSequence.size()];
	m_RunningIndex++;

	m_PastFrames.push_back(output);
	m_PastH.push_back(H);
	m_PastSign.push_back(sign);

	while (m_PastFrames.size() > m_WatermarkSequence.size())
	{
		m_PastFrames.erase(m_PastFrames.begin());
		m_PastH.erase(m_PastH.begin());
		m_PastSign.erase(m_PastSign.begin());
	}

	if (m_PastFrames.size() < m_WatermarkSequence.size())
	{
		return true;
	}

	for (int i = 0; i < m_SequenceLength; i++)
	{
		for (int n = 0; n < m_SequenceRepetition - 1; n++)
		{
			for (int m = n + 1; m < m_SequenceRepetition; m++)
			{
				int mi = n * m_SequenceLength + i;
				int ni = m * m_SequenceLength + i;

				auto sign = m_PastSign[mi] * m_PastSign[ni];
				auto ycorr = m_PastFrames[mi].innerProduct(m_PastFrames[ni]);
				auto h = m_PastH[mi] * m_PastH[ni];

				if (h != 0)
				{
					corr += sign * ycorr / h;
				}
			}
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////
int CSpreadSpectrumEncoderDecoder::pushInputForDecoding(TAudioSample* buffer, int count_samples)
{
	m_pInput->addData(buffer, count_samples);
	return count_samples;
}

////////////////////////////////////////////////////////////////
int CSpreadSpectrumEncoderDecoder::pushInputForEncoding(TAudioSample* buffer, int count_samples)
{
	m_pInput->addData(buffer, count_samples);

	CFrame frame;
	while (encode(frame, true))
	{
		m_pOutput->addData(frame.getData(), frame.getLength());
	}

	return count_samples;
}

////////////////////////////////////////////////////////////////
void CSpreadSpectrumEncoderDecoder::pushInput(TAudioSample* buffer, int count_samples)
{
	m_pInput->addData(buffer, count_samples);
}

////////////////////////////////////////////////////////////////
void CSpreadSpectrumEncoderDecoder::getOutputOfEncoding(TAudioSample* buffer, int count_samples)
{
	m_pOutput->dumpData(buffer, count_samples);
}

////////////////////////////////////////////////////////////////
void CSpreadSpectrumEncoderDecoder::resetBuffersAndIndex()
{
	m_pInput->reset(0);
	m_pTempBuffer->reset(0);
	m_RunningIndex = 0;
	m_PastFrames.clear();
	m_PastSign.clear();
	m_PastH.clear();
	m_State = INITIALIZED;
}

};