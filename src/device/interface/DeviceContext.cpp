//
// Created by michi on 18.05.24.
//

#include "DeviceContext.h"
#include "MidiInputStream.h"
#include "AudioInputStream.h"
#include "AudioOutputStream.h"

namespace tsunami {

DeviceContext::DeviceContext(Session *_session) {
	//session = _session;
}

class AudioOutputStreamDummy : public AudioOutputStream {
public:
	AudioOutputStreamDummy(Session* session, SharedData& shared_data) : AudioOutputStream(session, shared_data) {}
	void pause() override {}
	void unpause() override {}
	void pre_buffer() override {}
	void flush() override {}
	base::optional<int64> estimate_samples_played() override { return base::None; }
};

AudioOutputStream* DeviceContext::create_audio_output_stream(Session *session, Device *device, void* shared_data) {
	return new AudioOutputStreamDummy(session, *reinterpret_cast<AudioOutputStream::SharedData*>(shared_data));
}

class AudioInputStreamDummy : public AudioInputStream {
public:
	AudioInputStreamDummy(Session* session, SharedData& shared_data) : AudioInputStream(session, shared_data) {}
	void pause() override {}
	void unpause() override {}
	base::optional<int64> estimate_samples_captured() override { return base::None; }
};

AudioInputStream* DeviceContext::create_audio_input_stream(Session *session, Device *device, void* shared_data) {
	return new AudioInputStreamDummy(session, *reinterpret_cast<AudioInputStream::SharedData*>(shared_data));
}

class MidiInputStreamDummy : public MidiInputStream {
public:
	MidiInputStreamDummy(Session* session, SharedData& shared_data) : MidiInputStream(session, shared_data) {}
	bool start() override { return false; }
	bool stop() override { return false; }
	void clear_input_queue() override {}
	void read(MidiEventBuffer& buffer) override {}
};

MidiInputStream* DeviceContext::create_midi_input_stream(Session *session, Device *device, void* shared_data) {
	return new MidiInputStreamDummy(session, *reinterpret_cast<MidiInputStream::SharedData*>(shared_data));
}

}