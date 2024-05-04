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

class Port : public VirtualBase {
public:
	Port(SignalType type, const string &name);
	virtual ~Port(){}

	void _cdecl __init__(SignalType type, const string &name);
	void _cdecl __delete__() override;

	SignalType type;
	string name;

	static const int END_OF_STREAM;
	static const int NOT_ENOUGH_DATA;
	static const int NO_SOURCE;


	virtual int _cdecl read_audio(AudioBuffer &buf){ return 0; }
	virtual int _cdecl read_midi(MidiEventBuffer &midi){ return 0; };
	virtual int _cdecl read_beats(Array<Beat> &beats, int samples){ return 0; };
};

/*struct OutPort {
	OutPort(Module* module, SignalType type, const string& name);
};*/

struct InPort {
	InPort(Module* module, SignalType type, const string& name);
	SignalType type;
	string name;

	Port *source = nullptr; // out port of source
};

struct AudioInPort : InPort {
	AudioInPort(Module* module, const string& name);
};

struct MidiInPort : InPort {
	MidiInPort(Module* module, const string& name);
};

struct BeatsInPort : InPort {
	BeatsInPort(Module* module, const string& name);
};

#endif /* SRC_MODULE_PORT_PORT_H_ */
