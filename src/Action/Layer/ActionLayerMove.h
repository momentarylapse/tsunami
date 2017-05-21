/*
 * ActionLayerMove.h
 *
 *  Created on: 21.05.2017
 *      Author: michi
 */

#ifndef SRC_ACTION_LAYER_ACTIONLAYERMOVE_H_
#define SRC_ACTION_LAYER_ACTIONLAYERMOVE_H_

#include "../Action.h"

class ActionLayerMove : public Action
{
public:
	ActionLayerMove(int source, int target);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	int source, target;
};

#endif /* SRC_ACTION_LAYER_ACTIONLAYERMOVE_H_ */
