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
	ActionAudioDeleteSelection(AudioFile *a, int level_no, const Range &range, bool all_levels);

	void DeleteBuffersFromTrackLevel(AudioFile *a, Track *t, TrackLevel &l, const Range &range, int level_no);
};

#endif /* ACTIONAUDIODELETESELECTION_H_ */
