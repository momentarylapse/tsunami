/*
 * ActionBar__Edit.h
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#ifndef SRC_ACTION_BAR_ACTIONBAR__EDIT_H_
#define SRC_ACTION_BAR_ACTIONBAR__EDIT_H_

#include <lib/history/Action.h>

namespace tsunami {

class ActionBar__Edit : public history::Action {
public:
	ActionBar__Edit(int index, int length, Array<int> &beats, int divisor);

	virtual void* execute(history::Data* d);
	virtual void undo(history::Data* d);

private:
	int length, divisor;
	Array<int> beats;
	int index;
};

}

#endif /* SRC_ACTION_BAR_ACTIONBAR__EDIT_H_ */
