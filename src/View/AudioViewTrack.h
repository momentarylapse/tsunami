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


class AudioViewTrack : public Observable<VirtualBase>
{
public:
	AudioViewTrack(AudioView *view, Track *track);
	virtual ~AudioViewTrack();

	void drawHeader(Painter *c);
	void draw(Painter *c);

	void setSolo(bool solo);
	void setMuted(bool muted);
	void setPanning(float panning);
	void setVolume(float volume);

	void on_track_change();

	Track *track;
	rect area;
	Array<int> reference_tracks;
	bool solo;
	AudioView *view;
	static const float MIN_GRID_DIST;

	bool is_playable();


	//Array<AudioViewLayer*> layers;
};

#endif /* SRC_VIEW_AUDIOVIEWTRACK_H_ */
