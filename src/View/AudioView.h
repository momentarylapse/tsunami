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
#include "../Action/SubTrack/ActionSubTrackMove.h"

class TrackDialog;
class AudioFileDialog;

class AudioView : public HuiEventHandler, public Observer, public Observable
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

	void SetShowMono(bool mono);
	void SetGridMode(int mode);
	void SetPeaksMode(int mode);
	void ZoomIn();
	void ZoomOut();

	void DrawBuffer(HuiDrawingContext *c, const rect &r, Track *t, double pos, const color &col);
	void DrawSubFrame(HuiDrawingContext *c, const rect &r, Track *s, const color &col, int delay);
	void DrawSub(HuiDrawingContext *c, const rect &r, Track *s);
	void DrawMidi(HuiDrawingContext *c, const rect &r, MidiData &midi, color col);
	void DrawTrack(HuiDrawingContext *c, const rect &r, Track *t, color col, int track_no);
	void DrawGrid(HuiDrawingContext *c, const rect &r, const color &bg, bool show_time = false);
	void DrawGridTime(HuiDrawingContext *c, const rect &r, const color &bg, bool show_time = false);
	void DrawGridBars(HuiDrawingContext *c, const rect &r, const color &bg, bool show_time = false);
	void DrawTimeLine(HuiDrawingContext *c, int pos, int type, color &col, bool show_time = false);
	void DrawSelection(HuiDrawingContext *c, const rect &r);
	void DrawBackground(HuiDrawingContext *c, const rect &r);
	void DrawEmptyAudioFile(HuiDrawingContext *c, const rect &r);
	void DrawAudioFile(HuiDrawingContext *c, const rect &r);

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
		SEL_TYPE_SUB
	};

	struct SelectionType
	{
		int type;
		Track *track;
		Track *sub;
		int pos;
		int sub_offset;
		Array<int> barrier;

		SelectionType();
	};

	SelectionType Hover, Selection;
	ActionSubTrackMove *cur_action;

	int MousePossiblySelecting, MousePossiblySelectingStart;
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
	int MouseOverSub(Track *s);
	void SelectionUpdatePos(SelectionType &s);
	SelectionType GetMouseOver();
	void SelectUnderMouse();
	void SetBarriers(SelectionType *s);
	void ApplyBarriers(int &pos);

	void SelectSub(Track *s, bool diff);
	void SelectTrack(Track *t, bool diff);

	void SetCurSub(AudioFile *a, Track *s);
	void SetCurTrack(AudioFile *a, Track *t);

	double screen2sample(double x);
	double sample2screen(double s);

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
	int DetailSteps;
	int MouseMinMoveToSelect;
	int PreviewSleepTime;
	bool Antialiasing;

	int PeakMode;

	int DrawingWidth;

	AudioFile *audio;

	void SetCurSub(Track *s);
	void SetCurTrack(Track *t);
	Track *cur_track;
	Track *cur_sub;
	int cur_level;

	double view_pos;
	double view_zoom;

	TrackDialog *track_dialog;
	AudioFileDialog *audio_file_dialog;

	Image image_muted, image_unmuted, image_solo;
};

#endif /* AUDIOVIEW_H_ */
