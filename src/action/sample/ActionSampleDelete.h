/*
 * ActionSampleDelete.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef ACTIONSAMPLEDELETE_H_
#define ACTIONSAMPLEDELETE_H_

#include <lib/history/Action.h>

namespace tsunami {

class Sample;

class ActionSampleDelete : public history::Action {
public:
	explicit ActionSampleDelete(shared<Sample> s);

	string name() const override { return ":##:delete sample"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	shared<Sample> sample;
	int index;
};

}

#endif /* ACTIONSAMPLEDELETE_H_ */
