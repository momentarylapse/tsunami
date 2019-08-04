/*
 * ActionSubTrackInsert.h
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKINSERTSAMPLE_H_
#define ACTIONTRACKINSERTSAMPLE_H_

#include "../../ActionGroup.h"
class TrackLayer;

class ActionTrackInsertSample : public ActionGroup {
public:
	ActionTrackInsertSample(TrackLayer *layer, int index);

	void build(Data *d) override;

	TrackLayer *layer;
	int index;
};

#endif /* ACTIONTRACKINSERTSAMPLE_H_ */
