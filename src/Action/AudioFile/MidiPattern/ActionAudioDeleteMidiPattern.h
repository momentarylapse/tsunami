/*
 * ActionAudioDeleteMidiPattern.h
 *
 *  Created on: 14.08.2013
 *      Author: michi
 */

#ifndef ACTIONAUDIODELETEMIDIPATTERN_H_
#define ACTIONAUDIODELETEMIDIPATTERN_H_

#include "../../Action.h"

class MidiPattern;

class ActionAudioDeleteMidiPattern: public Action
{
public:
	ActionAudioDeleteMidiPattern(int index);
	virtual ~ActionAudioDeleteMidiPattern();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	MidiPattern *pattern;
	int index;
};

#endif /* ACTIONAUDIODELETEMIDIPATTERN_H_ */
