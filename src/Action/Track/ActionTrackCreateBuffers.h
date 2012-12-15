/*
 * ActionTrackCreateBuffers.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKCREATEBUFFERS_H_
#define ACTIONTRACKCREATEBUFFERS_H_

#include "../ActionGroup.h"
class Track;
class Range;

class ActionTrackCreateBuffers : public ActionGroup
{
public:
	ActionTrackCreateBuffers(Track *t, int level_no, const Range &r);
	virtual ~ActionTrackCreateBuffers();
};

#endif /* ACTIONTRACKCREATEBUFFERS_H_ */
