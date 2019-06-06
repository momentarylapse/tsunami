/*
 * ActionSampleDelete.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef ACTIONSAMPLEDELETE_H_
#define ACTIONSAMPLEDELETE_H_

#include "../Action.h"

class Sample;

class ActionSampleDelete : public Action
{
public:
	ActionSampleDelete(Sample *s);
	virtual ~ActionSampleDelete();

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	Sample *sample;
	int index;
};

#endif /* ACTIONSAMPLEDELETE_H_ */
