/*
 * ActionSongSampleEditName.h
 *
 *  Created on: 28.03.2014
 *      Author: michi
 */

#ifndef ACTIONSAMPLEEDITNAME_H_
#define ACTIONSAMPLEEDITNAME_H_

#include <lib/history/MergableAction.h>

namespace tsunami {

struct Sample;

class ActionSampleEditName : public history::MergableValueAction<string> {
public:
	ActionSampleEditName(shared<Sample> s, const string &name);

	string name() const override { return ":##:change sample name"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

	bool mergable(Action *a) override;

private:
	shared<Sample> sample;
};

}

#endif /* ACTIONSAMPLEEDITNAME_H_ */
