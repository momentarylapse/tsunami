/*
 * Clipboard.h
 *
 *  Created on: 21.12.2012
 *      Author: michi
 */

#ifndef SRC_STUFF_CLIPBOARD_H_
#define SRC_STUFF_CLIPBOARD_H_

#include "../lib/base/pointer.h"
#include "../lib/pattern/Observable.h"
class Song;
class Track;
class TrackLayer;
class AudioView;
class PluginManager;

class Clipboard : public Observable<VirtualBase> {
	friend class PluginManager;
public:
	Clipboard();

	void copy(AudioView *view);
	void paste(AudioView *view);
	void paste_as_samples(AudioView *view);
	void paste_insert_time(AudioView *view);
	void paste_aligned_to_beats(AudioView *view);

	bool prepare_layer_map(AudioView *view, Array<TrackLayer*> &source, Array<TrackLayer*> &target);

	void clear();
	void _copy_append_track(TrackLayer *l, AudioView *view, int offset);
	void paste_track(TrackLayer *source, TrackLayer *target, int offset);
	void paste_track_as_samples(TrackLayer *source, int source_index, TrackLayer *target, int offset);
	bool has_data();
	bool can_copy(AudioView *view);

private:
	shared<Song> temp;
	int temp_offset = 0;
	Array<int> ref_uid;
};

#endif /* SRC_STUFF_CLIPBOARD_H_ */
