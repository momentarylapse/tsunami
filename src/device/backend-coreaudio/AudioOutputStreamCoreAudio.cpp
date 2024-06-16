#if HAS_LIB_COREAUDIO

#include "AudioOutputStreamCoreAudio.h"
#include "../DeviceManager.h"
#include "../Device.h"
#include "../../Session.h"
#include "../../lib/hui/hui.h"

namespace tsunami {

::OSStatus AudioOutputStreamCoreAudio::request_callback(
	void *inRefCon,
	::AudioUnitRenderActionFlags *ioActionFlags,
	const ::AudioTimeStamp *inTimeStamp,
	::UInt32 inBusNumber,
	::UInt32 frames,
	::AudioBufferList *ioData)
{
    static float theta;
	auto stream = reinterpret_cast<AudioOutputStreamCoreAudio*>(inRefCon);

    auto *out = (float *)ioData->mBuffers[0].mData;


	bool out_of_data = stream->shared_data.feed_stream_output(frames, out);

	if (out_of_data)
		stream->signal_out_of_data();

    return ::noErr;
}

AudioOutputStreamCoreAudio::AudioOutputStreamCoreAudio(Session *session, Device *device, SharedData& shared_data) : AudioOutputStream(session, shared_data) {

	int dev_sample_rate = session->sample_rate();

	int chunk_size = hui::config.get_int("Output.Portaudio.chunk-size", 256);
	if (device->is_default()) {
	} else {
	}


	OSErr err;

	AudioComponentDescription acd = {
		.componentType = ::kAudioUnitType_Output,
		.componentSubType = ::kAudioUnitSubType_DefaultOutput,
		.componentManufacturer = ::kAudioUnitManufacturer_Apple,
	};

	output = AudioComponentFindNext(nullptr, &acd);
	if (!output) 
		session->e("CoreAudio: Can't find default output");

	err = AudioComponentInstanceNew(output, &tone_unit);
	if (err) 
		session->e(format("CoreAudio: Error creating unit: %d", (int)err));

	AURenderCallbackStruct input = {
		.inputProc = AudioOutputStreamCoreAudio::request_callback,
		.inputProcRefCon = this
	};
	err = AudioUnitSetProperty(tone_unit, kAudioUnitProperty_SetRenderCallback,
	::kAudioUnitScope_Input, 0, &input, sizeof(input));
	if (err)
		session->e(format("CoreAudio: Error setting callback: %d", (int)err));

	AudioStreamBasicDescription asbd = {
		.mFormatID = kAudioFormatLinearPCM,
		.mFormatFlags = 0
			| kAudioFormatFlagIsFloat
			| kAudioFormatFlagIsPacked,
		//	| kAudioFormatFlagIsNonInterleaved,
		.mSampleRate = (double)dev_sample_rate,
		.mBitsPerChannel = 32,
		.mChannelsPerFrame = 2,
		.mFramesPerPacket = 1,
		.mBytesPerFrame = 8,
		.mBytesPerPacket = 8,
	};

	err = AudioUnitSetProperty(tone_unit, kAudioUnitProperty_StreamFormat,
	kAudioUnitScope_Input, 0, &asbd, sizeof(asbd));
	if (err)
		session->e(format("CoreAudio: Error setting stream format: %d", (int)err));

	err = AudioUnitInitialize(tone_unit);
	if (err)
		session->e(format("CoreAudio: Error initializing unit: %d", (int)err));
}

AudioOutputStreamCoreAudio::~AudioOutputStreamCoreAudio() {
	AudioUnitUninitialize(tone_unit);
	AudioComponentInstanceDispose(tone_unit);
}


void AudioOutputStreamCoreAudio::pause() {
	AudioOutputUnitStop(tone_unit);
}

void AudioOutputStreamCoreAudio::unpause() {
	OSErr err = AudioOutputUnitStart(tone_unit);
	if (err)
		session->e(format("CoreAudio: Error starting unit: %d", (int)err));
}

void AudioOutputStreamCoreAudio::flush() {
}

base::optional<int64> AudioOutputStreamCoreAudio::estimate_samples_played() {
	return base::None;
}

}

#endif
