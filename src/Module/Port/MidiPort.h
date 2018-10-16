/*
 * MidiPort.h
 *
 *  Created on: 29.04.2016
 *      Author: michi
 */

#ifndef SRC_MODULE_PORT_MIDIPORT_H_
#define SRC_MODULE_PORT_MIDIPORT_H_

#include "Port.h"

class BeatSource;
class MidiEventBuffer;

class MidiPort : public Port
{
public:
	MidiPort(const string &name);
	virtual ~MidiPort(){}

	void _cdecl __init__(const string &name);
	virtual void _cdecl __delete__();

	virtual int _cdecl read(MidiEventBuffer &midi){ return 0; };
	virtual void _cdecl reset(){}
};

#endif /* SRC_MODULE_PORT_MIDIPORT_H_ */
