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

ActionTrackPasteAsSample::ActionTrackPasteAsSample(Track *t, int pos, BufferBox *buf) :
	ActionGroup()
{
	addSubAction(new ActionAudioAddSample("-paste-", *buf), t->root);
	addSubAction(new ActionTrackAddSample(t, pos, t->root->sample.num - 1), t->root);
}

ActionTrackPasteAsSample::ActionTrackPasteAsSample(Track *t, int pos, MidiData *midi) :
	ActionGroup()
{
	addSubAction(new ActionAudioAddSample("-paste-", *midi), t->root);
	addSubAction(new ActionTrackAddSample(t, pos, t->root->sample.num - 1), t->root);
}

