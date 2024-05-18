//
// Created by michi on 5/10/24.
//


#if HAS_LIB_PORTAUDIO

#include "AudioOutputStreamPort.h"
#include "../DeviceManager.h"
#include "../Device.h"
#include "../../Session.h"
#include "../../lib/hui/hui.h"
#include <portaudio.h>


AudioOutputStreamPort::AudioOutputStreamPort(Session *session, Device *device, SharedData& shared_data) : AudioOutputStream(session, shared_data) {

	int dev_sample_rate = session->sample_rate();

	int chunk_size = hui::config.get_int("Output.Portaudio.chunk-size", 256);
	//256*4; // paFramesPerBufferUnspecified
	if (device->is_default()) {
		PaError err = Pa_OpenDefaultStream(&portaudio_stream, 0, 2, paFloat32, dev_sample_rate, chunk_size,
										   &portaudio_stream_request_callback, this);
		_portaudio_test_error(err, "Pa_OpenDefaultStream");
	} else {
		PaStreamParameters params;
		params.channelCount = 2;
		params.sampleFormat = paFloat32;
		params.device = device->index_in_lib;
		params.hostApiSpecificStreamInfo = nullptr;
		params.suggestedLatency = 0;
		PaError err = Pa_OpenStream(&portaudio_stream, nullptr, &params, dev_sample_rate, chunk_size,
									paNoFlag, &portaudio_stream_request_callback, this);
		_portaudio_test_error(err, "Pa_OpenStream");
	}
}

AudioOutputStreamPort::~AudioOutputStreamPort() {
	PaError err = Pa_CloseStream(portaudio_stream);
	_portaudio_test_error(err, "Pa_CloseStream");
	portaudio_stream = nullptr;
}


void AudioOutputStreamPort::pause() {
	//Pa_AbortStream Pa_StopStream
	PaError err = Pa_AbortStream(portaudio_stream);
	_portaudio_test_error(err, "Pa_AbortStream");
}

void AudioOutputStreamPort::unpause() {
	PaError err = Pa_StartStream(portaudio_stream);
	_portaudio_test_error(err, "Pa_StartStream");
}

void AudioOutputStreamPort::flush() {
}

base::optional<int64> AudioOutputStreamPort::estimate_samples_played() {
	//	always returning 0???
	//	PaTime t = Pa_GetStreamTime(portaudio_stream);
	//	return (double)t / session->sample_rate() - shared_data.fake_samples_played;
	return base::None;
}



int AudioOutputStreamPort::portaudio_stream_request_callback(const void *inputBuffer, void *outputBuffer,
                                                    unsigned long frames,
                                                    const PaStreamCallbackTimeInfo* timeInfo,
                                                    PaStreamCallbackFlags statusFlags,
                                                    void *userData) {
//	printf("request %d\n", (int)frames);
	auto stream = static_cast<AudioOutputStreamPort*>(userData);

	auto out = static_cast<float*>(outputBuffer);
	(void) inputBuffer; /* Prevent unused variable warning. */

	bool out_of_data = stream->shared_data.feed_stream_output(frames, out);

	if (out_of_data)
		stream->signal_out_of_data();

	return paContinue;
}

bool AudioOutputStreamPort::_portaudio_test_error(PaError err, const char *msg) {
	if (err != paNoError) {
		session->e(format("AudioOutput: %s: %s", msg, Pa_GetErrorText(err)));
		return true;
	}
	return false;
}


#endif
