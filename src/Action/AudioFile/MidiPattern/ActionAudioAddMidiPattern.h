/*
 * ActionAudioAddMidiPattern.h
 *
 *  Created on: 14.08.2013
 *      Author: michi
 */

#ifndef ACTIONAUDIOADDMIDIPATTERN_H_
#define ACTIONAUDIOADDMIDIPATTERN_H_

#include "../../Action.h"

class MidiPattern;

class ActionAudioAddMidiPattern: public Action
{
public:
	ActionAudioAddMidiPattern(const string &name, int num_beats, int beat_partition);
	virtual ~ActionAudioAddMidiPattern();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	MidiPattern *pattern;
};

#endif /* ACTIONAUDIOADDMIDIPATTERN_H_ */
