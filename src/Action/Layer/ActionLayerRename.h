/*
 * ActionLayerRename.h
 *
 *  Created on: 30.03.2014
 *      Author: michi
 */

#ifndef SRC_ACTION_LAYER_ACTIONLAYERRENAME_H_
#define SRC_ACTION_LAYER_ACTIONLAYERRENAME_H_

#include "../Action.h"

class ActionLayerRename : public Action
{
public:
	ActionLayerRename(int index, const string &name);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);
private:
	int index;
	string name;
};

#endif /* SRC_ACTION_LAYER_ACTIONLAYERRENAME_H_ */
