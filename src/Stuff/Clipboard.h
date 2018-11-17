/*
 * Clipboard.h
 *
 *  Created on: 21.12.2012
 *      Author: michi
 */

#ifndef SRC_STUFF_CLIPBOARD_H_
#define SRC_STUFF_CLIPBOARD_H_

#include "Observable.h"
class Song;
class Track;
class TrackLayer;
class AudioView;

class Clipboard : public Observable<VirtualBase>
{
public:
	Clipboard();
	virtual ~Clipboard();

	void copy(AudioView *view);
	void paste(AudioView *view);
	void paste_as_samples(AudioView *view);
	void paste_with_time(AudioView *view);

	bool test_compatibility(AudioView *view);

	void clear();
	void append_track(TrackLayer *l, AudioView *view, int offset);
	void paste_track(int source_index, TrackLayer *target, AudioView *view);
	void paste_track_as_samples(int source_index, TrackLayer *target, AudioView *view);
	bool has_data();
	bool can_copy(AudioView *view);

private:
	Song *temp;
	Array<int> ref_uid;
};

#endif /* SRC_STUFF_CLIPBOARD_H_ */
