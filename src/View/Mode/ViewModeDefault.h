/*
 * ViewModeDefault.h
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODEDEFAULT_H_
#define SRC_VIEW_MODE_VIEWMODEDEFAULT_H_

#include "ViewMode.h"

class ActionTrackMoveSample;
class Range;
class Track;

class ViewModeDefault : public ViewMode
{
public:
	ViewModeDefault(AudioView *view);
	virtual ~ViewModeDefault();

	void onLeftButtonDown() override;
	void onLeftButtonUp() override;
	void onLeftDoubleClick() override;
	void onRightButtonDown() override;
	void onRightButtonUp() override;
	void onMouseWheel() override;
	void onMouseMove() override;
	void onKeyDown(int k) override;
	void onKeyUp(int k) override;
	void updateTrackHeights() override;


	void drawTrackBackground(Painter *c, AudioViewTrack *t) override;
	void drawLayerBackground(Painter *c, AudioViewLayer *l) override;
	void drawTrackData(Painter *c, AudioViewTrack *t) override;
	void drawLayerData(Painter *c, AudioViewLayer *l) override;
	void drawMidi(Painter *c, AudioViewLayer *l, const MidiNoteBuffer &midi, bool as_reference, int shift) override;
	void drawPost(Painter *c) override;

	int which_midi_mode(Track *t) override;

	SongSelection getSelectionForRange(const Range &r) override;
	SongSelection getSelectionForRect(const Range &r, int y0, int y1) override;
	SongSelection getSelectionForTrackRect(const Range &r, int y0, int y1) override;
	void startSelection() override;

	void selectUnderMouse();
	void setCursorPos(int pos, bool keep_track_selection);
	Selection getHover() override;
	Selection getHoverBasic();

	void setBarriers(Selection &s) override;

	int getTrackMoveTarget(bool visual);

	ActionTrackMoveSample *cur_action;

	Track* moving_track;
};

#endif /* SRC_VIEW_MODE_VIEWMODEDEFAULT_H_ */
