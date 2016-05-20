/*
 * InputStreamMidi.cpp
 *
 *  Created on: 19.02.2013
 *      Author: michi
 */

#include "../Tsunami.h"
#include "../Stuff/Log.h"
#include <alsa/asoundlib.h>
#include "Device.h"
#include "DeviceManager.h"
#include "InputStreamMidi.h"
#include "OutputStream.h"
#include "../Audio/Renderer/MidiRenderer.h"
#include "../Audio/Synth/Synthesizer.h"
#include "../Midi/MidiSource.h"


static const float DEFAULT_UPDATE_TIME = 0.005f;
const string InputStreamMidi::MESSAGE_CAPTURE = "Capture";

class MidiPreviewFeedSource : public MidiSource
{
public:
	virtual int read(MidiRawData &midi)
	{
		msg_write("mpfs.read");
		for (int i=events.num-1; i>=0; i--){
			if (events[i].pos < midi.samples){
				msg_write("add " + format("%d  %f", events[i].pitch, events[i].volume));
				midi.add(events[i]);
				events.erase(i);
			}else
				events[i].pos -= midi.samples;
		}

		events.samples = 0;
		return midi.samples;
	}

	void feed(const MidiRawData &midi)
	{
		events.append(midi);
		msg_write("feed " + i2s(midi.num));
	}

	MidiRawData events;
};


InputStreamMidi::InputStreamMidi(int _sample_rate) :
	PeakMeterSource("InputStreamMidi")
{
	subs = NULL;
	sample_rate = _sample_rate;
	update_dt = DEFAULT_UPDATE_TIME;
	chunk_size = 512;

	running = false;

	device_manager = tsunami->device_manager;

	init();


	preview_source = new MidiPreviewFeedSource;
	msg_error(p2s(preview_source));
	preview_renderer = new MidiRenderer(NULL, preview_source);
	preview_stream = new OutputStream(preview_renderer);
	preview_stream->setBufferSize(2048);
}

InputStreamMidi::~InputStreamMidi()
{
	stop();
	if (subs)
		unconnect();
	delete(preview_stream);
	delete(preview_renderer);
	delete(preview_source);
}

void InputStreamMidi::init()
{
	setDevice(device_manager->chooseDevice(Device::TYPE_MIDI_INPUT));
}

void InputStreamMidi::setPreviewSynthesizer(Synthesizer *s)
{
	preview_renderer->setSynthesizer(s);
	/*preview_renderer->setAutoStop(false);
	if (s and capturing)
		preview_stream->play();*/
}

bool InputStreamMidi::unconnect()
{
	if (!subs)
		return true;
	int r = snd_seq_unsubscribe_port(device_manager->handle, subs);
	if (r != 0)
		tsunami->log->error(string("Error unconnecting from midi port: ") + snd_strerror(r));
	snd_seq_port_subscribe_free(subs);
	subs = NULL;
	return r == 0;
}

void InputStreamMidi::setDevice(Device *d)
{
	device = d;


	if (subs)
		unconnect();

	if ((device->client < 0) or (device->port < 0))
		return;// true;

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
	return offset * (double)sample_rate;
}

void InputStreamMidi::clearInputQueue()
{
	while (true){
		snd_seq_event_t *ev;
		int r = snd_seq_event_input(device_manager->handle, &ev);
		if (r < 0)
			break;
		snd_seq_free_event(ev);
	}
}

bool InputStreamMidi::start()
{
	if (!device_manager->handle)
		return false;
	accumulating = false;
	offset = 0;
	resetAccumulation();

	clearInputQueue();
	/*preview_renderer->setAutoStop(false);
	if (preview_renderer->getSynthesizer())
		preview_stream->play();*/

	timer.reset();

	_startUpdate();
	capturing = true;
	return true;
}

void InputStreamMidi::stop()
{
	_stopUpdate();
	capturing = false;
	//preview_renderer->setAutoStop(true);
//	preview_renderer->endAllNotes();
	//preview_stream->stop();

	midi.sanify(Range(0, midi.samples));
}

int InputStreamMidi::doCapturing()
{
	double dt = timer.get();
	double offset_new = offset + dt;
	int pos = offset * (double)sample_rate;
	int pos_new = offset_new * (double)sample_rate;
	current_midi.clear();
	current_midi.samples = pos_new - pos;
	if (accumulating)
		offset = offset_new;

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

	if (current_midi.num > 0)
		preview_source->feed(current_midi);
	if ((current_midi.num > 0) and (!preview_stream->isPlaying()))
		preview_stream->play();

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

void InputStreamMidi::getSomeSamples(BufferBox &buf, int num_samples)
{
	preview_stream->getSomeSamples(buf, num_samples);
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
	hui_runner_id = HuiRunRepeatedM(update_dt, this, &InputStreamMidi::update);
	running = true;
}

void InputStreamMidi::_stopUpdate()
{
	if (!running)
		return;
	HuiCancelRunner(hui_runner_id);
	hui_runner_id = -1;
	running = false;
}

void InputStreamMidi::update()
{
	if (doCapturing() > 0)
		notify(MESSAGE_CAPTURE);

	running = isCapturing();
}

float InputStreamMidi::getSampleRate()
{
	return sample_rate;
}

void InputStreamMidi::setUpdateDt(float dt)
{
	if (dt > 0)
		update_dt = dt;
	else
		update_dt = DEFAULT_UPDATE_TIME;
}

void InputStreamMidi::setChunkSize(int size)
{
	if (size > 0)
		chunk_size = size;
	else
		chunk_size = 512;
}
