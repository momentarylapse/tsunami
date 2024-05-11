//
// Created by michi on 5/11/24.
//

#ifndef TSUNAMI_AUDIOINPUTSTREAM_H
#define TSUNAMI_AUDIOINPUTSTREAM_H


#include "../lib/base/base.h"
#include "../lib/base/optional.h"
#include "../data/audio/RingBuffer.h"
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
	};

	AudioInputStream(Session* session, SharedData& shared_data);
	virtual ~AudioInputStream();

	virtual void pause() = 0;
	virtual void unpause() = 0;

	Session *session;
	DeviceManager *device_manager;

	bool error = false;

	SharedData& shared_data;
};

#endif //TSUNAMI_AUDIOINPUTSTREAM_H
