//
// Created by michi on 5/10/24.
//

#include "AudioOutputStream.h"
#include "DeviceManager.h"
#include "../Session.h"
#include "../lib/hui/hui.h"

static const bool STREAM_WARNINGS = true;

AudioOutputStream::AudioOutputStream(Session *_session)
{
	session = _session;

	//prebuffer_size = hui::config.get_int("Output.BufferSize", DEFAULT_PREBUFFER_SIZE);

	device_manager = session->device_manager;

//	read_end_of_stream = false;
//	played_end_of_stream = false;
}

AudioOutputStream::~AudioOutputStream() = default;

/*void AudioOutputStream::signal_out_of_data() {
	if (read_end_of_stream and !played_end_of_stream) {
		//printf("end of data...\n");
		played_end_of_stream = true;
		//hui::run_later(0.001f, [stream]{ stream->on_played_end_of_stream(); }); // TODO prevent abort before playback really finished
	}
	hui::run_later(0.001f, [this] { callback_played_end_of_stream(); });
}*/