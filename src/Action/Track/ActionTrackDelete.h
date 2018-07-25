/*
 * ActionTrackDelete.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKDELETET_H_
#define ACTIONTRACKDELETET_H_

#include "../ActionGroup.h"

class Track;

class ActionTrackDelete : public ActionGroup
{
public:
	ActionTrackDelete(Track *track);

	void build(Data *d) override;

	Track *track;
};

#endif /* ACTIONTRACKDELETET_H_ */
