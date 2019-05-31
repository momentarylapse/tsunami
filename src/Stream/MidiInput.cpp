/*
 * MidiInput.cpp
 *
 *  Created on: 19.02.2013
 *      Author: michi
 */

#include "MidiInput.h"

#include "../Session.h"
#include "../Module/Port/Port.h"
#include "../Data/base.h"
#include "../Device/Device.h"
#include "../Device/DeviceManager.h"

#if HAS_LIB_ALSA
#include <alsa/asoundlib.h>
#endif

MidiInput::Output::Output(MidiInput *_input) : Port(SignalType::MIDI, "out")
{
	input = _input;
	real_time_mode = true;
}

int MidiInput::Output::read_midi(MidiEventBuffer &midi)
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
		if (samples < midi.samples)
			return NOT_ENOUGH_DATA;
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

void MidiInput::Output::feed(const MidiEventBuffer &midi)
{
	events.append(midi);
}

MidiInput::MidiInput(Session *_session) :
	Module(ModuleType::STREAM, "MidiInput")
{
	set_session_etc(_session, "MidiInput");
	_sample_rate = session->sample_rate();
	state = State::NO_DEVICE;

#if HAS_LIB_ALSA
	subs = nullptr;
#endif

	device_manager = session->device_manager;

	out = new Output(this);
	port_out.add(out);

	timer = new hui::Timer;

	init();
}

MidiInput::~MidiInput()
{
	stop();
	unconnect();
	_kill_dev();
	delete timer;
}

void MidiInput::init()
{
	set_device(device_manager->choose_device(DeviceType::MIDI_INPUT));
}

void MidiInput::_create_dev()
{
	if (state != State::NO_DEVICE)
		return;
#if HAS_LIB_ALSA
	portid = snd_seq_create_simple_port(device_manager->alsa_midi_handle, "Tsunami MIDI in",
				SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
				SND_SEQ_PORT_TYPE_APPLICATION);
	if (portid < 0){
		session->e(string("Error creating sequencer port: ") + snd_strerror(portid));
		return;
	}
#endif

	state = State::PAUSED;
}

void MidiInput::_kill_dev()
{
	if (state == State::NO_DEVICE)
		return;
#if HAS_LIB_ALSA
	snd_seq_delete_simple_port(device_manager->alsa_midi_handle, portid);
#endif
	state = State::NO_DEVICE;
}

bool MidiInput::unconnect()
{
#if HAS_LIB_ALSA
	if (!subs)
		return true;
	int r = snd_seq_unsubscribe_port(device_manager->alsa_midi_handle, subs);
	if (r != 0)
		session->e(_("Error unconnecting from midi port: ") + snd_strerror(r));
	snd_seq_port_subscribe_free(subs);
	subs = nullptr;
	return r == 0;
#endif
	return true;
}

void MidiInput::set_device(Device *d)
{
	if (state == State::NO_DEVICE)
		_create_dev();

	unconnect();

	device = d;

	if ((device->client < 0) or (device->port < 0))
		return;// true;

#if HAS_LIB_ALSA
	if (!device_manager->alsa_midi_handle)
		return;

	snd_seq_addr_t sender, dest;
	sender.client = device->client;
	sender.port = device->port;
	dest.client = snd_seq_client_id(device_manager->alsa_midi_handle);
	dest.port = portid;

	snd_seq_port_subscribe_malloc(&subs);
	snd_seq_port_subscribe_set_sender(subs, &sender);
	snd_seq_port_subscribe_set_dest(subs, &dest);
	int r = snd_seq_subscribe_port(device_manager->alsa_midi_handle, subs);
	if (r != 0){
		session->e(string("Error connecting to midi port: ") + snd_strerror(r));
		snd_seq_port_subscribe_free(subs);
		subs = nullptr;
	}
	return;// r == 0;

	// simple version raises "no permission" error...?!?
	/*int r = snd_seq_connect_to(handle, portid, p.client, p.port);
	if (r != 0)
		tsunami->log->Error(string("Error connecting to midi port: ") + snd_strerror(r));
	return r == 0;*/
#endif
}

Device *MidiInput::get_device()
{
	return device;
}

void MidiInput::clear_input_queue()
{
#if HAS_LIB_ALSA
	while (true){
		snd_seq_event_t *ev;
		int r = snd_seq_event_input(device_manager->alsa_midi_handle, &ev);
		if (r < 0)
			break;
		snd_seq_free_event(ev);
	}
#endif
}

bool MidiInput::start()
{
	if (state == State::NO_DEVICE)
		_create_dev();
	if (state != State::PAUSED)
		return false;
	session->i(_("capture midi start"));
#if HAS_LIB_ALSA
	if (!device_manager->alsa_midi_handle){
		session->e(_("no alsa midi handler"));
		return false;
	}
#endif

	offset = 0;

	clear_input_queue();

	timer->reset();

	state = State::CAPTURING;
	return true;
}

void MidiInput::stop()
{
	if (state != State::CAPTURING)
		return;
	session->i(_("capture midi stop"));

	//midi.sanify(Range(0, midi.samples));
	state = State::PAUSED;
}

// TODO: allow multiple streams/ports
//  need to buffer events for other ports
int MidiInput::do_capturing()
{
	double dt = timer->get();
	double offset_new = offset + dt;
	int pos = offset * (double)_sample_rate;
	int pos_new = offset_new * (double)_sample_rate;
	current_midi.clear();
	current_midi.samples = pos_new - pos;
	//if (accumulating)
		offset = offset_new;

#if HAS_LIB_ALSA
	while (true){
		snd_seq_event_t *ev;
		int r = snd_seq_event_input(device_manager->alsa_midi_handle, &ev);
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

	return current_midi.samples;
}

bool MidiInput::is_capturing()
{
	return state == State::CAPTURING;
}


int MidiInput::get_delay()
{
	return 0;
}

void MidiInput::reset_sync()
{
}

int MidiInput::command(ModuleCommand cmd, int param)
{
	if (cmd == ModuleCommand::START){
		start();
		return 0;
	}else if (cmd == ModuleCommand::STOP){
		stop();
		return 0;
	}else if (cmd == ModuleCommand::SUCK){
		return do_capturing();
	}
	return COMMAND_NOT_HANDLED;
}

