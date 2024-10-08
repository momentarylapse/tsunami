/*
 * AudioViewTrack.h
 *
 *  Created on: 25.10.2014
 *      Author: michi
 */

#ifndef SRC_VIEW_GRAPH_AUDIOVIEWTRACK_H_
#define SRC_VIEW_GRAPH_AUDIOVIEWTRACK_H_

#include "../../helper/graph/Node.h"

class Painter;

namespace tsunami {

class Track;
class AudioView;
class AudioViewLayer;
class MidiKeyChange;
class TrackHeader;
enum class MidiMode;

enum class AudioViewMode {
	Peaks,
	Spectrum
};


class AudioViewTrack : public ::scenegraph::NodeFree {
public:
	AudioViewTrack(AudioView *view, Track *track);
	~AudioViewTrack() override;

	obs::source out_solo_changed{this, "solo-changed"};
	obs::sink in_track_changed;

	void draw_imploded_data(Painter *c);
	void on_draw(Painter *c) override;

	void set_solo(bool solo);
	void set_muted(bool muted);
	void set_panning(float panning);
	void set_volume(float volume);

	void on_track_change();

	AudioView *view;
	Track *track;
	TrackHeader *header;
	AudioViewLayer *first_layer();
	Array<int> reference_tracks;
	bool solo;
	static const float MinGridDist;

	bool is_playable();

	void set_midi_mode(MidiMode mode);
	MidiMode midi_mode_wanted;
	MidiMode midi_mode();

	void set_audio_mode(AudioViewMode mode);
	AudioViewMode audio_mode;

	bool imploded;
};

}

#endif /* SRC_VIEW_GRAPH_AUDIOVIEWTRACK_H_ */
