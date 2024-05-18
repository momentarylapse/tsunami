//
// Created by michi on 5/11/24.
//

#include "AudioInputStream.h"
#include "../../Session.h"

AudioInputStream::SharedData::SharedData() :
		buffer(1048576) {
	chunk_size = DEFAULT_CHUNK_SIZE;
}



AudioInputStream::AudioInputStream(Session *_session, SharedData& _shared_data)
		: shared_data(_shared_data)
{
	session = _session;

	device_manager = session->device_manager;
}

AudioInputStream::~AudioInputStream() = default;
