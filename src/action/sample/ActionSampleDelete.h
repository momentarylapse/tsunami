/*
 * ActionSampleDelete.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef ACTIONSAMPLEDELETE_H_
#define ACTIONSAMPLEDELETE_H_

#include "../Action.h"

namespace tsunami {

class Sample;

class ActionSampleDelete : public Action {
public:
	explicit ActionSampleDelete(shared<Sample> s);

	string name() const override { return ":##:delete sample"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	shared<Sample> sample;
	int index;
};

}

#endif /* ACTIONSAMPLEDELETE_H_ */
