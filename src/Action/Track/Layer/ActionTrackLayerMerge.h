/*
 * ActionLayerMerge.h
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#ifndef SRC_ACTION_LAYER_ACTIONTRACKLAYERMERGE_H_
#define SRC_ACTION_LAYER_ACTIONTRACKLAYERMERGE_H_

#include "../../ActionGroup.h"

class Track;

class ActionTrackLayerMerge : public ActionGroup
{
public:
	ActionTrackLayerMerge(Track *t);

	void build(Data *d) override;

	Track *track;
};

#endif /* SRC_ACTION_LAYER_ACTIONTRACKLAYERMERGE_H_ */
