#ifndef AUDIOINPUTSTREAMCOREAUDIO_H
#define AUDIOINPUTSTREAMCOREAUDIO_H

#if HAS_LIB_COREAUDIO

#include "../interface/AudioInputStream.h"

#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>

namespace tsunami {

class AudioInputStreamCoreAudio : public AudioInputStream {
public:
	AudioInputStreamCoreAudio(Session *session, Device *device, SharedData& shared_data);
	~AudioInputStreamCoreAudio() override;

	::AudioComponent output;
	::AudioUnit unit;
	::AudioDeviceID device_id;
	::AudioBufferList* audio_buffer_list;

	void pause() override;
	void unpause() override;
	base::optional<int64> estimate_samples_captured() override;
};

}

#endif

#endif //AUDIOINPUTSTREAMCOREAUDIO_H
