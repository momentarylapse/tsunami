/*
 * Clipboard.h
 *
 *  Created on: 21.12.2012
 *      Author: michi
 */

#ifndef CLIPBOARD_H_
#define CLIPBOARD_H_

#include "Observable.h"
class Song;
class Track;
class AudioView;

class Clipboard : public Observable
{
public:
	Clipboard();
	virtual ~Clipboard();

	void copy_from_track(Track *t, AudioView *view);
	void copy_from_selected_tracks(AudioView *view);
	void paste(AudioView *view);

	void clear();
	void append_track(Track *t, AudioView *view);
	void paste_track(int source_index, Track *target, AudioView *view);
	bool hasData();
	bool canCopy(AudioView *view);

private:
	Song *temp;
	Array<int> ref_uid;
};

#endif /* CLIPBOARD_H_ */
