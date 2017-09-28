/*
 * InputStreamMidi.cpp
 *
 *  Created on: 19.02.2013
 *      Author: michi
 */

#include "../Tsunami.h"
#include "../Stuff/Log.h"
#include "Device.h"
#include "DeviceManager.h"
#include "InputStreamMidi.h"

#include "OutputStream.h"
#include "../Audio/Synth/Synthesizer.h"
#include "../Midi/MidiSource.h"

#ifdef DEVICE_MIDI_ALSA
#include <alsa/asoundlib.h>
#endif

const string InputStreamMidi::MESSAGE_CAPTURE = "Capture";
static const int DEFAULT_CHUNK_SIZE = 512;
static const float DEFAULT_UPDATE_TIME = 0.005f;

InputStreamMidi::Output::Output(InputStreamMidi *_input)
{
	input = _input;
	real_time_mode = true;
}

InputStreamMidi::Output::~Output(){}

int InputStreamMidi::Output::read(MidiRawData &midi)
{
	if (real_time_mode){
		for (auto &e: events){
			e.pos = 0;
			midi.add(e);
		}

		events.clear();
		return midi.samples;
	}else{
		int samples = min(midi.samples, events.samples);
		for (int i=events.num-1; i>=0; i--){
			if (events[i].pos < samples){
				//msg_write("add " + format("%.0f  %f", events[i].pitch, events[i].volume));
				midi.add(events[i]);
				events.erase(i);
			}else
				events[i].pos -= samples;
		}

		events.samples -= samples;
		return samples;
	}
}

void InputStreamMidi::Output::feed(const MidiRawData &midi)
{
	events.append(midi);
}

InputStreamMidi::InputStreamMidi(int _sample_rate)
{
	sample_rate = _sample_rate;
	chunk_size = -1;
	update_dt = -1;
	backup_mode = BACKUP_MODE_NONE;
	update_dt = DEFAULT_UPDATE_TIME;
	chunk_size = DEFAULT_CHUNK_SIZE;

#ifdef DEVICE_MIDI_ALSA
	subs = NULL;
#endif

	chunk_size = 512;

	running = false;

	device_manager = tsunami->device_manager;

	out = new Output(this);

	timer = new hui::Timer;

	init();
}

InputStreamMidi::~InputStreamMidi()
{
	stop();
	unconnect();
	delete out;
	delete timer;
}

void InputStreamMidi::init()
{
	setDevice(device_manager->chooseDevice(Device::TYPE_MIDI_INPUT));
}

bool InputStreamMidi::unconnect()
{
#ifdef DEVICE_MIDI_ALSA
	if (!subs)
		return true;
	int r = snd_seq_unsubscribe_port(device_manager->handle, subs);
	if (r != 0)
		tsunami->log->error(string("Error unconnecting from midi port: ") + snd_strerror(r));
	snd_seq_port_subscribe_free(subs);
	subs = NULL;
	return r == 0;
#endif
	return true;
}

void InputStreamMidi::setDevice(Device *d)
{
	device = d;

	unconnect();

	if ((device->client < 0) or (device->port < 0))
		return;// true;

#ifdef DEVICE_MIDI_ALSA
	if (!device_manager->handle)
		return;

	snd_seq_addr_t sender, dest;
	sender.client = device->client;
	sender.port = device->port;
	dest.client = snd_seq_client_id(device_manager->handle);
	dest.port = device_manager->portid;

	snd_seq_port_subscribe_malloc(&subs);
	snd_seq_port_subscribe_set_sender(subs, &sender);
	snd_seq_port_subscribe_set_dest(subs, &dest);
	int r = snd_seq_subscribe_port(device_manager->handle, subs);
	if (r != 0){
		tsunami->log->error(string("Error connecting to midi port: ") + snd_strerror(r));
		snd_seq_port_subscribe_free(subs);
		subs = NULL;
	}
	return;// r == 0;

	// simple version raises "no permission" error...?!?
	/*int r = snd_seq_connect_to(handle, portid, p.client, p.port);
	if (r != 0)
		tsunami->log->Error(string("Error connecting to midi port: ") + snd_strerror(r));
	return r == 0;*/
#endif
}

Device *InputStreamMidi::getDevice()
{
	return device;
}

void InputStreamMidi::accumulate(bool enable)
{
	accumulating = enable;
}

void InputStreamMidi::resetAccumulation()
{
	midi.clear();
	midi.samples = 0;
	offset = 0;
}

int InputStreamMidi::getSampleCount()
{
	return midi.samples;
}

void InputStreamMidi::clearInputQueue()
{
#ifdef DEVICE_MIDI_ALSA
	while (true){
		snd_seq_event_t *ev;
		int r = snd_seq_event_input(device_manager->handle, &ev);
		if (r < 0)
			break;
		snd_seq_free_event(ev);
	}
#endif
}

bool InputStreamMidi::start()
{
#ifdef DEVICE_MIDI_ALSA
	if (!device_manager->handle)
		return false;
#endif

	accumulating = false;
	offset = 0;
	resetAccumulation();

	clearInputQueue();

	timer->reset();

	_startUpdate();
	capturing = true;
	return true;
}

void InputStreamMidi::stop()
{
	_stopUpdate();
	capturing = false;

	midi.sanify(Range(0, midi.samples));
}

int InputStreamMidi::doCapturing()
{
	double dt = timer->get();
	double offset_new = offset + dt;
	int pos = offset * (double)sample_rate;
	int pos_new = offset_new * (double)sample_rate;
	current_midi.clear();
	current_midi.samples = pos_new - pos;
	//if (accumulating)
		offset = offset_new;

#ifdef DEVICE_MIDI_ALSA
	while (true){
		snd_seq_event_t *ev;
		int r = snd_seq_event_input(device_manager->handle, &ev);
		if (r < 0)
			break;
		int pitch = ev->data.note.note;
		switch (ev->type) {
			case SND_SEQ_EVENT_NOTEON:
				current_midi.add(MidiEvent(0, pitch, (float)ev->data.note.velocity / 127.0f));
				break;
			case SND_SEQ_EVENT_NOTEOFF:
				current_midi.add(MidiEvent(0, pitch, 0));
				break;
		}
		snd_seq_free_event(ev);
	}
#endif

	if (current_midi.samples > 0)
		out->feed(current_midi);

	if (accumulating)
		midi.append(current_midi);

	return current_midi.samples;
}

bool InputStreamMidi::isCapturing()
{
	return capturing;
}

int InputStreamMidi::getState()
{
	if (isCapturing())
		return STATE_PLAYING;
	return STATE_STOPPED;
}

void InputStreamMidi::getSomeSamples(AudioBuffer &buf, int num_samples)
{
}


int InputStreamMidi::getDelay()
{
	return 0;
}

void InputStreamMidi::resetSync()
{
}

void InputStreamMidi::_startUpdate()
{
	if (running)
		return;
	hui_runner_id = hui::RunRepeated(update_dt, std::bind(&InputStreamMidi::update, this));
	running = true;
}

void InputStreamMidi::_stopUpdate()
{
	if (!running)
		return;
	hui::CancelRunner(hui_runner_id);
	hui_runner_id = -1;
	running = false;
}

void InputStreamMidi::update()
{
	if (doCapturing() > 0)
	{}//	notify(MESSAGE_CAPTURE);

	running = isCapturing();
}

void InputStreamMidi::setBackupMode(int mode)
{
	backup_mode = mode;
}

void InputStreamMidi::setChunkSize(int size)
{
	if (size > 0)
		chunk_size = size;
	else
		chunk_size = DEFAULT_CHUNK_SIZE;
}

void InputStreamMidi::setUpdateDt(float dt)
{
	if (dt > 0)
		update_dt = dt;
	else
		update_dt = DEFAULT_UPDATE_TIME;
}

