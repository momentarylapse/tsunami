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
#include "../lib/kaba/lib/common.h"
#include "../Plugins/PluginManager.h"

namespace Kaba {
	VirtualTable* get_vtable(const VirtualBase *p);
}

#if HAS_LIB_ALSA
#include <alsa/asoundlib.h>
#endif

MidiInput::Output::Output(MidiInput *_input) : Port(SignalType::MIDI, "out") {
	input = _input;
}

int MidiInput::Output::read_midi(MidiEventBuffer &midi) {
	if (input->config.free_flow){
		for (auto &e: events){
			e.pos = 0;
			midi.add(e);
		}

		events.clear();
		return midi.samples;
	} else {
		int samples = min(midi.samples, events.samples);
		if (samples < midi.samples)
			return NOT_ENOUGH_DATA;
		for (int i=events.num-1; i>=0; i--){
			if (events[i].pos < samples){
				//msg_write("add " + format("%.0f  %f", events[i].pitch, events[i].volume));
				midi.add(events[i]);
				events.erase(i);
			} else
				events[i].pos -= samples;
		}

		events.samples -= samples;
		return samples;
	}
}

void MidiInput::Output::feed(const MidiEventBuffer &midi) {
	events.append(midi);
}

void MidiInput::Config::reset() {
	free_flow = true;
	device = _module->session->device_manager->choose_device(DeviceType::MIDI_INPUT);
}

string MidiInput::Config::auto_conf(const string &name) const {
	if (name == "device")
		return "midi:input";
	return "";
}

MidiInput::MidiInput(Session *_session) : Module(ModuleType::STREAM, "MidiInput") {
	session = _session;
	_sample_rate = session->sample_rate();
	state = State::NO_DEVICE;

#if HAS_LIB_ALSA
	subs = nullptr;
#endif

	device_manager = session->device_manager;
	cur_device = nullptr;

	out = new Output(this);
	port_out.add(out);

	auto *device_pointer_class = session->plugin_manager->get_class("Device*");
	auto _class = session->plugin_manager->get_class("MidiInputConfig");
	if (_class->elements.num == 0) {
		Kaba::add_class(_class);
		Kaba::class_add_elementx("device", device_pointer_class, &Config::device);
		Kaba::class_add_elementx("free_flow", Kaba::TypeBool, &Config::free_flow);
		_class->_vtable_location_target_ = Kaba::get_vtable(&config);
	}
	config._class = _class;

	timer = new hui::Timer;
	offset = 0;
	pfd = nullptr;
	npfd = 0;
	portid = -1;
}

MidiInput::~MidiInput() {
	stop();
	unconnect();
	_kill_dev();
	delete timer;
}

void MidiInput::_create_dev() {
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

void MidiInput::_kill_dev() {
	if (state == State::NO_DEVICE)
		return;
#if HAS_LIB_ALSA
	snd_seq_delete_simple_port(device_manager->alsa_midi_handle, portid);
#endif
	state = State::NO_DEVICE;
}

bool MidiInput::unconnect() {
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

void MidiInput::set_device(Device *d) {
	config.device = d;
	changed();
}

Device *MidiInput::get_device() {
	return cur_device;
}

void MidiInput::update_device() {

	if (state != State::NO_DEVICE)
		unconnect();

	cur_device = config.device;

	if (state == State::NO_DEVICE)
		_create_dev();

	if ((cur_device->client < 0) or (cur_device->port < 0))
		return;// true;

#if HAS_LIB_ALSA
	if (!device_manager->alsa_midi_handle)
		return;

	snd_seq_addr_t sender, dest;
	sender.client = cur_device->client;
	sender.port = cur_device->port;
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

void MidiInput::clear_input_queue() {
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

bool MidiInput::start() {
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

void MidiInput::stop() {
	if (state != State::CAPTURING)
		return;
	session->i(_("capture midi stop"));

	//midi.sanify(Range(0, midi.samples));
	state = State::PAUSED;
}

// TODO: allow multiple streams/ports
//  need to buffer events for other ports
int MidiInput::do_capturing() {
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

bool MidiInput::is_capturing() {
	return state == State::CAPTURING;
}


int MidiInput::get_delay() {
	return 0;
}

void MidiInput::reset_sync() {
}

int MidiInput::command(ModuleCommand cmd, int param) {
	if (cmd == ModuleCommand::START){
		start();
		return 0;
	} else if (cmd == ModuleCommand::STOP){
		stop();
		return 0;
	} else if (cmd == ModuleCommand::SUCK){
		return do_capturing();
	}
	return COMMAND_NOT_HANDLED;
}

ModuleConfiguration *MidiInput::get_config() const {
	return (ModuleConfiguration*)&config;
}

void MidiInput::on_config() {
	if (config.device != cur_device)
		update_device();
}

