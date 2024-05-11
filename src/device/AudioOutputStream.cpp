//
// Created by michi on 5/10/24.
//

#include "AudioOutputStream.h"
#include "DeviceManager.h"
#include "../Session.h"
#include "../lib/hui/hui.h"

const int AudioOutputStream::DEFAULT_PREBUFFER_SIZE = 4096;

static const bool STREAM_WARNINGS = true;

AudioOutputStream::SharedData::SharedData():
		ring_buf(1048576)
{
	read_end_of_stream = false;
	played_end_of_stream = false;

	prebuffer_size = hui::config.get_int("Output.BufferSize", DEFAULT_PREBUFFER_SIZE);

	buffer_is_cleared = true;
}

// return: underrun?
bool AudioOutputStream::SharedData::feed_stream_output(int frames_request, float *out) {

	//printf("feed %d\n", frames_request);
	int available = ring_buf.available();
	int frames = min(frames_request, available);
	samples_requested += frames_request;


	//printf("av=%d r=%d reos=%d\n", available, stream->reading.load(), stream->read_end_of_stream.load());

	// read 2x in case of wrap-around
	int done = 0;
	for (int n=0; (n<2) and (done < frames); n++) {
		AudioBuffer b;
		ring_buf.read_ref(b, frames - done);
		b.interleave(out, volume);
		ring_buf.read_ref_done(b);
		out += b.length * 2;
		done += b.length;
	}
	done = frames;


	// underflow? -> add silence
	if (available < frames_request) {
		if (!read_end_of_stream and !buffer_is_cleared)
			if (STREAM_WARNINGS)
				printf("< underflow  %d < %d\n", available, frames_request);
		// output silence...
		fake_samples_played += frames_request - done;
		for (int i=done; i<frames_request; i++) {
			*out ++ = 0;
			*out ++ = 0;
		}
		return true;
	}

	return false;
}

void AudioOutputStream::SharedData::clear_data_state() {
	ring_buf.clear();
	buffer_is_cleared = true;
	read_end_of_stream = false;
	played_end_of_stream = false;
	samples_requested = 0;
	fake_samples_played = 0;
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

