//
// Created by michi on 18.05.24.
//

#if HAS_LIB_PORTAUDIO

#include "DeviceContextPort.h"
#include "AudioInputStreamPort.h"
#include "AudioOutputStreamPort.h"
#include "../DeviceManager.h"
#include "../Device.h"
#include "../../Session.h"
#include "../../lib/hui/language.h"

#include <portaudio.h>

namespace tsunami {

DeviceContextPort* DeviceContextPort::instance;

DeviceContextPort::DeviceContextPort(Session* session) : DeviceContext(session) {
	instance = this;
}

DeviceContextPort::~DeviceContextPort() {
	PaError err = Pa_Terminate();
	_test_error(err, Session::GLOBAL, "Pa_Terminate");
}

bool DeviceContextPort::init(Session* session) {
	PaError err = Pa_Initialize();
	_test_error(err, session, "Pa_Initialize");
	if (err != paNoError)
		return false;

	session->i(_("please note, that portaudio does not support refreshing the device list after program launch"));
	return true;
}

AudioOutputStream* DeviceContextPort::create_audio_output_stream(Session *session, Device *device, void* shared_data) {
	return new AudioOutputStreamPort(session, device, *reinterpret_cast<AudioOutputStream::SharedData*>(shared_data));
}

AudioInputStream* DeviceContextPort::create_audio_input_stream(Session *session, Device *device, void* shared_data) {
	return new AudioInputStreamPort(session, device, *reinterpret_cast<AudioInputStream::SharedData*>(shared_data));
}

void _portaudio_add_dev(DeviceManager *dm, DeviceType type, int index) {
	if (index < 0)
		return;
	const PaDeviceInfo* dev = Pa_GetDeviceInfo(index);
	if (!dev)
		return;
	int channels = (type == DeviceType::AUDIO_OUTPUT) ? dev->maxOutputChannels : dev->maxInputChannels;
	if (channels > 0) {
		Device *d = dm->get_device_create(type, string(Pa_GetHostApiInfo(dev->hostApi)->name) + "/" + dev->name);
		d->name = string(Pa_GetHostApiInfo(dev->hostApi)->name) + "/" + dev->name;
		d->channels = channels;
		d->index_in_lib = index;
		if (type == DeviceType::AUDIO_OUTPUT)
			d->default_by_lib = (index == Pa_GetDefaultOutputDevice());
		else
			d->default_by_lib = (index == Pa_GetDefaultInputDevice());
		d->present = true;
		dm->set_device_config(d);
	}
}

void DeviceContextPort::update_device(DeviceManager* device_manager, bool serious) {
	if (!fully_initialized)
		return;
	for (Device *d: device_manager->output_devices)
		d->present = false;
	for (Device *d: device_manager->input_devices)
		d->present = false;

	// make sure, the default is first...
	_portaudio_add_dev(device_manager, DeviceType::AUDIO_OUTPUT, Pa_GetDefaultOutputDevice());
	_portaudio_add_dev(device_manager, DeviceType::AUDIO_INPUT, Pa_GetDefaultInputDevice());

	int count = Pa_GetDeviceCount();
	for (int i=0; i<count; i++) {
		_portaudio_add_dev(device_manager, DeviceType::AUDIO_OUTPUT, i);
		_portaudio_add_dev(device_manager, DeviceType::AUDIO_INPUT, i);
	}
}
bool DeviceContextPort::_test_error(PaError err, Session *session, const string &msg) {
	if (err != paNoError) {
		session->e(msg + ": " + Pa_GetErrorText(err));
		return true;
	}
	return false;
}

}

#endif
