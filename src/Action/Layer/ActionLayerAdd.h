/*
 * ActionLayerAdd.h
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_LAYER_ACTIONLAYERADD_H_
#define SRC_ACTION_LAYER_ACTIONLAYERADD_H_

#include "../Action.h"

class ActionLayerAdd : public Action
{
public:
	ActionLayerAdd(const string &name, int index);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);
private:
	string name;
	int index;
};

#endif /* SRC_ACTION_LAYER_ACTIONLAYERADD_H_ */
