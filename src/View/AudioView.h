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

class AudioView : public HuiEventHandler, public Observer
{
public:
	AudioView();
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


	void DrawBuffer(HuiDrawingContext *c, int x, int y, int width, int height, Track *t, int pos, float zoom, const color &col);
	void DrawSubFrame(HuiDrawingContext *c, int x, int y, int width, int height, Track *s, AudioFile *a, const color &col, int delay);
	void DrawSub(HuiDrawingContext *c, int x, int y, int width, int height, Track *s, AudioFile *a);
	void DrawBarCollection(HuiDrawingContext *c, int x, int y, int width, int height, Track *t, color col, AudioFile *a, int track_no, BarCollection *bc);
	void DrawTrack(HuiDrawingContext *c, int x, int y, int width, int height, Track *t, color col, AudioFile *a, int track_no);
	void DrawGrid(HuiDrawingContext *c, int x, int y, int width, int height, AudioFile *a, const color &bg, bool show_time = false);
	void DrawWaveFile(HuiDrawingContext *c, int x, int y, int width, int height, AudioFile *a);


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



	bool force_redraw;

	bool ShowTempFile;
	bool ShowMono;
	bool ShowGrid;
	int DetailSteps;
	int MouseMinMoveToSelect;
	int PreviewSleepTime;

	int DrawingWidth;
};

#endif /* AUDIOVIEW_H_ */
