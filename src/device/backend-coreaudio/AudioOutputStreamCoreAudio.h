#ifndef TSUNAMI_AUDIOOUTPUTSTREAMCOREAUDIO_H
#define TSUNAMI_AUDIOOUTPUTSTREAMCOREAUDIO_H

#if HAS_LIB_COREAUDIO

#include "../interface/AudioOutputStream.h"

#include <AudioUnit/AudioUnit.h>

namespace tsunami {

class AudioOutputStreamCoreAudio : public AudioOutputStream {
public:
	AudioOutputStreamCoreAudio(Session *session, Device *device, SharedData& shared_data);
	~AudioOutputStreamCoreAudio() override;

	void pause() override;
	void unpause() override;
	void flush() override;
	base::optional<int64> estimate_samples_played() override;


	::AudioComponent output;
	::AudioUnit tone_unit;


	static ::OSStatus request_callback(
		void *inRefCon,
		::AudioUnitRenderActionFlags *ioActionFlags,
		const ::AudioTimeStamp *inTimeStamp,
		::UInt32 inBusNumber,
		::UInt32 inNumberFrames,
		::AudioBufferList *ioData);
};

}

#endif

#endif //TSUNAMI_AUDIOOUTPUTSTREAMCOREAUDIO_H
