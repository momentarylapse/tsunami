//
// Created by michi on 18.05.24.
//

#include "MidiInputStream.h"
#include "../../Session.h"

namespace tsunami {

MidiInputStream::SharedData::SharedData() : buffer(4096) {
}

MidiInputStream::MidiInputStream(Session *_session, SharedData& _shared_data)
		: shared_data(_shared_data)
{
	session = _session;

	device_manager = session->device_manager;
}

MidiInputStream::~MidiInputStream() = default;

void MidiInputStream::clear_input_queue() {
	shared_data.buffer.clear();
}

}
