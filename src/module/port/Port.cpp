/*
 * Port.cpp
 *
 *  Created on: 04.04.2018
 *      Author: michi
 */

#include "Port.h"
#include "../Module.h"
#include "../../data/base.h"
#include "../../Session.h"


OutPort::OutPort(Module* _module, SignalType _type, const string& _name, int _port_no) {
	module = _module;
	type = _type;
	name = _name;
	port_no = _port_no;
	module->port_out.add(this);
}

void OutPort::connect(InPort &in) {
	if (_connection_count > 0) {
		module->session->e("connect: port already connected");
		return;
	}
	if (type != in.type) {
		module->session->e("connect: port type mismatch");
		return;
	}

	in.source = this;
	_connection_count ++;
}

int OutPort::read_audio(AudioBuffer &buf) {
	return module->read_audio(port_no, buf);
}

int OutPort::read_midi(MidiEventBuffer &midi) {
	return module->read_midi(port_no, midi);
}

int OutPort::read_beats(Array<Beat> &beats, int samples) {
	return module->read_beats(port_no, beats, samples);
}

AudioOutPort::AudioOutPort(Module *module, const string &name, int port_no) :
		OutPort(module, SignalType::AUDIO, name, port_no) {
}

MidiOutPort::MidiOutPort(Module *module, const string &name, int port_no) :
		OutPort(module, SignalType::MIDI, name, port_no) {
}

BeatsOutPort::BeatsOutPort(Module *module, const string &name, int port_no) :
		OutPort(module, SignalType::BEATS, name, port_no) {
}

InPort::InPort(Module* module, SignalType _type, const string& _name) {
	type = _type;
	name = _name;
	module->port_in.add(this);
}

void InPort::disconnect() {
	if (source)
		source->_connection_count --;
	source = nullptr;
}

AudioInPort::AudioInPort(Module* module, const string& name) :
		InPort(module, SignalType::AUDIO, name) {
}

MidiInPort::MidiInPort(Module* module, const string& name) :
		InPort(module, SignalType::MIDI, name) {
}

BeatsInPort::BeatsInPort(Module* module, const string& name) :
		InPort(module, SignalType::BEATS, name) {
}
