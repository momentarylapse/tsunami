/*
 * ActionLayerDelete.h
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#ifndef SRC_ACTION_LAYER_ACTIONLAYERDELETE_H_
#define SRC_ACTION_LAYER_ACTIONLAYERDELETE_H_

#include "../../ActionGroup.h"

class Track;

class ActionLayerDelete : public ActionGroup
{
public:
	ActionLayerDelete(Track *t, int index);

	void build(Data *d) override;

	Track *track;
	int index;
};

#endif /* SRC_ACTION_LAYER_ACTIONLAYERDELETE_H_ */
