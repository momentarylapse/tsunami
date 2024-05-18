//
// Created by michi on 18.05.24.
//

#ifndef TSUNAMI_DEVICECONTEXT_H
#define TSUNAMI_DEVICECONTEXT_H

#include "../../lib/pattern/Observable.h"

class AudioInputStream;
class AudioOutputStream;
class MidiInputStream;
class DeviceManager;
class Device;
class Session;

class DeviceContext : public obs::Node<VirtualBase> {
public:
	DeviceContext(Session* session);

	obs::source out_request_update{this, "request-update"};
	obs::xsource<Device> out_device_found{this, "device-found"};

	virtual bool init(Session* session) = 0;

	virtual void update_device(DeviceManager* device_manager, bool serious) = 0;
	virtual AudioOutputStream* create_audio_output_stream(Session *session, Device *device, void* shared_data) { return nullptr; }
	virtual AudioInputStream* create_audio_input_stream(Session *session, Device *device, void* shared_data) { return nullptr; }
	virtual MidiInputStream* create_midi_input_stream(Session *session, Device *device, void* shared_data) { return nullptr; }

	//Session* session;
	bool fully_initialized = false;
};


#endif //TSUNAMI_DEVICECONTEXT_H
