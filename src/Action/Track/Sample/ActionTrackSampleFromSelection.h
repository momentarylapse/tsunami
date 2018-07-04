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
	void CreateSamplesFromTrackAudio(TrackLayer *l, const SongSelection &sel);
	void CreateSamplesFromTrackMidi(Track *t, const SongSelection &sel);

	const SongSelection &sel;
};

#endif /* ACTIONTRACKSAMPLEFROMSELECTION_H_ */
