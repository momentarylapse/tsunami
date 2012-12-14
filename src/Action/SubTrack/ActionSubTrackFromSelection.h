/*
 * ActionSubTrackFromSelection.h
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#ifndef ACTIONSUBTRACKFROMSELECTION_H_
#define ACTIONSUBTRACKFROMSELECTION_H_

#include "../ActionGroup.h"
#include "../../Data/AudioFile.h"

class ActionSubTrackFromSelection : public ActionGroup
{
public:
	ActionSubTrackFromSelection(AudioFile *a, int level_no);
	virtual ~ActionSubTrackFromSelection();

private:
	void CreateSubsFromTrack(AudioFile *a, Track *t, int track_no, int level_no);
};

#endif /* ACTIONSUBTRACKFROMSELECTION_H_ */
