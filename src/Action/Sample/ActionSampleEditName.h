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

	void *execute(Data *d) override;
	void undo(Data *d) override;

	bool mergable(Action *a) override;

private:
	Sample *sample;
};

#endif /* ACTIONSAMPLEEDITNAME_H_ */
