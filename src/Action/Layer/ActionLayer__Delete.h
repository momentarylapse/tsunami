/*
 * ActionLayer__Delete.h
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#ifndef SRC_ACTION_LAYER_ACTIONLAYER__DELETE_H_
#define SRC_ACTION_LAYER_ACTIONLAYER__DELETE_H_

#include "../Action.h"

class ActionLayer__Delete : public Action
{
public:
	ActionLayer__Delete(int index);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);
private:
	int index;
	string name;
};

#endif /* SRC_ACTION_LAYER_ACTIONLAYER__DELETE_H_ */
