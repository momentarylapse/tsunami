/*
 * ActionTrackDelete.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKDELETET_H_
#define ACTIONTRACKDELETET_H_

#include "../../Data/Song.h"
#include "../ActionGroup.h"

class Track;

class ActionTrackDelete : public ActionGroup
{
public:
	ActionTrackDelete(Track *track);

	virtual void build(Data *d);

	Track *track;
};

#endif /* ACTIONTRACKDELETET_H_ */
