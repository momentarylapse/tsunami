/*
 * ActionSongAddSample.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef ACTIONSONGADDSAMPLE_H_
#define ACTIONSONGADDSAMPLE_H_

#include "../../Action.h"

class BufferBox;
class MidiNoteData;
class Sample;

class ActionSongAddSample : public Action
{
public:
	ActionSongAddSample(const string &name, BufferBox &buf);
	ActionSongAddSample(const string &name, MidiNoteData &midi);
	virtual ~ActionSongAddSample();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	Sample *sample;
};

#endif /* ACTIONSONGADDSAMPLE_H_ */
