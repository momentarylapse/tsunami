//
// Created by michi on 6/7/24.
//

#if HAS_LIB_PIPEWIRE

#include "AudioInputStreamPipewire.h"
#include "DeviceContextPipewire.h"
#include "../Device.h"
#include "../../lib/os/msg.h"
#include "../../Session.h"

#include <spa/pod/builder.h>
#include <spa/param/audio/raw.h>
#include <spa/param/audio/raw-utils.h>
#include <pipewire/pipewire.h>

namespace tsunami {

static void on_pipewire_process(void *userdata) {
	auto stream = reinterpret_cast<AudioInputStreamPipewire *>(userdata);

	struct pw_buffer *b;
	if ((b = pw_stream_dequeue_buffer(stream->stream)) == nullptr) {
		pw_log_warn("out of buffers: %m");
		return;
	}

	struct spa_buffer *buf = b->buffer;
	float *dst;
	if ((dst = (float*)buf->datas[0].data) == nullptr)
		return;

	int channels = stream->shared_data.num_channels;
	int stride = sizeof(float) * channels;
	int num_frames = buf->datas[0].chunk->size / stride;
	if (b->requested)
		num_frames = SPA_MIN(b->requested, num_frames);

	stream->handle_input(dst, num_frames);

	pw_stream_queue_buffer(stream->stream, b);
}

static void on_stream_param_changed(void *_data, uint32_t id, const struct spa_pod *param) {
	auto stream = reinterpret_cast<AudioInputStreamPipewire*>(_data);

	/* NULL means to clear the format */
	if (param == nullptr or id != SPA_PARAM_Format)
		return;

	spa_audio_info format;
	if (spa_format_parse(param, &format.media_type, &format.media_subtype) < 0)
		return;

	/* only accept raw audio */
	if (format.media_type != SPA_MEDIA_TYPE_audio or format.media_subtype != SPA_MEDIA_SUBTYPE_raw)
		return;

	/* call a helper function to parse the format for us. */
	spa_format_audio_raw_parse(param, &format.info.raw);

	stream->shared_data.num_channels = (int) format.info.raw.channels;

	fprintf(stdout, "capturing rate:%d channels:%d\n",
	        format.info.raw.rate, format.info.raw.channels);

}

static const struct pw_stream_events stream_events = {
		.version = PW_VERSION_STREAM_EVENTS,
		.param_changed = on_stream_param_changed,
		.process = on_pipewire_process
};

AudioInputStreamPipewire::AudioInputStreamPipewire(Session *session, Device *device, SharedData &shared_data)
		: AudioInputStream(session, shared_data) {
	const spa_pod *params[1];
	uint8_t buffer[1024];
	static spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

	DeviceContextPipewire::instance->lock();

	stream = pw_stream_new_simple(
			pw_thread_loop_get_loop(DeviceContextPipewire::instance->loop),
			"audio-capture",
			pw_properties_new(
					PW_KEY_MEDIA_TYPE, "Audio",
					PW_KEY_MEDIA_CATEGORY, "Capture",
					PW_KEY_MEDIA_ROLE, "Production",
					nullptr),
			&stream_events,
			this);

	spa_audio_info_raw format = {
			.format = SPA_AUDIO_FORMAT_F32,
			.flags = 0,
			.rate = (uint32_t) session->sample_rate(),
			.channels = 2 //(uint32_t)device->channels
	};
	params[0] = spa_format_audio_raw_build(&builder, SPA_PARAM_EnumFormat, &format);

	/*msg_write("------------>");
	msg_write(device->internal_name);
	msg_write(device->name);*/
	int id = device->index_in_lib;
	//msg_write(id);
	if (device->is_default())
		id = PW_ID_ANY;
	//msg_write(id);
	//msg_write(i2s(device->channels) + "  /  " + i2s(this->shared_data.num_channels));

	int r = pw_stream_connect(stream,
	                          PW_DIRECTION_INPUT,
	                          id,
	                          pw_stream_flags(PW_STREAM_FLAG_AUTOCONNECT |
	                                          PW_STREAM_FLAG_INACTIVE |
	                                          PW_STREAM_FLAG_MAP_BUFFERS |
	                                          PW_STREAM_FLAG_RT_PROCESS),
	                          params, 1);
	//msg_write(::format("connect: %d", r));

	DeviceContextPipewire::instance->unlock();
}

AudioInputStreamPipewire::~AudioInputStreamPipewire() {
	DeviceContextPipewire::instance->lock();
	int r = pw_stream_disconnect(stream);
	//msg_write(format("disconnect: %d", r));
	pw_stream_destroy(stream);
	DeviceContextPipewire::instance->unlock();

}

void AudioInputStreamPipewire::pause() {
	DeviceContextPipewire::instance->lock();
	int r = pw_stream_set_active(stream, false);
	//msg_write(format("pause: %d", r));
	DeviceContextPipewire::instance->unlock();
}

void AudioInputStreamPipewire::unpause() {
	DeviceContextPipewire::instance->lock();
	int r = pw_stream_set_active(stream, true);
	//msg_write(format("unpause: %d", r));
	DeviceContextPipewire::instance->unlock();

}

base::optional<int64> AudioInputStreamPipewire::estimate_samples_captured() {
	return base::None;
}

}

#endif
