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

class AudioView : public HuiEventHandler, public Observer, public Observable
{
public:
	AudioView(CHuiWindow *parent, AudioFile *audio_1, AudioFile *audio_2);
	virtual ~AudioView();

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

	void OnSelectNone();
	void OnSelectAll();
	void OnSelectNothing();

	void OnViewTempFile();
	void OnViewMono();
	void OnViewGrid();
	void OnViewOptimal();
	void OnZoomIn();
	void OnZoomOut();
	void OnJumpOtherFile();

	void DrawBuffer(HuiDrawingContext *c, int x, int y, int width, int height, Track *t, int pos, float zoom, const color &col);
	void DrawSubFrame(HuiDrawingContext *c, int x, int y, int width, int height, Track *s, AudioFile *a, const color &col, int delay);
	void DrawSub(HuiDrawingContext *c, int x, int y, int width, int height, Track *s, AudioFile *a);
	void DrawBarCollection(HuiDrawingContext *c, int x, int y, int width, int height, Track *t, color col, AudioFile *a, int track_no, BarCollection *bc);
	void DrawTrack(HuiDrawingContext *c, int x, int y, int width, int height, Track *t, color col, AudioFile *a, int track_no);
	void DrawGrid(HuiDrawingContext *c, int x, int y, int width, int height, AudioFile *a, const color &bg, bool show_time = false);
	void DrawWaveFile(HuiDrawingContext *c, int x, int y, int width, int height, AudioFile *a);

	void OptimizeView(AudioFile *a);
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
		SEL_TYPE_AUDIO,
		SEL_TYPE_TIME,
		SEL_TYPE_TRACK,
		SEL_TYPE_SUB
	};

	struct SelectionType
	{
		int type;
		AudioFile *audio;
		Track *track, *sub;
		int pos;
		int sub_offset;
		Array<int> barrier;
	};

	SelectionType Selection;
	ActionSubTrackMove *cur_action;

	int MousePossiblySelecting, MousePossiblySelectingStart;
	const int BarrierDist;
	float ScrollSpeed;
	float ScrollSpeedFast;
	float ZoomSpeed;

	int mx,my;
	int mx0,my0;

	void SelectNone(AudioFile *a);
	void SelectAll(AudioFile *a);

	void SetMouse();
	void ClearMouseOver(AudioFile *a);
	bool MouseOverAudio(AudioFile *a);
	bool MouseOverTrack(Track *t);
	int MouseOverSub(Track *s);
	void SelectionUpdatePos(SelectionType &s);
	SelectionType GetMouseOver(bool set = true);
	void SelectUnderMouse();
	void SetBarriers(AudioFile *a, SelectionType *s);
	void ApplyBarriers(int &pos);
	SelectionType mo_old;

	void SelectSub(Track *s, bool diff);
	void SelectTrack(Track *t, bool diff);

	void SetCurSub(AudioFile *a, Track *s);
	void SetCurTrack(AudioFile *a, Track *t);
	void SetCurAudioFile(AudioFile *a);



	void ZoomAudioFile(AudioFile *a, float f);
	void MoveView(AudioFile *a, float dpos);

	void ExecuteSubDialog(CHuiWindow *win, Track *s);
	void ExecuteTrackDialog(CHuiWindow *win, Track *t);
	void ExecuteAudioDialog(CHuiWindow *win, AudioFile *a);


	bool force_redraw;

	bool ShowTempFile;
	bool ShowMono;
	bool ShowGrid;
	int DetailSteps;
	int MouseMinMoveToSelect;
	int PreviewSleepTime;

	int DrawingWidth;

	AudioFile *audio[2];
};

#endif /* AUDIOVIEW_H_ */
