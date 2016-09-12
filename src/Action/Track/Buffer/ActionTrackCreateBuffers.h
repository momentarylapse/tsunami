/*
 * ActionTrackCreateBuffers.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKCREATEBUFFERS_H_
#define ACTIONTRACKCREATEBUFFERS_H_

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

#endif /* ACTIONTRACKCREATEBUFFERS_H_ */
