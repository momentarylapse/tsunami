/*
 * Port.h
 *
 *  Created on: 04.04.2018
 *      Author: michi
 */

#ifndef SRC_MODULE_PORT_PORT_H_
#define SRC_MODULE_PORT_PORT_H_

#include "../../lib/base/base.h"

enum class SignalType;

class Module;
class AudioBuffer;
class MidiEventBuffer;
class Beat;
struct InPort;


struct OutPort {
	OutPort(Module* module, SignalType type, const string& name = "out");

	void connect(InPort& in);
	void operator>>(InPort& in) { connect(in); }

	int read_audio(AudioBuffer &buf);
	int read_midi(MidiEventBuffer &midi);
	int read_beats(Array<Beat> &beats, int samples);

	Module *module;
	int port_no;
	SignalType type;
	string name;
	int _connection_count = 0;
};

struct AudioOutPort : OutPort {
	AudioOutPort(Module *module, const string &name = "out");
};

struct MidiOutPort : OutPort {
	MidiOutPort(Module *module, const string &name = "out");
};

struct BeatsOutPort : OutPort {
	BeatsOutPort(Module *module, const string &name = "out");
};


struct InPort {
	InPort(Module* module, SignalType type, const string& name = "in");
	void disconnect();

	SignalType type;
	string name;

	OutPort *source = nullptr;
};

struct AudioInPort : InPort {
	AudioInPort(Module* module, const string& name = "in");
};

struct MidiInPort : InPort {
	MidiInPort(Module* module, const string& name = "in");
};

struct BeatsInPort : InPort {
	BeatsInPort(Module* module, const string& name = "in");
};

#endif /* SRC_MODULE_PORT_PORT_H_ */
