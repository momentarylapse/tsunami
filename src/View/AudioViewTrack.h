/*
 * AudioViewTrack.h
 *
 *  Created on: 25.10.2014
 *      Author: michi
 */

#ifndef SRC_VIEW_AUDIOVIEWTRACK_H_
#define SRC_VIEW_AUDIOVIEWTRACK_H_

#include "../lib/math/math.h"
#include "../Stuff/Observable.h"

class Track;
class Painter;
class AudioView;
class MidiKeyChange;


class AudioViewTrack : public Observable<VirtualBase>
{
public:
	AudioViewTrack(AudioView *view, Track *track);
	virtual ~AudioViewTrack();

	void draw_header(Painter *c);
	void draw(Painter *c);

	void set_solo(bool solo);
	void set_muted(bool muted);
	void set_panning(float panning);
	void set_volume(float volume);

	void on_track_change();

	Track *track;
	rect area;
	Array<int> reference_tracks;
	bool solo;
	AudioView *view;
	static const float MIN_GRID_DIST;

	bool is_playable();


	bool imploded;
};

#endif /* SRC_VIEW_AUDIOVIEWTRACK_H_ */
