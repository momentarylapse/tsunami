/*
 * ActionTrackPasteAsSample.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef ACTIONTRACKPASTEASSAMPLE_H_
#define ACTIONTRACKPASTEASSAMPLE_H_

#include <lib/history/ActionGroup.h>

namespace tsunami {

struct TrackLayer;
struct Song;
struct AudioBuffer;
struct MidiNoteBuffer;
struct Sample;

class ActionTrackPasteAsSample : public history::ActionGroup {
public:
	ActionTrackPasteAsSample(TrackLayer *t, int pos, const AudioBuffer &buf, bool auto_delete);
	ActionTrackPasteAsSample(TrackLayer *t, int pos, const MidiNoteBuffer &midi, bool auto_delete);

	void* compose(history::Data* d) override;

	TrackLayer *layer;
	int pos;
	const AudioBuffer *buf;
	const MidiNoteBuffer *midi;
	Sample *sample;
	bool auto_delete;
};

}

#endif /* ACTIONTRACKPASTEASSAMPLE_H_ */
