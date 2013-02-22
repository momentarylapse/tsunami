/*
 * AudioInputMidi.cpp
 *
 *  Created on: 19.02.2013
 *      Author: michi
 */

#include "AudioInputMidi.h"
#include "../Tsunami.h"
#include "../Stuff/Log.h"
#include <alsa/asoundlib.h>



AudioInputMidi::AudioInputMidi(MidiData &_data) :
	data(_data)
{
	handle = NULL;
	timer = HuiCreateTimer();

	Init();
}

AudioInputMidi::~AudioInputMidi()
{
	if (handle)
		snd_seq_close(handle);
}

void AudioInputMidi::Init()
{
	int portid;

	if (snd_seq_open(&handle, "hw", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK) < 0) {
		tsunami->log->Error("Error opening ALSA sequencer");
		return;
	}
	snd_seq_set_client_name(handle, "Tsunami");
	if ((portid = snd_seq_create_simple_port(handle, "Tsunami MIDI in",
			SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
			SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
		tsunami->log->Error("Error creating sequencer port");
		return;
	}
}

void AudioInputMidi::ClearInput()
{
	while (true){
		snd_seq_event_t *ev;
		int r = snd_seq_event_input(handle, &ev);
		if (r < 0)
			break;
		snd_seq_free_event(ev);
	}
}

bool AudioInputMidi::Start(int _sample_rate)
{
	if (!handle)
		return false;
	sample_rate = _sample_rate;
	offset = 0;
	data.clear();

	ClearInput();

	HuiGetTime(timer);

	for (int i=0;i<128;i++)
		tone_start[i] = -1;

	capturing = true;
	return true;
}

void AudioInputMidi::Stop()
{
	capturing = false;
}

int AudioInputMidi::DoCapturing()
{
	int pos0 = offset * (double)sample_rate;
	offset += HuiGetTime(timer);
	int pos = offset * (double)sample_rate;

	while (true){
		snd_seq_event_t *ev;
		int r = snd_seq_event_input(handle, &ev);
		if (r < 0)
			break;
		switch (ev->type) {
			case SND_SEQ_EVENT_NOTEON:
				tone_start[ev->data.note.note] = pos;
				tone_volume[ev->data.note.note] = (float)ev->data.note.velocity / 127.0f;
				//msg_write(format("note on %d %d", ev->data.control.channel, ev->data.note.note));
				break;
			case SND_SEQ_EVENT_NOTEOFF:
				//msg_write(format("note off %d %d", ev->data.control.channel, ev->data.note.note));
				if (tone_start[ev->data.note.note] >= 0){
					MidiNote n;
					n.pitch = ev->data.note.note;
					n.volume = tone_volume[ev->data.note.note];
					n.range.offset = tone_start[ev->data.note.note];
					n.range.num = pos - tone_start[ev->data.note.note];
					tone_start[ev->data.note.note] = -1;
					data.add(n);
				}
				break;
		}
		snd_seq_free_event(ev);
	}
	return pos - pos0;
}

bool AudioInputMidi::IsCapturing()
{
	return capturing;
}


float AudioInputMidi::GetSampleRate()
{
	return sample_rate;
}

BufferBox AudioInputMidi::GetSomeSamples(int num_samples)
{
	int pos = offset * (double)sample_rate;
	BufferBox buf;
	buf.resize(num_samples);
	if (data.num > 0)
		if (data.back().range.end() > pos - num_samples){
			buf.r.back() = buf.l.back() = 1;
		}
	return buf;
}


int AudioInputMidi::GetDelay()
{
	return 0;
}

void AudioInputMidi::ResetSync(){}

