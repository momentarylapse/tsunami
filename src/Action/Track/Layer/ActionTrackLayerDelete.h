/*
 * ActionTrackLayerDelete.h
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYERDELETE_H_
#define SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYERDELETE_H_

#include "../../ActionGroup.h"

class Track;

class ActionTrackLayerDelete : public ActionGroup
{
public:
	ActionTrackLayerDelete(Track *t, int index);

	void build(Data *d) override;

	Track *track;
	int index;
};

#endif /* SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYERDELETE_H_ */
