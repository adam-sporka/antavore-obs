#include <math.h>
#include <util/bmem.h>
#include <obs.h>

#include <stdio.h>
#include <stdlib.h>

void create_encoder(void **l, void **r);
void apply_encoder(float *buffer, int num_frames, void *encoder, bool message_changed, char *message);

////////////////////////////////////////////////////////////////
struct antavore_plugin_instance {
	obs_source_t *context;
	size_t channels;
	uint32_t sample_rate;
	char message[64];
	bool message_changed;

	void *antavore_L;
	void *antavore_R;
};

////////////////////////////////////////////////////////////////
static const char *antavore_plugin_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return "Antavore Watermark Encoder";
}

////////////////////////////////////////////////////////////////
static void antavore_plugin_update(void *data, obs_data_t *s)
{
	struct antavore_plugin_instance *info = data;
	info->channels = audio_output_get_channels(obs_get_audio());
	info->sample_rate = audio_output_get_sample_rate(obs_get_audio());

	char *c = (char*)obs_data_get_string(s, "Message");
	int length = strlen(c);
	if (length > 63)
		length = 63;

	if (strcmp(info->message, c)) {
		memset(info->message, 0, 64);
		memcpy(info->message, c, length);
		info->message_changed = true;
	}
}

////////////////////////////////////////////////////////////////
static void *antavore_plugin_create(obs_data_t *settings, obs_source_t *source)
{
	struct antavore_plugin_instance *gf = bzalloc(sizeof(struct antavore_plugin_instance));
	gf->context = source;
	antavore_plugin_update(gf, settings);
	create_encoder(&gf->antavore_L, &gf->antavore_R);

	return gf;
}

////////////////////////////////////////////////////////////////
static void antavore_plugin_destroy(void *data)
{
	struct antavore_plugin_instance *info = data;
	bfree(info);
}

////////////////////////////////////////////////////////////////
static struct obs_audio_data *antavore_plugin_filter_audio(void *data,
					   struct obs_audio_data *audio)
{
	struct antavore_plugin_instance *info = data;
	auto channels = info->channels;
	float **adata = (float **)audio->data;

	for (size_t c = 0; c < channels; c++) {
		if (audio->data[c]) {
			if (c == 0) apply_encoder(adata[c], audio->frames, info->antavore_L, info->message_changed, info->message);
			if (c == 1) apply_encoder(adata[c], audio->frames, info->antavore_R, info->message_changed, info->message);
		}
	}

	info->message_changed = false;

	return audio;
}

////////////////////////////////////////////////////////////////
static obs_properties_t *antavore_plugin_get_properties(void *data)
{
	obs_property_t *p;
	obs_properties_t *props = obs_properties_create();
	obs_properties_add_text(props, "Message", "Imprint message",
				OBS_TEXT_DEFAULT);

	UNUSED_PARAMETER(data);

	return props;
}

////////////////////////////////////////////////////////////////
struct obs_source_info my_source = {
	.id = "antavore",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_AUDIO,
	.get_name = antavore_plugin_get_name,
	.create = antavore_plugin_create,
	.destroy = antavore_plugin_destroy,
	.filter_audio = antavore_plugin_filter_audio,
	.get_properties = antavore_plugin_get_properties,
	.update = antavore_plugin_update
};
