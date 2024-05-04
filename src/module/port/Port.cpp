/*
 * Port.cpp
 *
 *  Created on: 04.04.2018
 *      Author: michi
 */

#include "Port.h"
#include "../Module.h"
#include "../../data/base.h"

Port::Port(SignalType _type, const string &_name) {
	type = _type;
	name = _name;
}

void Port::__init__(SignalType _type, const string &_name) {
	new(this) Port(_type, _name);
}

void Port::__delete__() {
	this->Port::~Port();
}


/*OutPort::OutPort(Module* module, SignalType type, const string& name) {
	module->port_out.add(new Output(this));
};*/

InPort::InPort(Module* module, SignalType _type, const string& _name) {
	type = _type;
	name = _name;
	module->port_in.add(this);
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
