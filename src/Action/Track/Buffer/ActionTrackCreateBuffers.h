/*
 * ActionTrackCreateBuffers.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_BUFFER_ACTIONTRACKCREATEBUFFERS_H_
#define SRC_ACTION_TRACK_BUFFER_ACTIONTRACKCREATEBUFFERS_H_

#include "../../ActionGroup.h"
#include "../../../Data/Range.h"
class Track;


class ActionTrackCreateBuffers : public ActionGroup
{
public:
	ActionTrackCreateBuffers(Track *t, int level_no, const Range &r);

	virtual void build(Data *d);

	int track_no;
	int level_no;
	Range r;
};

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACKCREATEBUFFERS_H_ */
