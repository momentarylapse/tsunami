/*
 * ActionTrackPasteAsSample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "ActionTrackPasteAsSample.h"
#include "ActionTrackAddSampleRef.h"
#include "../../AudioFile/Sample/ActionAudioAddSample.h"
#include "../../../Data/AudioFile.h"

ActionTrackPasteAsSample::ActionTrackPasteAsSample(AudioFile *a, int track_no, int pos, BufferBox *buf) :
	ActionGroup()
{
	int i = (int)(long)AddSubAction(new ActionAudioAddSample("-paste-", *buf), a);
	AddSubAction(new ActionTrackAddSampleRef(track_no, pos, i), a);
}

ActionTrackPasteAsSample::~ActionTrackPasteAsSample()
{
}

