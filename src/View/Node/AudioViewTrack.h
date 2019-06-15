/*
 * AudioViewTrack.h
 *
 *  Created on: 25.10.2014
 *      Author: michi
 */

#ifndef SRC_VIEW_NODE_AUDIOVIEWTRACK_H_
#define SRC_VIEW_NODE_AUDIOVIEWTRACK_H_

#include "ViewNode.h"

class Track;
class Painter;
class AudioView;
class AudioViewLayer;
class MidiKeyChange;


class AudioViewTrack : public ViewNodeFree {
public:
	AudioViewTrack(AudioView *view, Track *track);
	~AudioViewTrack() override;

	void draw_imploded_data(Painter *c);
	void draw(Painter *c) override;

	void set_solo(bool solo);
	void set_muted(bool muted);
	void set_panning(float panning);
	void set_volume(float volume);

	void on_track_change();

	AudioView *view;
	Track *track;
	AudioViewLayer *first_layer();
	Array<int> reference_tracks;
	bool solo;
	static const float MIN_GRID_DIST;

	bool is_playable();


	bool imploded;
};

#endif /* SRC_VIEW_NODE_AUDIOVIEWTRACK_H_ */
