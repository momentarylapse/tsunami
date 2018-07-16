/*
 * ActionSongMoveSelection.h
 *
 *  Created on: 17.07.2018
 *      Author: michi
 */

#ifndef SRC_ACTION_SONG_ACTIONSONGMOVESELECTION_H_
#define SRC_ACTION_SONG_ACTIONSONGMOVESELECTION_H_


#include "../Action.h"
class Song;
class Track;
class SampleRef;
class MidiNote;
class TrackMarker;
class SongSelection;

class ActionSongMoveSelection: public Action
{
public:
	ActionSongMoveSelection(Song *s, const SongSelection &sel);

	void *execute(Data *d) override;
	void undo(Data *d) override;

	// continuous editing
	virtual void abort(Data *d);
	virtual void abort_and_notify(Data *d);
	virtual void set_param_and_notify(Data *d, int _param);

	bool is_trivial() override;

private:
	struct SampleSaveData{
		Track *track;
		SampleRef *sample;
		int pos_old;
	};
	Array<SampleSaveData> samples;
	struct NoteSaveData{
		MidiNote *note;
		int pos_old;
	};
	Array<NoteSaveData> notes;
	struct MarkerSaveData{
		Track *track;
		TrackMarker *marker;
		int pos_old;
	};
	Array<MarkerSaveData> markers;
	int param;
};

#endif /* SRC_ACTION_SONG_ACTIONSONGMOVESELECTION_H_ */
