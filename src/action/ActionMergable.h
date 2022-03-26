/*
 * ActionMergable.h
 *
 *  Created on: Mar 17, 2014
 *      Author: ankele
 */

#ifndef ACTIONMERGABLE_H_
#define ACTIONMERGABLE_H_

#include "Action.h"

class ActionMergableBase : public Action {
public:

	virtual bool mergable(Action *a) = 0;
	virtual bool absorb(ActionMergableBase *a) = 0;
};

template<class T>
class ActionMergable : public ActionMergableBase {
public:

	bool absorb(ActionMergableBase *a) override {
		if (!mergable(a))
			return false;
		new_value = dynamic_cast<ActionMergable<T>*>(a)->new_value;
		return true;
	}

protected:
	T new_value;
	T old_value;
};

#endif /* ACTIONMERGABLE_H_ */
