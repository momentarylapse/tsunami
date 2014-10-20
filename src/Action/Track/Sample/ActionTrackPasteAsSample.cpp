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
	AddSubAction(new ActionAudioAddSample("-paste-", *buf), t->root);
	AddSubAction(new ActionTrackAddSample(t, pos, t->root->sample.num - 1), t->root);
}

ActionTrackPasteAsSample::ActionTrackPasteAsSample(Track *t, int pos, MidiData *midi) :
	ActionGroup()
{
	AddSubAction(new ActionAudioAddSample("-paste-", *midi), t->root);
	AddSubAction(new ActionTrackAddSample(t, pos, t->root->sample.num - 1), t->root);
}

ActionTrackPasteAsSample::~ActionTrackPasteAsSample()
{
}

