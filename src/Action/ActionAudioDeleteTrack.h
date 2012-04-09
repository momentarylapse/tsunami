/*
 * ActionAudioDeleteTrack.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#ifndef ACTIONAUDIODELETETRACK_H_
#define ACTIONAUDIODELETETRACK_H_

#include "ActionGroup.h"
#include "../Data/AudioFile.h"

class ActionAudioDeleteTrack : public ActionGroup
{
public:
	ActionAudioDeleteTrack(AudioFile *a, int index);
	virtual ~ActionAudioDeleteTrack();
};

#endif /* ACTIONAUDIODELETETRACK_H_ */
