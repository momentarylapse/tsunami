//
// Created by michi on 5/10/24.
//

#ifndef TSUNAMI_AUDIOOUTPUTSTREAM_H
#define TSUNAMI_AUDIOOUTPUTSTREAM_H


#include "../lib/base/base.h"
#include "../lib/base/optional.h"
#include "../data/audio/RingBuffer.h"
#include <atomic>
#include <functional>

class Session;
class Device;
class DeviceManager;

class AudioOutputStream {
public:
	struct SharedData {
		SharedData();
		std::atomic<bool> read_end_of_stream;
		std::atomic<bool> played_end_of_stream;
		std::function<bool(float*,int)> callback_feed;
		std::function<void()> callback_played_end_of_stream;
	};

	AudioOutputStream(Session* session, SharedData& shared_data);
	virtual ~AudioOutputStream();

	virtual void pause() = 0;
	virtual void unpause() = 0;
	virtual void pre_buffer() {}
	virtual int64 flush(int64 samples_offset_since_reset, int64 samples_requested) = 0;
	virtual base::optional<int64> estimate_samples_played(int64 samples_offset_since_reset, int64 samples_requested) = 0;

	Session *session;
	DeviceManager *device_manager;

	static const int DEFAULT_PREBUFFER_SIZE;

	void signal_out_of_data();

	bool error = false;

	SharedData& shared_data;
};


#endif //TSUNAMI_AUDIOOUTPUTSTREAM_H
