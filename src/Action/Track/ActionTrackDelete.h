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

class ActionTrackDelete : public ActionGroup
{
public:
	ActionTrackDelete(Song *a, int index);
	virtual ~ActionTrackDelete();
};

#endif /* ACTIONTRACKDELETET_H_ */
