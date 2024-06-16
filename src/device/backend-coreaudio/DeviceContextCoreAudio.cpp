#if HAS_LIB_COREAUDIO

#include "DeviceContextCoreAudio.h"
#include "AudioOutputStreamCoreAudio.h"
#include "../DeviceManager.h"
#include "../Device.h"
#include "../../Session.h"
#include "../../lib/hui/language.h"

namespace tsunami {

DeviceContextCoreAudio* DeviceContextCoreAudio::instance;

DeviceContextCoreAudio::DeviceContextCoreAudio(Session* session) : DeviceContext(session) {
	instance = this;
    session->w("CoreAudio backend is still highly experimental!\nRecording not supported yet.\nIf you encounter any problems, please use portaudio instead.");
}

DeviceContextCoreAudio::~DeviceContextCoreAudio() {
}

bool DeviceContextCoreAudio::init(Session* session) {
	return true;
}

AudioOutputStream* DeviceContextCoreAudio::create_audio_output_stream(Session *session, Device *device, void* shared_data) {
	return new AudioOutputStreamCoreAudio(session, device, *reinterpret_cast<AudioOutputStream::SharedData*>(shared_data));
}

//AudioInputStream* DeviceContextCoreAudio::create_audio_input_stream(Session *session, Device *device, void* shared_data) {
//	return new AudioInputStreamCoreAudio(session, device, *reinterpret_cast<AudioInputStream::SharedData*>(shared_data));
//}


void DeviceContextCoreAudio::update_device(DeviceManager* device_manager, bool serious) {
	if (!fully_initialized)
		return;


	// system default
	auto *def = device_manager->get_device_create(DeviceType::AUDIO_OUTPUT, "");
	def->channels = 2;
	def->default_by_lib = true;
	def->present = true;
}

}

#endif
