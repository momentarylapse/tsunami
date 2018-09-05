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
enum class NoteModifier;
enum class MidiMode;


class AudioViewLayer : public Observable<VirtualBase>
{
public:
	AudioViewLayer(AudioView *v, TrackLayer *l);


	enum MidiNoteState
	{
		STATE_DEFAULT = 0,
		STATE_HOVER = 1,
		STATE_REFERENCE = 2,
		STATE_SELECTED = 4,
	};

	color getBackgroundColor();
	color getBackgroundSelectionColor();
	void drawBlankBackground(Painter *c);

	void drawMidiLinear(Painter *c, const MidiNoteBuffer &midi, bool as_reference, int shift);
	void drawMidiNoteLinear(Painter *c, const MidiNote &n, int shift, MidiNoteState state);
	void drawMidiTab(Painter *c, const MidiNoteBuffer &midi, bool as_reference, int shift);
	void drawMidiNoteTab(Painter *c, const MidiNote *n, int shift, MidiNoteState state);
	void drawMidiClefTab(Painter *c);
	void drawMidiClassical(Painter *c, const MidiNoteBuffer &midi, bool as_reference, int shift);
	void drawMidiClefClassical(Painter *c, const Clef &clef, const Scale &scale);
	void drawMidiNoteClassical(Painter *c, const MidiNote *n, int shift, MidiNoteState state, const Clef &clef);

	void drawTrackBuffers(Painter *c, double pos);
	void drawBuffer(Painter *c, AudioBuffer &b, double view_pos_rel, const color &col);
	void drawBufferSelection(Painter *c, AudioBuffer &b, double view_pos_rel, const color &col, const Range &r);

	void drawSampleFrame(Painter *c, SampleRef *s, const color &col, int delay);
	void drawSample(Painter *c, SampleRef *s);

	void drawMarker(Painter *c, const TrackMarker *marker, int index, bool hover);

	void drawVersionHeader(Painter *c);
	void draw(Painter *c);

	void draw_complex_note(Painter *c, const MidiNote *n, MidiNoteState state, float x1, float x2, float y, float r);
	static void draw_simple_note(Painter *c, float x1, float x2, float y, float r, float rx, const color &col, const color &col_shadow, bool force_circle);

	TrackLayer *layer;
	rect area;
	rect area_last, area_target;
	int height_wish, height_min;
	Array<rect> marker_areas;
	Array<rect> marker_label_areas;
	AudioView *view;
	void set_midi_mode(MidiMode wanted);
	MidiMode midi_mode;


	static color getPitchColor(int pitch);

	float clef_pos_to_screen(int pos);
	int screen_to_clef_pos(float y);
	float string_to_screen(int string);
	int screen_to_string(float y);

	float pitch2y_classical(int pitch);
	float pitch2y_linear(int pitch);
	int y2pitch_classical(float y, NoteModifier modifier);
	int y2pitch_linear(float y);
	int y2clef_classical(float y, NoteModifier &mod);
	int y2clef_linear(float y, NoteModifier &mod);

	void setEditPitchMinMax(int pitch_min, int pitch_max);
	int pitch_min, pitch_max;
	int edit_pitch_min, edit_pitch_max;

	bool is_playable();


	float clef_dy;
	float clef_y0;


	void setSolo(bool solo);
	bool solo;


	bool mouse_over();
};

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
