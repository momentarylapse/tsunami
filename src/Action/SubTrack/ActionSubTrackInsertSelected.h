/*
 * ActionSubTrackInsertSelected.h
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#ifndef ACTIONSUBTRACKINSERTSELECTED_H_
#define ACTIONSUBTRACKINSERTSELECTED_H_

#include "../ActionGroup.h"
#include "../../Data/AudioFile.h"

class ActionSubTrackInsertSelected : public ActionGroup
{
public:
	ActionSubTrackInsertSelected(AudioFile *a, int level_no);
	virtual ~ActionSubTrackInsertSelected();
};

#endif /* ACTIONSUBTRACKINSERTSELECTED_H_ */
