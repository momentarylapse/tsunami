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

class ActionSongDeleteSelection : public ActionGroup {
public:
	ActionSongDeleteSelection(const SongSelection &sel);

	void build(Data *d) override;
	void DeleteBuffersFromTrackLayer(Song *s, Track *t, TrackLayer *l, const SongSelection &sel);

	const SongSelection &sel;
};

#endif /* SRC_ACTION_SONG_ACTIONSONGDELETESELECTION_H_ */
