/*
 * ViewMode.h
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODE_H_
#define SRC_VIEW_MODE_VIEWMODE_H_

class AudioView;
class AudioViewTrack;
class Selection;
class SongSelection;
class Track;
class ViewPort;
class TsunamiWindow;
class Song;
class MidiData;
class Painter;
class rect;
class color;
class Range;

class ViewMode
{
public:
	ViewMode(AudioView *view);
	virtual ~ViewMode();

	virtual void onLeftButtonDown(){}
	virtual void onLeftButtonUp(){}
	virtual void onLeftDoubleClick(){}
	virtual void onRightButtonDown(){}
	virtual void onRightButtonUp(){}
	virtual void onMouseMove(){}
	virtual void onMouseWheel(){}
	virtual void onKeyDown(int k){}
	virtual void onKeyUp(int k){}
	virtual void updateTrackHeights(){}
	virtual void onCurTrackChange(){}

	virtual Selection getHover();
	virtual void setBarriers(Selection &s){}

	virtual void drawTrackBackground(Painter *c, AudioViewTrack *t){}
	virtual void drawTrackData(Painter *c, AudioViewTrack *t){}
	virtual void drawPost(Painter *c){}
	virtual void drawMidi(Painter *c, AudioViewTrack *t, const MidiData &midi, bool as_reference, int shift){}

	virtual int which_midi_mode(Track *t) = 0;

	SongSelection getSelection();
	virtual SongSelection getSelectionForRange(const Range &r);
	virtual SongSelection getSelectionForRect(const Range &r, int y0, int y1);
	virtual void startSelection(){}

	AudioView *view;
	ViewPort *cam;
	Selection *hover;
	TsunamiWindow *win;
	Song *song;
};

#endif /* SRC_VIEW_MODE_VIEWMODE_H_ */
