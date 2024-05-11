//
// Created by michi on 5/10/24.
//

#include "AudioOutputStream.h"
#include "DeviceManager.h"
#include "../Session.h"
#include "../lib/hui/hui.h"

static const bool STREAM_WARNINGS = true;

AudioOutputStream::SharedData::SharedData() {
	read_end_of_stream = false;
	played_end_of_stream = false;
}

AudioOutputStream::AudioOutputStream(Session *_session, SharedData& _shared_data)
	: shared_data(_shared_data)
{
	session = _session;

	//prebuffer_size = hui::config.get_int("Output.BufferSize", DEFAULT_PREBUFFER_SIZE);

	device_manager = session->device_manager;
}

AudioOutputStream::~AudioOutputStream() = default;

void AudioOutputStream::signal_out_of_data() {
	if (shared_data.read_end_of_stream and !shared_data.played_end_of_stream) {
		//printf("end of data...\n");
		shared_data.played_end_of_stream = true;
		hui::run_later(0.001f, [this]{ shared_data.callback_played_end_of_stream(); }); // TODO prevent abort before playback really finished
	}
}

