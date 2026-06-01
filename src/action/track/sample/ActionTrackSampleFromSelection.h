/*
 * ActionTrackSampleFromSelection.h
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKSAMPLEFROMSELECTION_H_
#define ACTIONTRACKSAMPLEFROMSELECTION_H_

#include <lib/history/ActionGroup.h>

namespace tsunami {

class SongSelection;
class TrackLayer;

class ActionTrackSampleFromSelection : public history::ActionGroup {
public:
	ActionTrackSampleFromSelection(const SongSelection &sel, bool auto_delete);

	void* compose(history::Data* d) override;

private:
	void CreateSamplesFromLayerAudio(TrackLayer *l);
	void CreateSamplesFromLayerMidi(TrackLayer *l);

	const SongSelection &sel;
	bool auto_delete;
};

}

#endif /* ACTIONTRACKSAMPLEFROMSELECTION_H_ */
