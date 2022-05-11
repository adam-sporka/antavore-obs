#include "core/antavore_definitions.h"
#include <time.h>

antavore::TAudioSample temp[65536];

////////////////////////////////////////////////////////////////
extern "C" void create_encoder(void **l, void **r)
{
	time_t t;
	time(&t);

	*l = new antavore::CSignalEncodingPipeline();
	*r = new antavore::CSignalEncodingPipeline();
}

////////////////////////////////////////////////////////////////
extern "C" void apply_encoder(float *buffer, int num_frames, void *encoder,
			      bool message_changed, char *message)
{
	antavore::CSignalEncodingPipeline *the_encoder =
		(antavore::CSignalEncodingPipeline *)encoder;

	for (int a = 0; a < num_frames; a++)
		temp[a] = ((float *)buffer)[a];

	if (message_changed) the_encoder->changeMessage(message);
	the_encoder->process(temp, num_frames);

	for (int a = 0; a < num_frames; a++)
		((float *)buffer)[a] = temp[a];
}
