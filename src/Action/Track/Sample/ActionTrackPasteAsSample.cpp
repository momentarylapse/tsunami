/*
 * ActionTrackPasteAsSample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "ActionTrackPasteAsSample.h"
#include "ActionTrackAddSample.h"
#include "../../AudioFile/Sample/ActionAudioAddSample.h"
#include "../../../Data/AudioFile.h"

ActionTrackPasteAsSample::ActionTrackPasteAsSample(AudioFile *a, int track_no, int pos, BufferBox *buf) :
	ActionGroup()
{
	AddSubAction(new ActionAudioAddSample("-paste-", *buf), a);
	AddSubAction(new ActionTrackAddSample(track_no, pos, a->sample.num - 1), a);
}

ActionTrackPasteAsSample::~ActionTrackPasteAsSample()
{
}

