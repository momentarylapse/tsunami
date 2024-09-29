/*
 * ActionSongSampleEditName.h
 *
 *  Created on: 28.03.2014
 *      Author: michi
 */

#ifndef ACTIONSAMPLEEDITNAME_H_
#define ACTIONSAMPLEEDITNAME_H_

#include "../ActionMergable.h"

namespace tsunami {

class Sample;

class ActionSampleEditName : public ActionMergable<string> {
public:
	ActionSampleEditName(shared<Sample> s, const string &name);

	string name() const override { return ":##:change sample name"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

	bool mergable(Action *a) override;

private:
	shared<Sample> sample;
};

}

#endif /* ACTIONSAMPLEEDITNAME_H_ */
