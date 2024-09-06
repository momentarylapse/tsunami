#include <hui/Callback.h>
#if HAS_LIB_COREAUDIO

//#include <CoreServices/CoreServices.h>
#include <CoreAudio/CoreAudio.h>

#include "DeviceContextCoreAudio.h"
#include "AudioInputStreamCoreAudio.h"
#include "AudioOutputStreamCoreAudio.h"
#include "../DeviceManager.h"
#include "../Device.h"
#include "../../Session.h"


//	https://developer.apple.com/library/archive/documentation/MusicAudio/Conceptual/CoreAudioOverview/ARoadmaptoCommonTasks/ARoadmaptoCommonTasks.html#//apple_ref/doc/uid/TP40003577-CH6-SW1
//	https://developer.apple.com/library/archive/technotes/tn2091/_index.html
//	https://developer.apple.com/library/archive/samplecode/CAPlayThrough/Listings/AudioDevice_cpp.html#//apple_ref/doc/uid/DTS10004443-AudioDevice_cpp-DontLinkElementID_5

namespace tsunami {

DeviceContextCoreAudio* DeviceContextCoreAudio::instance;

DeviceContextCoreAudio::DeviceContextCoreAudio(Session* session) : DeviceContext(session) {
	instance = this;
    session->w("CoreAudio backend is still experimental!\nIf you encounter any problems, please use portaudio instead.");
}

DeviceContextCoreAudio::~DeviceContextCoreAudio() = default;

OSStatus on_coreaudio_devices_changed(AudioObjectID id, UInt32 num_addresses, const AudioObjectPropertyAddress* aAddresses, void* client_data) {
	auto dm = reinterpret_cast<DeviceContextCoreAudio*>(client_data);
	hui::run_in_gui_thread([dm] {
		dm->out_request_update();
	});
	return 0;
}

bool DeviceContextCoreAudio::init(Session* session) {
	AudioObjectPropertyAddress theAddress0 = {
		kAudioHardwarePropertyDevices,
		kAudioObjectPropertyScopeGlobal,
		kAudioObjectPropertyElementMain
	};
	AudioObjectAddPropertyListener(kAudioObjectSystemObject, &theAddress0, &on_coreaudio_devices_changed, this);
	return true;
}

AudioOutputStream* DeviceContextCoreAudio::create_audio_output_stream(Session *session, Device *device, void* shared_data) {
	return new AudioOutputStreamCoreAudio(session, device, *reinterpret_cast<AudioOutputStream::SharedData*>(shared_data));
}

AudioInputStream* DeviceContextCoreAudio::create_audio_input_stream(Session *session, Device *device, void* shared_data) {
	return new AudioInputStreamCoreAudio(session, device, *reinterpret_cast<AudioInputStream::SharedData*>(shared_data));
}


void DeviceContextCoreAudio::update_device(DeviceManager* device_manager, bool serious) {
	if (!fully_initialized)
		return;

	for (Device *d: device_manager->output_devices)
		d->present = false;
	for (Device *d: device_manager->input_devices)
		d->present = false;

	auto check_device = [this, device_manager] (AudioDeviceID id, DeviceType type, bool is_default) {

		UInt32 propsize = sizeof(Float32);
		UInt32 mSafetyOffset;
		UInt32 mBufferSizeFrames;
		AudioStreamBasicDescription     mFormat;

		AudioObjectPropertyScope theScope = (type == DeviceType::AudioInput) ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput;
		AudioObjectPropertyAddress theAddress1 = {
			kAudioDevicePropertySafetyOffset,
			theScope,
			0}; // channel

		AudioObjectGetPropertyData(id,
									&theAddress1,
									0,
									nullptr,
									&propsize,
									&mSafetyOffset);


		propsize = sizeof(UInt32);
		theAddress1.mSelector = kAudioDevicePropertyBufferFrameSize;
		AudioObjectGetPropertyData(id,
									&theAddress1,
									0,
									nullptr,
									&propsize,
									&mBufferSizeFrames);

		propsize = sizeof(AudioStreamBasicDescription);
		theAddress1.mSelector = kAudioDevicePropertyStreamFormat;
		AudioObjectGetPropertyData(id,
									&theAddress1,
									0,
									nullptr,
									&propsize,
									&mFormat);

		if (mFormat.mChannelsPerFrame == 0)
			return;

		char temp[1024];
		UInt32 len = sizeof(temp);
		AudioObjectPropertyAddress theAddress2 = { kAudioDevicePropertyDeviceName,
												  theScope,
												  0 }; // channel
		AudioObjectGetPropertyData(id, &theAddress2, 0, nullptr, &len, temp);

		Device dev;
		dev.type = type;
		dev.name = temp;
		dev.internal_name = temp;
		dev.index_in_lib = (int)id;
		dev.channels = (int)mFormat.mChannelsPerFrame;
		dev.present = true;
		dev.default_by_lib = is_default;
		out_device_found(dev);
	};


	UInt32 propsize;
	Array<AudioDeviceID> devids;
	AudioDeviceID default_devid = 0;

// output
	// default
	AudioObjectPropertyAddress theAddress0 = {
			kAudioHardwarePropertyDefaultOutputDevice,
			kAudioDevicePropertyScopeOutput,
			kAudioObjectPropertyElementMain
	};
	propsize = sizeof(AudioDeviceID);
	AudioObjectGetPropertyData(kAudioObjectSystemObject, &theAddress0, 0, nullptr, &propsize, &default_devid);



	// all
	theAddress0 = {
			kAudioHardwarePropertyDevices,
			//kAudioObjectPropertyScopeGlobal,
			kAudioDevicePropertyScopeOutput,
			kAudioObjectPropertyElementMain
	};

	AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &theAddress0, 0, nullptr, &propsize);
	devids.resize(propsize / sizeof(AudioDeviceID));
	AudioObjectGetPropertyData(kAudioObjectSystemObject, &theAddress0, 0, nullptr, &propsize, &devids[0]);

	// update
	check_device(default_devid, DeviceType::AudioOutput, true);
	for (auto id: devids)
		check_device(id, DeviceType::AudioOutput, id == default_devid);

// input
	// default
	theAddress0 = {
		kAudioHardwarePropertyDefaultInputDevice,
		kAudioDevicePropertyScopeInput,
		kAudioObjectPropertyElementMain
	};
	propsize = sizeof(AudioDeviceID);
	AudioObjectGetPropertyData(kAudioObjectSystemObject, &theAddress0, 0, nullptr, &propsize, &default_devid);

	// all
	theAddress0 = {
		kAudioHardwarePropertyDevices,
		kAudioDevicePropertyScopeInput,
		kAudioObjectPropertyElementMain
	};
	AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &theAddress0, 0, nullptr, &propsize);
	devids.resize(propsize / sizeof(AudioDeviceID));
	AudioObjectGetPropertyData(kAudioObjectSystemObject, &theAddress0, 0, nullptr, &propsize, &devids[0]);

	// update
	check_device(default_devid, DeviceType::AudioInput, true);
	for (auto id: devids)
		check_device(id, DeviceType::AudioInput, id == default_devid);

}

}

#endif
