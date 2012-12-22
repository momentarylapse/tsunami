/*
 * ActionTrackDelete.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKDELETET_H_
#define ACTIONTRACKDELETET_H_

#include "../ActionGroup.h"
#include "../../Data/AudioFile.h"

class ActionTrackDelete : public ActionGroup
{
public:
	ActionTrackDelete(AudioFile *a, int index);
	virtual ~ActionTrackDelete();
};

#endif /* ACTIONTRACKDELETET_H_ */
