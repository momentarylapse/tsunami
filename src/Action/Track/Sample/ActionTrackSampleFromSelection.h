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

class ActionTrackSampleFromSelection : public ActionGroup
{
public:
	ActionTrackSampleFromSelection(const SongSelection &sel, int layer_no);

	virtual void build(Data *d);

private:
	void CreateSamplesFromTrackAudio(Track *t, const SongSelection &sel, int layer_no);
	void CreateSamplesFromTrackMidi(Track *t, const SongSelection &sel);

	const SongSelection &sel;
	int layer_no;
};

#endif /* ACTIONTRACKSAMPLEFROMSELECTION_H_ */
