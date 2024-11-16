//
// Created by michi on 18.05.24.
//

#ifndef TSUNAMI_MIDIINPUTSTREAM_H
#define TSUNAMI_MIDIINPUTSTREAM_H

#include "../../data/midi/MidiEventRingBuffer.h"

namespace tsunami {

class Session;
class Device;
class DeviceManager;
class MidiEventBuffer;


class MidiInputStream {
public:
	struct SharedData {
		SharedData();

		int portid = -1;
		MidiEventRingBuffer buffer;
	};

	MidiInputStream(Session* session, SharedData& shared_data);
	virtual ~MidiInputStream();

	virtual bool start() = 0;
	virtual bool stop() = 0;
	virtual void clear_input_queue();
	virtual void tick() {} // in case we have to poke the library (alsa)

	/*virtual void pause() = 0;
	virtual void unpause() = 0;
	virtual base::optional<int64> estimate_samples_captured() = 0;*/

	Session *session;
	DeviceManager *device_manager;

	bool error = false;

	SharedData& shared_data;
};

}

#endif //TSUNAMI_MIDIINPUTSTREAM_H
