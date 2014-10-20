/*
 * ActionTrackPasteAsSample.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef ACTIONTRACKPASTEASSAMPLE_H_
#define ACTIONTRACKPASTEASSAMPLE_H_

#include "../../ActionGroup.h"

class Track;
class AudioFile;
class BufferBox;
class MidiData;

class ActionTrackPasteAsSample : public ActionGroup
{
public:
	ActionTrackPasteAsSample(Track *t, int pos, BufferBox *buf);
	ActionTrackPasteAsSample(Track *t, int pos, MidiData *midi);
	virtual ~ActionTrackPasteAsSample();
};

#endif /* ACTIONTRACKPASTEASSAMPLE_H_ */
