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
class Song;
class BufferBox;
class MidiData;
class Sample;

class ActionTrackPasteAsSample : public ActionGroup
{
public:
	ActionTrackPasteAsSample(Track *t, int pos, const BufferBox &buf, bool auto_delete);
	ActionTrackPasteAsSample(Track *t, int pos, const MidiData &midi, bool auto_delete);

	virtual void build(Data *d);
	virtual void *execute_return(Data *d);

	Track *t;
	int pos;
	const BufferBox *buf;
	const MidiData *midi;
	Sample *sample;
	bool auto_delete;
};

#endif /* ACTIONTRACKPASTEASSAMPLE_H_ */
