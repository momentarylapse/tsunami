/*
 * AudioViewLayer.h
 *
 *  Created on: 25.10.2014
 *      Author: michi
 */

#ifndef SRC_VIEW_GRAPH_AUDIOVIEWLAYER_H_
#define SRC_VIEW_GRAPH_AUDIOVIEWLAYER_H_

#include "../Helper/Graph/Node.h"
#include "../../lib/base/map.h"

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
class HoverData;
enum class NoteModifier;
enum class MidiMode;
class MidiKeyChange;
class LayerHeader;
class ScrollBar;
class AudioViewTrack;


class AudioViewLayer : public scenegraph::NodeFree {
public:
	AudioViewLayer(AudioView *v, TrackLayer *l);
	~AudioViewLayer() override;

	bool on_left_button_down(float mx, float my) override;
	bool on_left_double_click(float mx, float my) override;
	bool on_right_button_down(float mx, float my) override;

	bool allow_handle_click_when_gaining_focus() const override;

	string get_tip() const override;
	HoverData get_hover_data(float mx, float my) override;
	HoverData get_hover_data_default(float mx, float my);

	void on_layer_change();
	void on_track_change();


	color background_color() const;
	color background_selection_color() const;

	void draw_track_buffers(Painter *c);

	void draw_sample_frame(Painter *c, SampleRef *s, const color &col, int delay);
	void draw_sample(Painter *c, SampleRef *s);

	void draw_midi(Painter *c, const MidiNoteBuffer &midi, bool as_reference, int shift);

	void draw_marker(Painter *c, const TrackMarker *marker, bool hover, const Range &group_range, bool first, bool last);
	void draw_marker_group(Painter *c, const Array<TrackMarker*> &markers, HoverData &hover);
	void draw_markers(Painter *c, const Array<TrackMarker*> &markers, HoverData &hover);

	void draw_fades(Painter *c);

	void on_draw(Painter *c) override;

	bool on_screen();

	GridColors grid_colors();

	AudioView *view;
	TrackLayer *layer;
	Track *track();
	AudioViewTrack *vtrack();
	MidiMode midi_mode();
	rect area_last, area_target;
	int height;
	Map<const TrackMarker*, rect> marker_areas;
	Map<const TrackMarker*, rect> marker_label_areas;


	static color marker_color(const TrackMarker *m);

	void set_edit_pitch_min_max(float pitch_min, float pitch_max);
	float edit_pitch_min, edit_pitch_max;

	virtual bool is_playable() const;

	void update_midi_key_changes();
	Array<MidiKeyChange> midi_key_changes;


	void set_solo(bool solo);
	bool solo;

	bool represents_imploded;

	LayerHeader *header;
	void update_header();

	ScrollBar *scroll_bar;
};

#endif /* SRC_VIEW_GRAPH_AUDIOVIEWLAYER_H_ */
