/*
 * ActionSongDeleteSelection.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_SONG_ACTIONSONGDELETESELECTION_H_
#define SRC_ACTION_SONG_ACTIONSONGDELETESELECTION_H_

#include "../../Data/Song.h"
#include "../ActionGroup.h"

class SongSelection;

class ActionSongDeleteSelection : public ActionGroup
{
public:
	ActionSongDeleteSelection(int layer_no, const SongSelection &sel, bool all_layers);

	virtual void build(Data *d);
	void DeleteBuffersFromTrackLayer(Song *s, Track *t, TrackLayer &l, const SongSelection &sel, int layer_no);

	int layer_no;
	const SongSelection &sel;
	bool all_layers;
};

#endif /* SRC_ACTION_SONG_ACTIONSONGDELETESELECTION_H_ */
