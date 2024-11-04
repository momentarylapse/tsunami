//
// Created by Michael Ankele on 2024-07-23.
//

#ifndef MIDIINPUTSTREAMCOREMIDI_H
#define MIDIINPUTSTREAMCOREMIDI_H

#if HAS_LIB_COREMIDI

#include "../interface/MidiInputStream.h"
#include "../../data/midi/MidiData.h"

namespace tsunami {

	class MidiInputStreamCoreMidi : public MidiInputStream {
	public:
		MidiInputStreamCoreMidi(Session* session, Device* device, SharedData& shared_data);
		~MidiInputStreamCoreMidi() override;

		bool start() override;
		bool stop() override;
		bool connect(Device* device);
		bool unconnect();
		void clear_input_queue() override;
		void read(MidiEventBuffer& buffer) override;

		int port = -1;
		int endpoit_ref = -1;
		MidiEventBuffer buffer;
		bool active = false;
	};

}

#endif

#endif //MIDIINPUTSTREAMCOREMIDI_H
