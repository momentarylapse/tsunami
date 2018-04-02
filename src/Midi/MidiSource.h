/*
 * MidiSource.h
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#ifndef SRC_MIDI_MIDISOURCE_H_
#define SRC_MIDI_MIDISOURCE_H_

#include "../Plugins/Configurable.h"
#include "MidiPort.h"

class BeatSource;

class MidiSource : public Configurable
{
public:
	MidiSource();
	virtual ~MidiSource();

	void _cdecl __init__();
	virtual void _cdecl __delete__();

	class Output : public MidiPort
	{
	public:
		Output(MidiSource *s);
		virtual ~Output(){}
		virtual int _cdecl read(MidiEventBuffer &midi);
		virtual void _cdecl reset();
		MidiSource *source;
	};
	Output *out;

	virtual int _cdecl read(MidiEventBuffer &midi){ return 0; };
	virtual void _cdecl reset(){}

	BeatSource *beat_source;
	void _cdecl set_beat_source(BeatSource *s);
};

MidiSource *_cdecl CreateMidiSource(Session *session, const string &name);

#endif /* SRC_MIDI_MIDISOURCE_H_ */
