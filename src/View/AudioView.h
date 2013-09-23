/*
 * AudioView.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef AUDIOVIEW_H_
#define AUDIOVIEW_H_

#include "../lib/hui/hui.h"
#include "../Data/AudioFile.h"
#include "../Stuff/Observer.h"

class ActionTrackMoveSample;
class TrackDialog;
class AudioFileDialog;

class AudioView : public Observer, public Observable
{
public:
	AudioView(HuiWindow *parent, AudioFile *audio);
	virtual ~AudioView();

	void CheckConsistency();
	void ForceRedraw();

	void OnDraw();
	void OnMouseMove();
	void OnLeftButtonDown();
	void OnLeftButtonUp();
	void OnMiddleButtonDown();
	void OnMiddleButtonUp();
	void OnRightButtonDown();
	void OnRightButtonUp();
	void OnLeftDoubleClick();
	void OnMouseWheel();
	void OnKeyDown();
	void OnKeyUp();
	void OnCommand(const string &id);

	void OnUpdate(Observable *o);

	void OnCloseEditMidiMode();
	void OnMidiPitch();
	void OnMidiBeatPartition();

	void SetShowMono(bool mono);
	void SetGridMode(int mode);
	void SetPeaksMode(int mode);
	void ZoomIn();
	void ZoomOut();
	void MakeSampleVisible(int sample);

	color GetPitchColor(int pitch);

	void DrawTrackBuffers(HuiPainter *c, const rect &r, Track *t, double pos, const color &col);
	void DrawBuffer(HuiPainter *c, const rect &r, BufferBox &b, double view_pos_rel, const color &col);
	void DrawSampleFrame(HuiPainter *c, const rect &r, SampleRef *s, const color &col, int delay);
	void DrawSample(HuiPainter *c, const rect &r, SampleRef *s);
	void DrawMidi(HuiPainter *c, const rect &r, MidiData &midi, color col);
	void DrawMidiEditable(HuiPainter *c, const rect &r, MidiData &midi, color col);
	void DrawTrack(HuiPainter *c, const rect &r, Track *t, color col, int track_no);
	void DrawGrid(HuiPainter *c, const rect &r, const color &bg, bool show_time = false);
	void DrawGridTime(HuiPainter *c, const rect &r, const color &bg, bool show_time = false);
	void DrawGridBars(HuiPainter *c, const rect &r, const color &bg, bool show_time = false);
	void DrawTimeLine(HuiPainter *c, int pos, int type, color &col, bool show_time = false);
	void DrawSelection(HuiPainter *c, const rect &r);
	void DrawBackground(HuiPainter *c, const rect &r);
	void DrawEmptyAudioFile(HuiPainter *c, const rect &r);
	void DrawAudioFile(HuiPainter *c, const rect &r);

	void OptimizeView();
	void UpdateMenu();


	color ColorBackground;
	color ColorBackgroundCurWave;
	color ColorBackgroundCurTrack;
	color ColorGrid;
	color ColorSelectionInternal;
	color ColorSelectionBoundary;
	color ColorSelectionBoundaryMO;
	color ColorPreviewMarker;
	color ColorCaptureMarker;
	color ColorWave;
	color ColorWaveCur;
	color ColorSub;
	color ColorSubMO;
	color ColorSubNotCur;
	const int SUB_FRAME_HEIGHT;
	const int TIME_SCALE_HEIGHT;

	enum
	{
		SEL_TYPE_NONE,
		SEL_TYPE_SELECTION_START,
		SEL_TYPE_SELECTION_END,
		SEL_TYPE_PLAYBACK_START,
		SEL_TYPE_PLAYBACK_END,
		SEL_TYPE_PLAYBACK,
		SEL_TYPE_TIME,
		SEL_TYPE_TRACK,
		SEL_TYPE_MUTE,
		SEL_TYPE_SOLO,
		SEL_TYPE_SAMPLE,
		SEL_TYPE_MIDI_NOTE,
		SEL_TYPE_MIDI_PITCH,
	};

	struct SelectionType
	{
		int type;
		Track *track;
		SampleRef *sample;
		int pos;
		int sample_offset;
		Array<int> barrier;
		Track *show_track_controls;
		int pitch, note;

		SelectionType();
	};

	SelectionType hover, selection;
	ActionTrackMoveSample *cur_action;

	int mouse_possibly_selecting, mouse_possibly_selecting_start;
	const int BarrierDist;
	float ScrollSpeed;
	float ScrollSpeedFast;
	float ZoomSpeed;

	int mx,my;
	int mx0,my0;

	void SelectNone();
	void SelectAll();

	void SetMouse();
	bool MouseOverTrack(Track *t);
	int MouseOverSample(SampleRef *s);
	void SelectionUpdatePos(SelectionType &s);
	SelectionType GetMouseOver();
	void SelectUnderMouse();
	void SetBarriers(SelectionType *s);
	void ApplyBarriers(int &pos);

	void SelectSample(SampleRef *s, bool diff);
	void SelectTrack(Track *t, bool diff);

	void SetCurSample(AudioFile *a, SampleRef *s);
	void SetCurTrack(AudioFile *a, Track *t);

	double screen2sample(double x);
	double sample2screen(double s);
	int y2pitch(int y);
	float pitch2y(int p);

	void Zoom(float f);
	void Move(float dpos);

	void ExecuteSubDialog(HuiWindow *win);
	void ExecuteTrackDialog(HuiWindow *win);
	void ExecuteAudioDialog(HuiWindow *win);


	bool force_redraw;

	bool show_mono;
	int grid_mode;
	enum{
		GRID_MODE_NONE,
		GRID_MODE_TIME,
		GRID_MODE_BARS
	};
	int detail_steps;
	int mouse_min_move_to_select;
	int preview_sleep_time;
	bool antialiasing;

	int peak_mode;

	int drawing_width;

	int edit_mode;
	enum{
		EDIT_MODE_DEFAULT,
		EDIT_MODE_MIDI
	};
	int pitch_min, pitch_max;
	Track *midi_edit_track;
	void SetEditModeDefault();
	void SetEditModeMidi(Track *t);

	AudioFile *audio;

	void SetCurSample(SampleRef *s);
	void SetCurTrack(Track *t);
	Track *cur_track;
	SampleRef *cur_sample;
	int cur_level;

	double view_pos;
	double view_zoom;

	int prefered_buffer_level;
	double buffer_zoom_factor;
	void UpdateBufferZoom();

	TrackDialog *track_dialog;
	AudioFileDialog *audio_file_dialog;

	Image image_muted, image_unmuted, image_solo;
	Image image_track_audio, image_track_time, image_track_midi;
};

#endif /* AUDIOVIEW_H_ */
