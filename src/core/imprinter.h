#pragma once

namespace antavore
{

class CImprinter
{
private:
	CCircularBuffer *m_pInput;
	CCircularBuffer *m_pOutput;

	char m_sMessage[1024];
	int m_nLetter = 0;
	int m_nColumn = 0;
	int m_nSubcolumn = 0;
	int m_nTimeout = 100;

	int m_nBand = 160;
	double m_Strength = 0.001;

	struct OPERATOR
	{
		double m_Angle = 0.0;
		double m_Frequency = 0.0;
		double m_Volume = 0.0;
		double m_TargetVolume = 0.0;
		void fade()
		{
			if (m_TargetVolume > m_Volume) {
				m_Volume += 0.0009765625;
				if (m_Volume > m_TargetVolume) m_Volume = m_TargetVolume;
			}
			else if (m_TargetVolume < m_Volume) {
				m_Volume -= 0.0009765625;
				if (m_Volume < m_TargetVolume) m_Volume = m_TargetVolume;
			}
		}
		double getSample(double strength)
		{
			m_Angle += m_Frequency * (2.0 * 3.1415926535 / 48000.0);
			if (m_Angle > 7) m_Angle -= 2.0 * 3.1415926535;
			auto value = strength * m_Volume * sin(m_Angle);
			return value;
		}
		void setFrequency(double frequency)
		{
			m_Frequency = frequency;
		}
	};

	OPERATOR m_Operators[8];

public:
	CImprinter();

	void setMessage(const char *message);
	void resetGenerator();
	void setStrength(double strength) { m_Strength = strength; };

	int pushInput(TAudioSample* buffer, int count_samples);
	bool getOutput(TAudioSample* buffer, int count_samples);
	bool oneFrame();

	~CImprinter();
};

}
