/*
 * ActionSubTrackInsert.h
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#ifndef ACTIONSUBTRACKINSERT_H_
#define ACTIONSUBTRACKINSERT_H_

#include "../ActionGroup.h"
#include "../../Data/AudioFile.h"

class ActionSubTrackInsert : public ActionGroup
{
public:
	ActionSubTrackInsert(AudioFile *a, int track_no, int index, int level_no);
	virtual ~ActionSubTrackInsert();
};

#endif /* ACTIONSUBTRACKINSERT_H_ */
