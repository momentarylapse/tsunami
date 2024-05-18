//
// Created by michi on 5/11/24.
//

#ifndef TSUNAMI_AUDIOINPUTSTREAM_H
#define TSUNAMI_AUDIOINPUTSTREAM_H


#include "../../lib/base/base.h"
#include "../../lib/base/optional.h"
#include "../../data/audio/RingBuffer.h"
#include <atomic>
#include <functional>

class Session;
class Device;
class DeviceManager;

class AudioInputStream {
public:
	struct SharedData {
		SharedData();

		RingBuffer buffer;

		int num_channels;
		int chunk_size;
	};

	AudioInputStream(Session* session, SharedData& shared_data);
	virtual ~AudioInputStream();


	static constexpr int DEFAULT_CHUNK_SIZE = 512;

	virtual void pause() = 0;
	virtual void unpause() = 0;
	virtual base::optional<int64> estimate_samples_captured() = 0;

	Session *session;
	DeviceManager *device_manager;

	bool error = false;

	SharedData& shared_data;
};

#endif //TSUNAMI_AUDIOINPUTSTREAM_H
