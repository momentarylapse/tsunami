/*
 * ActionSongMoveSelection.h
 *
 *  Created on: 17.07.2018
 *      Author: michi
 */

#pragma once


#include "../Action.h"
class Song;
class Track;
class TrackLayer;
class SampleRef;
class MidiNote;
class TrackMarker;
class SongSelection;

class ActionSongMoveSelection: public Action {
public:
	ActionSongMoveSelection(Song *s, const SongSelection &sel, bool move_buffers);

	string name() const override { return ":##:move selection"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

	// continuous editing
	virtual void abort(Data *d);
	virtual void abort_and_notify(Data *d);
	virtual void set_param_and_notify(Data *d, int _param);

	bool is_trivial() override;

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

