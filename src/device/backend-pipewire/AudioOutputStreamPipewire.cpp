//
// Created by michi on 18.05.24.
//

#if HAS_LIB_PIPEWIRE

#include "AudioOutputStreamPipewire.h"
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
	auto stream = reinterpret_cast<AudioOutputStreamPipewire*>(userdata);

	struct pw_buffer *b;
	if ((b = pw_stream_dequeue_buffer(stream->stream)) == nullptr) {
		pw_log_warn("out of buffers: %m");
		return;
	}

	struct spa_buffer *buf = b->buffer;
	float *dst;
	if ((dst = (float*)buf->datas[0].data) == nullptr)
		return;

	int stride = sizeof(float) * 2; // channels
	int num_frames = buf->datas[0].maxsize / stride;
	if (b->requested)
		num_frames = SPA_MIN(b->requested, num_frames);
	
	//printf("..%d\n", num_frames);

	bool out_of_data = stream->shared_data.feed_stream_output(num_frames, dst);

	if (out_of_data)
		stream->signal_out_of_data();

	buf->datas[0].chunk->offset = 0;
	buf->datas[0].chunk->stride = stride;
	buf->datas[0].chunk->size = num_frames * stride;

	pw_stream_queue_buffer(stream->stream, b);
}

void on_pipewire_drawined(void *userdata) {
	auto stream = reinterpret_cast<AudioOutputStreamPipewire*>(userdata);
	printf("drained\n");
}

static const struct pw_stream_events stream_events = {
	.version = PW_VERSION_STREAM_EVENTS,
	.process = on_pipewire_process,
	.drained = on_pipewire_drawined
};

AudioOutputStreamPipewire::AudioOutputStreamPipewire(Session *session, Device *device, SharedData& shared_data) : AudioOutputStream(session, shared_data) {
	const spa_pod *params[1];
	uint8_t buffer[1024];
	static spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

	DeviceContextPipewire::instance->lock();

	stream = pw_stream_new_simple(
			pw_thread_loop_get_loop(DeviceContextPipewire::instance->loop),
			"audio-src",
			pw_properties_new(
					PW_KEY_MEDIA_TYPE, "Audio",
					PW_KEY_MEDIA_CATEGORY, "Playback",
					PW_KEY_MEDIA_ROLE, "Production",
					nullptr),
			&stream_events,
			this);

	spa_audio_info_raw format = {
			.format = SPA_AUDIO_FORMAT_F32,
			.flags = 0,
			.rate = (uint32_t)session->sample_rate(),
			.channels = (uint32_t)device->channels
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

	int r = pw_stream_connect(stream,
	                  PW_DIRECTION_OUTPUT,
	                  id,
	                  pw_stream_flags(PW_STREAM_FLAG_AUTOCONNECT |
			                                  PW_STREAM_FLAG_INACTIVE |
	                                  PW_STREAM_FLAG_MAP_BUFFERS |
	                                  PW_STREAM_FLAG_RT_PROCESS),
	                  params, 1);
	//msg_write(::format("connect: %d", r));

	DeviceContextPipewire::instance->unlock();
}

AudioOutputStreamPipewire::~AudioOutputStreamPipewire() {
	DeviceContextPipewire::instance->lock();
	int r = pw_stream_disconnect(stream);
	//msg_write(format("disconnect: %d", r));
	pw_stream_destroy(stream);
	DeviceContextPipewire::instance->unlock();
}

void AudioOutputStreamPipewire::pause() {
	DeviceContextPipewire::instance->lock();
	int r = pw_stream_set_active(stream, false);
	//msg_write(format("pause: %d", r));
	DeviceContextPipewire::instance->unlock();
}

void AudioOutputStreamPipewire::unpause() {
	DeviceContextPipewire::instance->lock();
	int r = pw_stream_set_active(stream, true);
	//msg_write(format("unpause: %d", r));
	DeviceContextPipewire::instance->unlock();
}

void AudioOutputStreamPipewire::pre_buffer() {

}

void AudioOutputStreamPipewire::flush() {
	DeviceContextPipewire::instance->lock();
	//pw_stream_flush(stream, true);
	DeviceContextPipewire::instance->unlock();
}

base::optional<int64> AudioOutputStreamPipewire::estimate_samples_played() {
	return base::None;
}

}

#endif