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

class ActionSampleAdd : public Action {
public:
	ActionSampleAdd(Sample *s);
	~ActionSampleAdd() override;

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	Sample *sample;
};

#endif /* ACTIONSAMPLEADD_H_ */
