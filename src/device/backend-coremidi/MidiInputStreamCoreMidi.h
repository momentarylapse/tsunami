//
// Created by Michael Ankele on 2024-07-23.
//

#ifndef MIDIINPUTSTREAMCOREMIDI_H
#define MIDIINPUTSTREAMCOREMIDI_H

#if HAS_LIB_COREMIDI

#include "../interface/MidiInputStream.h"

namespace tsunami {

	class MidiInputStreamCoreMidi : public MidiInputStream {
	public:
		MidiInputStreamCoreMidi(Session* session, Device* device, SharedData& shared_data);
		~MidiInputStreamCoreMidi() override;

		bool start() override;
		bool stop() override;
		bool connect(Device* device);
		bool unconnect();

		int port = -1;
		int endpoit_ref = -1;
		bool active = false;
	};

}

#endif

#endif //MIDIINPUTSTREAMCOREMIDI_H
