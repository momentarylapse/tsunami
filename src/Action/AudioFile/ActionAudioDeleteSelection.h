/*
 * ActionAudioDeleteSelection.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#ifndef ACTIONAUDIODELETESELECTION_H_
#define ACTIONAUDIODELETESELECTION_H_

#include "../ActionGroup.h"
#include "../../Data/AudioFile.h"

class ActionAudioDeleteSelection : public ActionGroup
{
public:
	ActionAudioDeleteSelection(AudioFile *a, bool all_levels);
	virtual ~ActionAudioDeleteSelection();

	void DeleteBuffersFromTrackLevel(AudioFile *a, Track &t, TrackLevel &l, int level_no);
};

#endif /* ACTIONAUDIODELETESELECTION_H_ */
