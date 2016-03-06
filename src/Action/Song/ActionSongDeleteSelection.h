/*
 * ActionSongDeleteSelection.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#ifndef ACTIONSONGDELETESELECTION_H_
#define ACTIONSONGDELETESELECTION_H_

#include "../../Data/Song.h"
#include "../ActionGroup.h"

class SongSelection;

class ActionSongDeleteSelection : public ActionGroup
{
public:
	ActionSongDeleteSelection(Song *s, int level_no, const SongSelection &sel, bool all_levels);

	void DeleteBuffersFromTrackLevel(Song *s, Track *t, TrackLevel &l, const SongSelection &sel, int level_no);
};

#endif /* ACTIONSONGDELETESELECTION_H_ */
