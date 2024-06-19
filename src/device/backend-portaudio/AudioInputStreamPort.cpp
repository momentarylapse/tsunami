//
// Created by michi on 18.05.24.
//

#if HAS_LIB_PORTAUDIO

#include "AudioInputStreamPort.h"
#include "../DeviceManager.h"
#include "../Device.h"
#include "../../Session.h"
#include "../../lib/hui/hui.h"
#include <portaudio.h>

namespace tsunami {

AudioInputStreamPort::AudioInputStreamPort(Session *session, Device *device, AudioInputStream::SharedData &shared_data) : AudioInputStream(session, shared_data) {

	int _sample_rate = session->sample_rate();
	int num_channels = shared_data.num_channels;

	int chunk_size = hui::config.get_int("portaudio.chunk-size", 256);
	// on windows, some devices will stutter, due to some mysterious limit of 100 requests/s
	// nah, that doesn't work either:
	/*paFramesPerBufferUnspecified*/

	if (device->is_default()) {
		session->i(format("open def stream %d  %d", _sample_rate, num_channels));
		PaError err = Pa_OpenDefaultStream(&portaudio_stream, num_channels, 0, paFloat32, _sample_rate, chunk_size,
		                                   &portaudio_stream_request_callback, this);
		_portaudio_test_error(err, "Pa_OpenDefaultStream");
	} else {
		session->i(format("open stream %d  %d", _sample_rate, num_channels));
		PaStreamParameters params;
		params.channelCount = num_channels;
		params.sampleFormat = paFloat32;
		params.device = device->index_in_lib;
		params.hostApiSpecificStreamInfo = nullptr;
		params.suggestedLatency = 0;
		PaError err = Pa_OpenStream(&portaudio_stream, &params, nullptr, _sample_rate, chunk_size,
		                            paNoFlag, &portaudio_stream_request_callback, this);
		_portaudio_test_error(err, "Pa_OpenStream");
	}
}

AudioInputStreamPort::~AudioInputStreamPort() {
	if (portaudio_stream) {
		PaError err = Pa_CloseStream(portaudio_stream);
		_portaudio_test_error(err, "Pa_CloseStream");
		portaudio_stream = nullptr;
	}
}

void AudioInputStreamPort::pause() {
	if (portaudio_stream) {
		// often crashes here... might be a bug in the libraries...?!?
		PaError err = Pa_StopStream(portaudio_stream);
		_portaudio_test_error(err, "Pa_StopStream");
	}
}

void AudioInputStreamPort::unpause() {
	if (portaudio_stream) {
		PaError err = Pa_StartStream(portaudio_stream);
		_portaudio_test_error(err, "Pa_StartStream");
	}
}

base::optional<int64> AudioInputStreamPort::estimate_samples_captured() {
	return base::None;
}

int AudioInputStreamPort::portaudio_stream_request_callback(const void *inputBuffer, void *outputBuffer,
                                                  unsigned long frames,
                                                  const PaStreamCallbackTimeInfo* timeInfo,
                                                  PaStreamCallbackFlags statusFlags,
                                                  void *userData) {
	//printf("request %d\n", (int)frames);
	auto stream = static_cast<AudioInputStreamPort*>(userData);

	(void)outputBuffer; /* Prevent unused variable warning. */
	auto in = static_cast<const float*>(inputBuffer);


	if (in) {
		//if (input->is_capturing()) {

			RingBuffer &buf = stream->shared_data.buffer;
			AudioBuffer b;
			buf.write_ref(b, frames);
			b.deinterleave(in, stream->shared_data.num_channels);
			buf.write_ref_done(b);

			int done = b.length;
			if (done < (int)frames) {
				buf.write_ref(b, frames - done);
				b.deinterleave(&in[stream->shared_data.num_channels * done], stream->shared_data.num_channels);
				buf.write_ref_done(b);
			}
		//}
	} else {
		hui::run_later(0.001f, [=] { stream->session->w("stream callback error"); });
	}
	return 0;
}
bool AudioInputStreamPort::_portaudio_test_error(PaError err, const char *msg) {
	if (err != paNoError) {
		session->e(format("%s: (input): %s", msg, Pa_GetErrorText(err)));
		return true;
	}
	return false;
}

}

#endif