/*
 * ActionSongSampleEditName.h
 *
 *  Created on: 28.03.2014
 *      Author: michi
 */

#ifndef ACTIONSAMPLEEDITNAME_H_
#define ACTIONSAMPLEEDITNAME_H_

#include "../ActionMergable.h"

class Sample;

class ActionSampleEditName: public ActionMergable<string>
{
public:
	ActionSampleEditName(Sample *s, const string &name);
	virtual ~ActionSampleEditName();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	virtual bool mergable(Action *a);

private:
	Sample *sample;
};

#endif /* ACTIONSAMPLEEDITNAME_H_ */
