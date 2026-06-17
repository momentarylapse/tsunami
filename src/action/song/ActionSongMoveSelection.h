/*
 * ActionSongMoveSelection.h
 *
 *  Created on: 17.07.2018
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>

namespace tsunami {

class Song;
class Track;
class TrackLayer;
class SampleRef;
class MidiNote;
class TrackMarker;
class SongSelection;

class ActionSongMoveSelection: public history::Action {
public:
	ActionSongMoveSelection(Song *s, const SongSelection &sel, bool move_buffers);

	string name() const override { return ":##:move selection"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

	// continuous editing
	void abort(history::Data *d) override;
	virtual void abort_and_notify(history::Data *d);
	virtual void set_param_and_notify(history::Data *d, int _param);

	bool is_trivial() const override;

private:
	struct SampleSaveData {
		SampleRef *sample;
		int pos_old;
	};
	Array<SampleSaveData> samples;
	struct NoteSaveData {
		MidiNote *note;
		int pos_old;
	};
	Array<NoteSaveData> notes;
	struct MarkerSaveData {
		TrackMarker *marker;
		int pos_old;
	};
	Array<MarkerSaveData> markers;
	struct BufferSaveData {
		TrackLayer *layer;
		int index;
		int pos_old;
	};
	Array<BufferSaveData> buffers;
	Array<const Track*> tracks;
	int param;
};

}

