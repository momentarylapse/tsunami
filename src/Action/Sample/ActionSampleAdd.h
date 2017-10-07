/*
 * ActionSampleAdd.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef ACTIONSAMPLEADD_H_
#define ACTIONSAMPLEADD_H_

#include "../Action.h"

class AudioBuffer;
class MidiNoteBuffer;
class Sample;

class ActionSampleAdd : public Action
{
public:
	ActionSampleAdd(const string &name, const AudioBuffer &buf, bool auto_delete);
	ActionSampleAdd(const string &name, const MidiNoteBuffer &midi, bool auto_delete);
	virtual ~ActionSampleAdd();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	Sample *sample;
};

#endif /* ACTIONSAMPLEADD_H_ */
