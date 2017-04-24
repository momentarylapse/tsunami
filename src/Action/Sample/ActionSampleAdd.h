/*
 * ActionSampleAdd.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef ACTIONSAMPLEADD_H_
#define ACTIONSAMPLEADD_H_

#include "../Action.h"

class BufferBox;
class MidiData;
class Sample;

class ActionSampleAdd : public Action
{
public:
	ActionSampleAdd(const string &name, const BufferBox &buf, bool auto_delete);
	ActionSampleAdd(const string &name, const MidiData &midi, bool auto_delete);
	virtual ~ActionSampleAdd();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	Sample *sample;
};

#endif /* ACTIONSAMPLEADD_H_ */
