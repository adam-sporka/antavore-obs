#pragma once

namespace antavore
{

class CSignalEncodingPipeline
{
private:
	CImprinter imprinter;
	CSpreadSpectrumEncoderDecoder encoder;

public:
	CSignalEncodingPipeline();

	int process(TAudioSample* buffer, int count_samples);
	void changeMessage(const char *message);
};

}
