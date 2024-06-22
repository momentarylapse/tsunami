//
// Created by michi on 5/11/24.
//

#include "AudioInputStream.h"
#include "../../Session.h"

namespace tsunami {

AudioInputStream::SharedData::SharedData() :
		buffer(1048576) {
	chunk_size = DEFAULT_CHUNK_SIZE;
}


AudioInputStream::AudioInputStream(Session *_session, SharedData &_shared_data)
		: shared_data(_shared_data) {
	session = _session;

	device_manager = session->device_manager;
}

AudioInputStream::~AudioInputStream() = default;

void AudioInputStream::handle_input(const float *data, int num_samples) {
	int num_channels = shared_data.num_channels;

	auto &ring_buf = shared_data.buffer;
	AudioBuffer b;
	ring_buf.write_ref(b, num_samples);
	b.deinterleave(data, num_channels);
	ring_buf.write_ref_done(b);

	int done = b.length;
	if (done < num_samples) {
		ring_buf.write_ref(b, num_samples - done);
		b.deinterleave(&data[num_channels * done], num_channels);
		ring_buf.write_ref_done(b);
	}
}

}
