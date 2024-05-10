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
	AudioOutputStream(Session* session);
	virtual ~AudioOutputStream();

	virtual void pause() = 0;
	virtual void unpause() = 0;
	virtual void flush() = 0;
	virtual base::optional<int64> estimate_samples_played() = 0;

	Session *session;
	DeviceManager *device_manager;

	static const int DEFAULT_PREBUFFER_SIZE;

	/*void signal_out_of_data();

	std::atomic<bool> read_end_of_stream;
	std::atomic<bool> played_end_of_stream;


	std::function<void()> callback_read_end_of_stream;
	std::function<void()> callback_played_end_of_stream;*/

	bool error = false;
};


#endif //TSUNAMI_AUDIOOUTPUTSTREAM_H
