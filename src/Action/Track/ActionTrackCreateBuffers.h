/*
 * ActionTrackCreateBuffers.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKCREATEBUFFERS_H_
#define ACTIONTRACKCREATEBUFFERS_H_

#include "../ActionGroup.h"
#include "../../Data/Track.h"
#include "../../lib/file/file.h"

class ActionTrackCreateBuffers : public ActionGroup
{
public:
	ActionTrackCreateBuffers(Track *t, const Range &r);
	virtual ~ActionTrackCreateBuffers();
};

#endif /* ACTIONTRACKCREATEBUFFERS_H_ */
