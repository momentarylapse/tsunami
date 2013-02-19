/*
 * MidiInput.cpp
 *
 *  Created on: 19.02.2013
 *      Author: michi
 */

#include "MidiInput.h"
#include "../Tsunami.h"
#include "../Stuff/Log.h"
#include <alsa/asoundlib.h>


#define UPDATE_TIME		10

MidiInput::MidiInput() :
	PeakMeterSource("MidiInput")
{
	handle = NULL;
}

MidiInput::~MidiInput()
{
}

void MidiInput::Init()
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

bool MidiInput::Start(int _sample_rate)
{
	sample_rate = _sample_rate;
	Init();

	if (handle)
		HuiRunLaterM(UPDATE_TIME, this, &MidiInput::Update);

	return handle;
}

void MidiInput::Stop()
{
	if (!handle)
		return;

	snd_seq_close(handle);
	handle = NULL;
}

int MidiInput::DoCapturing()
{
	while (true){
		snd_seq_event_t *ev;
		int r = snd_seq_event_input(handle, &ev);
		if (r < 0)
			break;
		switch (ev->type) {
			case SND_SEQ_EVENT_NOTEON:
				msg_write(format("note on %d %d", ev->data.control.channel, ev->data.note.note));
				break;
			case SND_SEQ_EVENT_NOTEOFF:
				msg_write(format("note off %d %d", ev->data.control.channel, ev->data.note.note));
				break;
		}
		snd_seq_free_event(ev);
	}
	return 0;
}

void MidiInput::Update()
{
	DoCapturing();
	if (handle)
		HuiRunLaterM(UPDATE_TIME, this, &MidiInput::Update);
}

bool MidiInput::IsCapturing()
{
	return handle;
}


float MidiInput::GetSampleRate()
{
	return sample_rate;
}

BufferBox MidiInput::GetSomeSamples(int num_samples)
{
	BufferBox buf;
	return buf;
}

