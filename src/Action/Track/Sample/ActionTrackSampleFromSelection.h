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
	ActionTrackSampleFromSelection(Song *a, const SongSelection &sel, int level_no);

private:
	void CreateSubsFromTrack(Track *t, const SongSelection &sel, int level_no);
};

#endif /* ACTIONTRACKSAMPLEFROMSELECTION_H_ */
