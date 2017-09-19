/*
 * AudioView.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef AUDIOVIEW_H_
#define AUDIOVIEW_H_

#include "../Data/Song.h"
#include "../Data/SongSelection.h"
#include "../Midi/Scale.h"
#include "../lib/hui/hui.h"
#include "../Stuff/Observable.h"
#include "TrackHeightManager.h"
#include "ViewPort.h"
#include "Selection.h"
#include "ColorScheme.h"

class DeviceManager;
class OutputStream;
class SongRenderer;
class TsunamiWindow;
class AudioViewTrack;
class PeakThread;
class ViewMode;
class ViewModeDefault;
class ViewModeMidi;
class ViewModeScaleBars;
class ViewModeCurve;
class ViewModeCapture;

class AudioView : public Observable<VirtualBase>
{
public:
	AudioView(TsunamiWindow *parent, const string &id, Song *audio);
	virtual ~AudioView();

	void checkConsistency();
	void forceRedraw();

	void onDraw(Painter *p);
	void onMouseMove();
	void onLeftButtonDown();
	void onLeftButtonUp();
	void onMiddleButtonDown();
	void onMiddleButtonUp();
	void onRightButtonDown();
	void onRightButtonUp();
	void onLeftDoubleClick();
	void onMouseWheel();
	void onKeyDown();
	void onKeyUp();
	void onCommand(const string &id);

	void onSongUpdate();
	void onStreamUpdate();
	void onUpdate();
	static const string MESSAGE_CUR_TRACK_CHANGE;
	static const string MESSAGE_CUR_SAMPLE_CHANGE;
	static const string MESSAGE_CUR_LAYER_CHANGE;
	static const string MESSAGE_SELECTION_CHANGE;
	static const string MESSAGE_SETTINGS_CHANGE;
	static const string MESSAGE_VIEW_CHANGE;
	static const string MESSAGE_VTRACK_CHANGE;
	static const string MESSAGE_INPUT_CHANGE;

	void updatePeaks();
	void zoomIn();
	void zoomOut();

	void drawGridTime(Painter *c, const rect &r, const color &bg, bool show_time = false);
	void drawTimeLine(Painter *c, int pos, int type, const color &col, bool show_time = false);
	void drawSelection(Painter *c, const rect &r);
	void drawBackground(Painter *c, const rect &r);
	void drawAudioFile(Painter *c, const rect &r);

	void drawBoxedStr(Painter *c, float x, float y, const string &str, const color &col_text, const color &col_bg);

	void optimizeView();
	void updateMenu();

	string id;

	Array<ColorSchemeBasic> basic_schemes;
	static ColorSchemeBasic basic_colors;
	static ColorScheme _export_colors;
	ColorScheme colors;
	void setColorScheme(const string &name);

	static const int SAMPLE_FRAME_HEIGHT;
	static const int TIME_SCALE_HEIGHT;
	static const float LINE_WIDTH;
	static const float CORNER_RADIUS;
	static const int FONT_SIZE;
	static const int MAX_TRACK_CHANNEL_HEIGHT;
	static const int TRACK_HANDLE_WIDTH;
	static const int TRACK_HANDLE_HEIGHT;
	static const int TRACK_HANDLE_HEIGHT_SMALL;
	static const int BARRIER_DIST;

	Selection hover;
	SongSelection sel;
	SongSelection sel_temp;

	enum SelectionMode{
		SELECTION_MODE_NONE,
		SELECTION_MODE_TIME,
		SELECTION_MODE_RECT,
		SELECTION_MODE_TRACK_RECT,
		SELECTION_MODE_FAKE,
	};
	SelectionMode selection_mode;
	bool hide_selection;


	void applyBarriers(int &pos);

	void _cdecl unselectAllSamples();

	bool enabled;
	void enable(bool enabled);

	float ScrollSpeed;
	float ScrollSpeedFast;
	float ZoomSpeed;

	int mx, my;

	struct MouseSelectionPlanner
	{
		float dist;
		int start_pos;
		int start_y;
		int min_move_to_select;
		void start(int pos, int y);
		bool step();
		bool selecting();
		void stop();
	}msp;

	void selectNone();
	void selectAll();
	void selectExpand();
	void updateSelection();
	void setSelection(const SongSelection &s);
	Range getPlaybackSelection();

	void setMouse();
	bool mouseOverTrack(AudioViewTrack *t);
	int mouseOverSample(SampleRef *s);
	void selectionUpdatePos(Selection &s);
	bool mouse_over_time(int pos);

	void selectSample(SampleRef *s, bool diff);
	void selectTrack(Track *t, bool diff);


	bool force_redraw;

	int detail_steps;
	int preview_sleep_time;
	bool antialiasing;


	void setMidiViewMode(int mode);
	int midi_view_mode;
	enum{
		MIDI_MODE_LINEAR,
		MIDI_MODE_TAB,
		MIDI_MODE_CLASSICAL,
		MIDI_MODE_DRUM
	};

	ViewMode *mode;
	void setMode(ViewMode *m);
	ViewModeDefault *mode_default;
	ViewModeMidi *mode_midi;
	ViewModeScaleBars *mode_scale_bars;
	ViewModeCurve *mode_curve;
	ViewModeCapture *mode_capture;

	TsunamiWindow *win;

	Song *song;

	OutputStream *stream;
	SongRenderer *renderer;

	void setCurSample(SampleRef *s);
	void setCurTrack(Track *t);
	void setCurLayer(int l);
	Track *cur_track;
	SampleRef *cur_sample;
	int cur_layer;

	bool editingTrack(Track *t);


	void setScale(const Scale &s);
	Scale midi_scale;

	bool bars_edit_data;

	rect area;
	TrackHeightManager thm;

	ViewPort cam;

	Array<AudioViewTrack*> vtrack;
	AudioViewTrack *dummy_vtrack;
	AudioViewTrack *get_track(Track *track);
	void updateTracks();

	int prefered_buffer_layer;
	double buffer_zoom_factor;
	void updateBufferZoom();

	PeakThread *peak_thread;
	bool is_updating_peaks;

	struct ImageData
	{
		Image *speaker, *x, *solo;
		Image *speaker_bg, *x_bg, *solo_bg;
		Image *track_audio, *track_time, *track_midi;
		Image *track_audio_bg, *track_time_bg, *track_midi_bg;
	};
	ImageData images;

	hui::Menu *menu_track;
	hui::Menu *menu_sample;
	hui::Menu *menu_marker;
	hui::Menu *menu_song;

	int perf_channel;
};

#endif /* AUDIOVIEW_H_ */
