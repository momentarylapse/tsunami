/*
 * ActionTrackSampleFromSelection.h
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKSAMPLEFROMSELECTION_H_
#define ACTIONTRACKSAMPLEFROMSELECTION_H_

#include "../../ActionGroup.h"
class Song;
class SongSelection;
class Track;
class TrackLayer;

class ActionTrackSampleFromSelection : public ActionGroup
{
public:
	ActionTrackSampleFromSelection(const SongSelection &sel);

	virtual void build(Data *d);

private:
	void CreateSamplesFromLayerAudio(TrackLayer *l, const SongSelection &sel);
	void CreateSamplesFromLayerMidi(TrackLayer *l, const SongSelection &sel);

	const SongSelection &sel;
};

#endif /* ACTIONTRACKSAMPLEFROMSELECTION_H_ */
