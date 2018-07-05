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
#include "../Data/Midi/Scale.h"
#include "../lib/hui/hui.h"
#include "../Stuff/Observable.h"
#include "TrackHeightManager.h"
#include "ViewPort.h"
#include "Selection.h"
#include "ColorScheme.h"

class DeviceManager;
class OutputStream;
class SongRenderer;
class PeakMeter;
class TsunamiWindow;
class AudioViewTrack;
class AudioViewLayer;
class PeakThread;
class ViewMode;
class ViewModeDefault;
class ViewModeMidi;
class ViewModeScaleBars;
class ViewModeCurve;
class ViewModeCapture;
class Session;

class AudioView : public Observable<VirtualBase>
{
public:
	AudioView(Session *session, const string &id);
	virtual ~AudioView();

	void checkConsistency();
	void forceRedraw();
	void forceRedrawPart(const rect &r);

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
	void onStreamStateChange();
	void onStreamEndOfStream();
	void onUpdate();
	static const string MESSAGE_CUR_TRACK_CHANGE;
	static const string MESSAGE_CUR_SAMPLE_CHANGE;
	static const string MESSAGE_CUR_LAYER_CHANGE;
	static const string MESSAGE_SELECTION_CHANGE;
	static const string MESSAGE_SETTINGS_CHANGE;
	static const string MESSAGE_VIEW_CHANGE;
	static const string MESSAGE_VTRACK_CHANGE;
	static const string MESSAGE_INPUT_CHANGE;
	static const string MESSAGE_OUTPUT_STATE_CHANGE;

	void updatePeaks();
	void zoomIn();
	void zoomOut();

	void drawGridTime(Painter *c, const rect &r, const color &bg, bool show_time = false);
	void drawTimeLine(Painter *c, int pos, int type, const color &col, bool show_time = false);
	void drawSelection(Painter *c);
	void drawBackground(Painter *c);
	void drawAudioFile(Painter *c);

	rect getBoxedStrRect(Painter *c, float x, float y, const string &str);
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
		NONE,
		TIME,
		RECT,
		TRACK_RECT,
		FAKE,
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
	float mouse_wheel_speed;

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
	Range getPlaybackSelection(bool for_recording);

	void setMouse();
	bool mouseOverTrack(AudioViewTrack *t);
	bool mouseOverLayer(AudioViewLayer *l);
	int mouseOverSample(SampleRef *s);
	void selectionUpdatePos(Selection &s);
	bool mouse_over_time(int pos);

	void selectSample(SampleRef *s, bool diff);

	int detail_steps;
	int preview_sleep_time;
	bool antialiasing;


	void setMidiViewMode(int mode);
	int midi_view_mode;
	enum MidiMode{
		LINEAR,
		TAB,
		CLASSICAL,
		DRUM
	};

	ViewMode *mode;
	void setMode(ViewMode *m);
	ViewModeDefault *mode_default;
	ViewModeMidi *mode_midi;
	ViewModeScaleBars *mode_scale_bars;
	ViewModeCurve *mode_curve;
	ViewModeCapture *mode_capture;

	Session *session;
	TsunamiWindow *win;

	Song *song;

	OutputStream *stream;
	bool playback_active;
	SongRenderer *renderer;
	PeakMeter *peak_meter;
	void play(const Range &range, bool allow_loop);
	void stop();
	void pause(bool pause);
	bool isPlaybackActive();
	bool isPaused();
	int playbackPos();
	Set<Track*> get_playable_tracks();
	Set<Track*> get_selected_tracks();
	bool hasAnySolo();

	void setCurSample(SampleRef *s);
	void setCurTrack(Track *t);
	void setCurLayer(TrackLayer *l);
	Track *cur_track;
	SampleRef *cur_sample;
	TrackLayer *cur_layer;

	bool editingTrack(Track *t);


	void setScale(const Scale &s);
	Scale midi_scale;

	bool bars_edit_data;

	rect area;
	rect clip;
	TrackHeightManager thm;

	ViewPort cam;

	Array<AudioViewTrack*> vtrack;
	Array<AudioViewLayer*> vlayer;
	AudioViewTrack *dummy_vtrack;
	AudioViewLayer *dummy_vlayer;
	AudioViewTrack *get_track(Track *track);
	AudioViewLayer *get_layer(TrackLayer *layer);
	void updateTracks();

	void update_peaks_now(AudioBuffer &buf);
	void update_peaks(AudioBuffer &buf);
	void update_peaks(Track *t);
	void update_peaks(Song *s);

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
	hui::Menu *menu_time_track;
	hui::Menu *menu_layer;
	hui::Menu *menu_sample;
	hui::Menu *menu_marker;
	hui::Menu *menu_bar;
	hui::Menu *menu_song;

	int perf_channel;
};

#endif /* AUDIOVIEW_H_ */
