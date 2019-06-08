/*
 * AudioViewLayer.h
 *
 *  Created on: 25.10.2014
 *      Author: michi
 */

#ifndef SRC_VIEW_NODE_AUDIOVIEWLAYER_H_
#define SRC_VIEW_NODE_AUDIOVIEWLAYER_H_

#include "ViewNode.h"

class Track;
class TrackLayer;
class Painter;
class AudioView;
class AudioBuffer;
class SampleRef;
class MidiNoteBuffer;
class MidiNote;
class MidiEvent;
class TrackMarker;
class Clef;
class Scale;
class Range;
class GridColors;
class Selection;
enum class NoteModifier;
enum class MidiMode;
class MidiKeyChange;
class LayerHeader;


class AudioViewLayer : public ViewNode
{
public:
	AudioViewLayer(AudioView *v, TrackLayer *l);
	~AudioViewLayer() override;

	void on_track_change();


	color background_color();
	color background_selection_color();

	void draw_track_buffers(Painter *c);

	void draw_sample_frame(Painter *c, SampleRef *s, const color &col, int delay);
	void draw_sample(Painter *c, SampleRef *s);

	void draw_midi(Painter *c, const MidiNoteBuffer &midi, bool as_reference, int shift);

	void draw_marker(Painter *c, const TrackMarker *marker, bool hover, const Range &group_range, bool first, bool last);
	void draw_marker_group(Painter *c, const Array<TrackMarker*> &markers, Selection &hover);
	void draw_markers(Painter *c, const Array<TrackMarker*> &markers, Selection &hover);

	void draw_fades(Painter *c);

	void draw(Painter *c) override;

	bool on_screen();

	GridColors grid_colors();

	TrackLayer *layer;
	rect area_last, area_target;
	int height_wish, height_min;
	Map<const TrackMarker*, rect> marker_areas;
	Map<const TrackMarker*, rect> marker_label_areas;
	void set_midi_mode(MidiMode wanted);
	MidiMode midi_mode;


	static color marker_color(const TrackMarker *m);

	void set_edit_pitch_min_max(int pitch_min, int pitch_max);
	int edit_pitch_min, edit_pitch_max;

	virtual bool is_playable();

	void update_midi_key_changes();
	Array<MidiKeyChange> midi_key_changes;


	void set_solo(bool solo);
	bool solo;

	bool represents_imploded;

	LayerHeader *header;
	void update_header();
};

#endif /* SRC_VIEW_NODE_AUDIOVIEWLAYER_H_ */
