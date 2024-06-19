//
// Created by michi on 18.05.24.
//

#ifndef TSUNAMI_AUDIOINPUTSTREAMPORT_H
#define TSUNAMI_AUDIOINPUTSTREAMPORT_H


#if HAS_LIB_PORTAUDIO

#include "../interface/AudioInputStream.h"

typedef void PaStream;
struct PaStreamCallbackTimeInfo;
typedef unsigned long PaStreamCallbackFlags;
typedef int PaError;

namespace tsunami {

class AudioInputStreamPort : public AudioInputStream {
public:
	AudioInputStreamPort(Session *session, Device *device, SharedData& shared_data);
	~AudioInputStreamPort() override;

	void pause() override;
	void unpause() override;
	base::optional<int64> estimate_samples_captured() override;


	PaStream *portaudio_stream = nullptr;


	static int portaudio_stream_request_callback(const void *inputBuffer, void *outputBuffer,
	                                             unsigned long frames,
	                                             const PaStreamCallbackTimeInfo* timeInfo,
	                                             PaStreamCallbackFlags statusFlags,
	                                             void *userData);
	bool _portaudio_test_error(PaError err, const char *msg);
};

}

#endif

#endif //TSUNAMI_AUDIOINPUTSTREAMPORT_H
