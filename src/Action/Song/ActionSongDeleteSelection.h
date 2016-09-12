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
	ActionSongDeleteSelection(int level_no, const SongSelection &sel, bool all_levels);

	virtual void build(Data *d);
	void DeleteBuffersFromTrackLevel(Song *s, Track *t, TrackLevel &l, const SongSelection &sel, int level_no);

	int level_no;
	const SongSelection &sel;
	bool all_levels;
};

#endif /* ACTIONSONGDELETESELECTION_H_ */
