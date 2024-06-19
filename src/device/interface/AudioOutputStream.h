//
// Created by michi on 5/10/24.
//

#ifndef TSUNAMI_AUDIOOUTPUTSTREAM_H
#define TSUNAMI_AUDIOOUTPUTSTREAM_H


#include "../../lib/base/base.h"
#include "../../lib/base/optional.h"
#include "../../data/audio/RingBuffer.h"
#include <atomic>
#include <functional>

namespace tsunami {

class Session;
class Device;
class DeviceManager;

class AudioOutputStream {
public:
	struct SharedData {
		SharedData();

		bool feed_stream_output(int frames_request, float *out);

		void clear_data_state();

		RingBuffer ring_buf;
		bool buffer_is_cleared;
		int prebuffer_size;
		int64 samples_offset_since_reset = 0;

		int64 samples_requested = 0;
		int64 fake_samples_played = 0;

		std::atomic<float> volume;

		std::atomic<bool> read_end_of_stream;
		std::atomic<bool> played_end_of_stream;

		std::function<void()> callback_played_end_of_stream;
	};

	AudioOutputStream(Session* session, SharedData& shared_data);
	virtual ~AudioOutputStream();

	virtual void pause() = 0;
	virtual void unpause() = 0;
	virtual void pre_buffer() {}
	virtual void flush() = 0;
	virtual base::optional<int64> estimate_samples_played() = 0;

	Session *session;
	DeviceManager *device_manager;

	static const int DEFAULT_PREBUFFER_SIZE;

	void signal_out_of_data();

	bool error = false;

	SharedData& shared_data;
};

}

#endif //TSUNAMI_AUDIOOUTPUTSTREAM_H
