#pragma once

#include <vector>
#include "antavore_definitions.h"

namespace antavore
{

////////////////////////////////////////////////////////////////
class CSpreadSpectrumEncoderDecoder
{
private:
	TSpectrum m_ppWatermarkSpectrum[g_nMscBandCount];
	std::vector<int> m_WatermarkSequence; // 1 item = 5 bits
	std::vector<int> m_PlusMinusSequence; // -1, +1
	int m_RunningIndex = 0;

	CCircularBuffer *m_pInput = nullptr;
	CCircularBuffer *m_pOutput = nullptr;

	// For non-detection recovery
	CCircularBuffer *m_pTempBuffer = nullptr;

	// For past detection data
	std::vector<CFrame> m_PastFrames;
	std::vector<double> m_PastH;
	std::vector<double> m_PastSign;

	int m_SequenceLength = 0;
	int m_SequenceRepetition = 0;
	double m_Strength = 0.1;
	int m_nHysteresis = 10;

	enum EDecoderState
	{
		INITIALIZED,
		ENOUGH_DATA,
		ENOUGH_DATA_WATERMARK_DETECTED
	};

public:
	EDecoderState m_State = INITIALIZED;
	double m_LastCorrelation = 0.0;

public:
	CSpreadSpectrumEncoderDecoder();
	~CSpreadSpectrumEncoderDecoder();

	int calcMinAmountData();
	void setStrength(double strength) { m_Strength = strength; }

	void resetBuffersAndIndex();
	void pushInput(TAudioSample* buffer, int count_samples);
	int pushInputForEncoding(TAudioSample* buffer, int count_samples);
	void getOutputOfEncoding(TAudioSample* buffer, int count_samples);

	int pushInputForDecoding(TAudioSample* buffer, int count_samples);


	void initializeSpectrum(); // Random spectrum
	void saveSpectrum(const char *filename); // Save spectrum to a binary file
	void loadSpectrum(const char *filename); // Load spectrum from a binary file
	void fetchSpectrumFromArray(); // Load the "factory-provided" spectrum

	void initializeRandomSequence(const char *sequence);
	void initializeWatermarkSequence(int repetition, std::vector<int> order);

	bool encode(CFrame &output, bool apply_watermark);

	bool decodeWithoutRecovery(double &corr);
	bool decodeWithRecovery(double &corr);

	TAudioCursor getNumHandled() { return m_pTempBuffer->getStartAbs(); }
};

} 