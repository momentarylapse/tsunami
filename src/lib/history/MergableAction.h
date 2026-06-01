//
// Created by michi on 11/15/25.
//

#pragma once

#include "Action.h"

namespace history {

class MergableAction : public Action {
public:
	MergableAction();

	// merge this (later) into previous
	// AFTER both have been executed!
	virtual bool absorb(Action* previous) = 0;
};

template<class T>
class MergableValueAction : public MergableAction {
public:
	virtual bool mergable(Action* previous) = 0;
	bool absorb(Action* previous) override {
		if (!mergable(previous))
			return false;
		new_value = dynamic_cast<MergableValueAction<T>*>(previous)->new_value;
		return true;
	}
	T old_value;
	T new_value;
};

}

