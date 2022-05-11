#include "antavore_definitions.h"

namespace antavore
{

////////////////////////////////////////////////////////////////
CSignalEncodingPipeline::CSignalEncodingPipeline()
{
	// Dummy buffer
	TAudioSample silence[1024];
	memset(silence, 0, sizeof(TAudioSample) * 1024);

	imprinter.setStrength(0.001);
	imprinter.setMessage(" ");
	imprinter.pushInput(silence, 1024);

	encoder.setStrength(0.1);
	encoder.fetchSpectrumFromArray();
	encoder.initializeWatermarkSequence(4, { 0, 10, 11, 15, 29, 20 });
	encoder.initializeRandomSequence("-++---++++-----++++++---");
	encoder.pushInputForEncoding(silence, 1024);
}

////////////////////////////////////////////////////////////////
int CSignalEncodingPipeline::process(TAudioSample* buffer, int count_samples)
{
	imprinter.pushInput(buffer, count_samples);
	imprinter.getOutput(buffer, count_samples);

	encoder.pushInputForEncoding(buffer, count_samples);
	encoder.getOutputOfEncoding(buffer, count_samples);

	return true;
}

////////////////////////////////////////////////////////////////
void CSignalEncodingPipeline::changeMessage(const char* message)
{
	imprinter.setMessage(message);
}

}
