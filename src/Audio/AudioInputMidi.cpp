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
	subs = NULL;

	Init();
}

AudioInputMidi::~AudioInputMidi()
{
	if (subs)
		Unconnect();
	if (handle)
		snd_seq_close(handle);
}

void AudioInputMidi::Init()
{
	int r = snd_seq_open(&handle, "hw", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK);
	if (r < 0){
		tsunami->log->Error(string("Error opening ALSA sequencer: ") + snd_strerror(r));
		return;
	}
	snd_seq_set_client_name(handle, "Tsunami");
	portid = snd_seq_create_simple_port(handle, "Tsunami MIDI in",
				SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
				SND_SEQ_PORT_TYPE_APPLICATION);
	if (portid < 0){
		tsunami->log->Error(string("Error creating sequencer port: ") + snd_strerror(portid));
		return;
	}
}

bool AudioInputMidi::ConnectTo(AudioInputMidi::MidiPort p)
{
	if (subs)
		Unconnect();
	snd_seq_addr_t sender, dest;
	sender.client = p.client;
	sender.port = p.port;
	dest.client = snd_seq_client_id(handle);
	dest.port = portid;

	snd_seq_port_subscribe_malloc(&subs);
	snd_seq_port_subscribe_set_sender(subs, &sender);
	snd_seq_port_subscribe_set_dest(subs, &dest);
	int r = snd_seq_subscribe_port(handle, subs);
	if (r != 0)
		tsunami->log->Error(string("Error connecting to midi port: ") + snd_strerror(r));
	return r == 0;

	// simple version raises "no permission" error...?!?
	/*int r = snd_seq_connect_to(handle, portid, p.client, p.port);
	if (r != 0)
		tsunami->log->Error(string("Error connecting to midi port: ") + snd_strerror(r));
	return r == 0;*/
}

bool AudioInputMidi::Unconnect()
{
	if (!subs)
		return true;
	int r = snd_seq_unsubscribe_port(handle, subs);
	if (r != 0)
		tsunami->log->Error(string("Error unconnecting to midi port: ") + snd_strerror(r));
	snd_seq_port_subscribe_free(subs);
	subs = NULL;
	return r == 0;
}


Array<AudioInputMidi::MidiPort> AudioInputMidi::FindPorts()
{
	Array<MidiPort> ports;
	snd_seq_client_info_t *cinfo;
	snd_seq_port_info_t *pinfo;

	snd_seq_client_info_alloca(&cinfo);
	snd_seq_port_info_alloca(&pinfo);
	snd_seq_client_info_set_client(cinfo, -1);
	while (snd_seq_query_next_client(handle, cinfo) >= 0){
		snd_seq_port_info_set_client(pinfo, snd_seq_client_info_get_client(cinfo));
		snd_seq_port_info_set_port(pinfo, -1);
		while (snd_seq_query_next_port(handle, pinfo) >= 0){
			if ((snd_seq_port_info_get_capability(pinfo) & SND_SEQ_PORT_CAP_READ) == 0)
				continue;
			if ((snd_seq_port_info_get_capability(pinfo) & SND_SEQ_PORT_CAP_SUBS_READ) == 0)
				continue;
			MidiPort p;
			p.client = snd_seq_client_info_get_client(cinfo);
			p.client_name = snd_seq_client_info_get_name(cinfo);
			p.port = snd_seq_port_info_get_port(pinfo);
			p.port_name = snd_seq_port_info_get_name(pinfo);
			ports.add(p);
		}
	}
	return ports;
}

void AudioInputMidi::Accumulate(bool enable)
{
	accumulating = enable;
}

void AudioInputMidi::ResetAccumulation()
{
	data.clear();
	offset = 0;

	for (int i=0;i<128;i++)
		tone_start[i] = -1;
}

int AudioInputMidi::GetSampleCount()
{
	return offset * (double)sample_rate;
}

void AudioInputMidi::ClearInputQueue()
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
	accumulating = false;
	ResetAccumulation();

	ClearInputQueue();

	timer.reset();

	capturing = true;
	return true;
}

void AudioInputMidi::Stop()
{
	capturing = false;
}

int AudioInputMidi::DoCapturing()
{
	double dt = timer.get();
	if (accumulating)
		offset += dt;
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
					if (accumulating)
						data.add(n);
				}
				break;
		}
		snd_seq_free_event(ev);
	}
	return dt * (double)sample_rate;
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
	//int pos = offset * (double)sample_rate;
	BufferBox buf;
	buf.resize(num_samples);
	for (int i=0;i<128;i++)
		if (tone_start[i] >= 0){
			buf.r.back() = buf.l.back() = 0.9f;
			break;
		}
	/*if (data.num > 0)
		if (data.back().range.end() > pos - num_samples){
			buf.r.back() = buf.l.back() = 1;
		}*/
	return buf;
}


int AudioInputMidi::GetDelay()
{
	return 0;
}

void AudioInputMidi::ResetSync(){}

