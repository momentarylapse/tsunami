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
class AudioView;

class Clipboard : public Observable<VirtualBase>
{
public:
	Clipboard();
	virtual ~Clipboard();

	void copy(AudioView *view);
	void paste(AudioView *view);
	void pasteAsSamples(AudioView *view);

	bool test_compatibility(AudioView *view, bool *paste_single);

	void clear();
	void append_track(Track *t, AudioView *view);
	void paste_track(int source_index, Track *target, AudioView *view);
	void paste_track_as_samples(int source_index, Track *target, AudioView *view);
	bool hasData();
	bool canCopy(AudioView *view);

private:
	Song *temp;
	Array<int> ref_uid;
};

#endif /* SRC_STUFF_CLIPBOARD_H_ */
