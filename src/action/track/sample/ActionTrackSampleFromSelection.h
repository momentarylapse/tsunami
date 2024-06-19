/*
 * ActionTrackSampleFromSelection.h
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKSAMPLEFROMSELECTION_H_
#define ACTIONTRACKSAMPLEFROMSELECTION_H_

#include "../../ActionGroup.h"

namespace tsunami {

class SongSelection;
class TrackLayer;

class ActionTrackSampleFromSelection : public ActionGroup {
public:
	ActionTrackSampleFromSelection(const SongSelection &sel, bool auto_delete);

	void build(Data *d) override;

private:
	void CreateSamplesFromLayerAudio(TrackLayer *l);
	void CreateSamplesFromLayerMidi(TrackLayer *l);

	const SongSelection &sel;
	bool auto_delete;
};

}

#endif /* ACTIONTRACKSAMPLEFROMSELECTION_H_ */
