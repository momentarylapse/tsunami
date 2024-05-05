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

/*class Port : public VirtualBase {
public:
	Port(SignalType type, const string &name);
	virtual ~Port(){}

	void _cdecl __init__(SignalType type, const string &name);
	void _cdecl __delete__() override;

	SignalType type;
	string name;


	virtual int _cdecl read_audio(AudioBuffer &buf){ return 0; }
	virtual int _cdecl read_midi(MidiEventBuffer &midi){ return 0; };
	virtual int _cdecl read_beats(Array<Beat> &beats, int samples){ return 0; };
};*/

struct OutPort {
	OutPort(Module* module, SignalType type, const string& name = "out", int port_no = 0);
	Module *module;
	int port_no;
	SignalType type;
	string name;

	int read_audio(AudioBuffer &buf);
	int read_midi(MidiEventBuffer &midi);
	int read_beats(Array<Beat> &beats, int samples);
};

struct AudioOutPort : OutPort {
	AudioOutPort(Module *module, const string &name = "out", int port_no = 0);
};

struct MidiOutPort : OutPort {
	MidiOutPort(Module *module, const string &name = "out", int port_no = 0);
};

struct BeatsOutPort : OutPort {
	BeatsOutPort(Module *module, const string &name = "out", int port_no = 0);
};


struct InPort {
	InPort(Module* module, SignalType type, const string& name = "in");
	SignalType type;
	string name;

	OutPort *source = nullptr; // out port of source
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
