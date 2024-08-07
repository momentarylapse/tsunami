/*
 * ActionSongDeleteSelection.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#pragma once

#include "../../data/Song.h"
#include "../ActionGroup.h"

namespace tsunami {

class SongSelection;

class ActionSongDeleteSelection : public ActionGroup {
public:
	ActionSongDeleteSelection(const SongSelection &sel);

	string name() const override { return ":##:delete selection"; }

	void build(Data *d) override;
	void delete_buffers_from_track_layer(Song *s, Track *t, TrackLayer *l, const SongSelection &sel);

	const SongSelection &sel;
};

}

