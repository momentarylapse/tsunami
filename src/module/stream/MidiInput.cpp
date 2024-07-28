/*
 * MidiInput.cpp
 *
 *  Created on: 19.02.2013
 *      Author: michi
 */

#include "MidiInput.h"
#include "../../device/interface/DeviceContext.h"
#include "../../device/Device.h"
#include "../../device/DeviceManager.h"
#include "../../Session.h"
#include "../../data/base.h"
#include "../../lib/kaba/lib/extern.h"
#include "../../lib/os/time.h"
#include "../../lib/hui/language.h"
#include "../../plugins/PluginManager.h"

namespace kaba {
	VirtualTable* get_vtable(const VirtualBase *p);
}

namespace tsunami {

int MidiInput::read_midi(int port, MidiEventBuffer &midi) {
	if (config.free_flow){
		for (auto &e: events){
			e.pos = 0;
			midi.add(e);
		}

		events.clear();
		return midi.samples;
	} else {
		int samples = min(midi.samples, events.samples);
		if (samples < midi.samples)
			return Return::NotEnoughData;
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

void MidiInput::feed(const MidiEventBuffer &midi) {
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

MidiInput::MidiInput(Session *_session) : Module(ModuleCategory::Stream, "MidiInput") {
	session = _session;
	_sample_rate = session->sample_rate();
	state = State::NoDevice;

	device_manager = session->device_manager;
	cur_device = nullptr;

	auto *device_pointer_class = session->plugin_manager->get_class("Device*");
	auto _class = session->plugin_manager->get_class("MidiInputConfig");
	if (_class->elements.num == 0) {
		kaba::add_class(_class);
		kaba::class_add_element("device", device_pointer_class, &Config::device);
		kaba::class_add_element("free_flow", kaba::TypeBool, &Config::free_flow);
		_class->_vtable_location_target_ = kaba::get_vtable(&config);
	}
	config.kaba_class = _class;

	timer = new os::Timer;
	offset = 0;
	pfd = nullptr;
	npfd = 0;
}

MidiInput::~MidiInput() {
	stop();
	unconnect();
	_kill_dev();
	delete timer;
}

void MidiInput::_create_dev() {
	if (state != State::NoDevice)
		return;
	stream = device_manager->midi_context->create_midi_input_stream(session, cur_device, &shared_data);
	if (stream->error)
		return;

	state = State::Paused;
}

void MidiInput::_kill_dev() {
	if (state == State::NoDevice)
		return;
	delete stream;
	stream = nullptr;
	state = State::NoDevice;
}

bool MidiInput::unconnect() {
	if (stream)
		return stream->unconnect();
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
	if (state != State::NoDevice)
		unconnect();

	cur_device = config.device;

	if (state == State::NoDevice)
		_create_dev();

	stream->update_device(cur_device);
}

bool MidiInput::start() {
	if (state == State::NoDevice)
		_create_dev();
	if (state != State::Paused)
		return false;
	session->i(_("capture midi start"));

	if (stream->start())
		return false;

	offset = 0;

	stream->clear_input_queue();

	timer->reset();

	state = State::Capturing;
	return true;
}

void MidiInput::stop() {
	if (state != State::Capturing)
		return;
	session->i(_("capture midi stop"));

	if (stream->stop())
		return;

	//midi.sanify(Range(0, midi.samples));
	state = State::Paused;
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
	offset = offset_new;

	stream->read(current_midi);

	if (current_midi.samples > 0)
		feed(current_midi);

	return current_midi.samples;
}

bool MidiInput::is_capturing() const {
	return state == State::Capturing;
}


int MidiInput::get_delay() {
	return 0;
}

void MidiInput::reset_sync() {
}

base::optional<int64> MidiInput::command(ModuleCommand cmd, int64 param) {
	if (cmd == ModuleCommand::Start){
		start();
		return 0;
	} else if (cmd == ModuleCommand::Stop){
		stop();
		return 0;
	} else if (cmd == ModuleCommand::Suck){
		return (int64)do_capturing();
	}
	return base::None;
}

ModuleConfiguration *MidiInput::get_config() const {
	return (ModuleConfiguration*)&config;
}

void MidiInput::on_config() {
	if (config.device != cur_device)
		update_device();
}

}

