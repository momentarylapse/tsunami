/*
 * ActionTrackCreateBuffers.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_BUFFER_ACTIONTRACKCREATEBUFFERS_H_
#define SRC_ACTION_TRACK_BUFFER_ACTIONTRACKCREATEBUFFERS_H_

#include "../../ActionGroup.h"
#include "../../../data/Range.h"

class Track;
class TrackLayer;


class ActionTrackCreateBuffers : public ActionGroup
{
public:
	ActionTrackCreateBuffers(TrackLayer *l, const Range &r);

	virtual void build(Data *d);

	TrackLayer *layer;
	Range r;
};

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACKCREATEBUFFERS_H_ */
