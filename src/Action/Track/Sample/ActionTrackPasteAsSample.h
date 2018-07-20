/*
 * ActionTrackPasteAsSample.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef ACTIONTRACKPASTEASSAMPLE_H_
#define ACTIONTRACKPASTEASSAMPLE_H_

#include "../../ActionGroup.h"

class TrackLayer;
class Song;
class AudioBuffer;
class MidiNoteBuffer;
class Sample;

class ActionTrackPasteAsSample : public ActionGroup
{
public:
	ActionTrackPasteAsSample(TrackLayer *t, int pos, const AudioBuffer &buf, bool auto_delete);
	ActionTrackPasteAsSample(TrackLayer *t, int pos, const MidiNoteBuffer &midi, bool auto_delete);

	void build(Data *d) override;
	void *execute_return(Data *d) override;

	TrackLayer *layer;
	int pos;
	const AudioBuffer *buf;
	const MidiNoteBuffer *midi;
	Sample *sample;
	bool auto_delete;
};

#endif /* ACTIONTRACKPASTEASSAMPLE_H_ */
