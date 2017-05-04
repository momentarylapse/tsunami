/*
 * ViewModeDefault.cpp
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#include "ViewModeDefault.h"
#include "../AudioView.h"
#include "../AudioViewTrack.h"
#include "../SideBar/SideBar.h"
#include "../../TsunamiWindow.h"
#include "../../Device/OutputStream.h"
#include "../../Action/Track/Sample/ActionTrackMoveSample.h"
#include "../../Audio/Renderer/SongRenderer.h"
#include "math.h"

ViewModeDefault::ViewModeDefault(AudioView *view) :
	ViewMode(view)
{
	cur_action = NULL;
	mouse_possibly_selecting_start = -1;
	mouse_possibly_selecting = -1;
}

ViewModeDefault::~ViewModeDefault()
{
}


void ViewModeDefault::onLeftButtonDown()
{
	selectUnderMouse();

	setBarriers(selection);

	applyBarriers(selection->pos);
	mouse_possibly_selecting_start = selection->pos;

	// selection:
	//   start after lb down and moving
	if ((selection->type == Selection::TYPE_TRACK) or (selection->type == Selection::TYPE_TIME)){
		setCursorPos(selection->pos);
	}else if (selection->type == Selection::TYPE_SELECTION_START){
		// swap end / start
		selection->type = Selection::TYPE_SELECTION_END;
		hover->type = Selection::TYPE_SELECTION_END;
		view->sel_raw.invert();
	}else if (selection->type == Selection::TYPE_MUTE){
		selection->track->setMuted(!selection->track->muted);
	}else if (selection->type == Selection::TYPE_SOLO){
		for (Track *t: song->tracks)
			view->sel.set(t, (t == selection->track));
		if (selection->track->muted)
			selection->track->setMuted(false);
	}else if (selection->type == Selection::TYPE_SAMPLE){
		cur_action = new ActionTrackMoveSample(view->song, view->sel);
	}
}

void ViewModeDefault::onLeftButtonUp()
{
	if (selection->type == Selection::TYPE_SAMPLE){
		if (cur_action)
			song->execute(cur_action);
	}
	cur_action = NULL;
}

void ViewModeDefault::onLeftDoubleClick()
{
	selectUnderMouse();

	if (mouse_possibly_selecting < view->mouse_min_move_to_select){
		if (selection->type == Selection::TYPE_SAMPLE){
			win->side_bar->open(SideBar::SAMPLEREF_CONSOLE);
		}else if ((selection->type == Selection::TYPE_TRACK) or (selection->type == Selection::TYPE_TRACK_HANDLE) or ((selection->track) and ((selection->type == Selection::TYPE_SELECTION_START) or (selection->type == Selection::TYPE_SELECTION_END)))){
			win->side_bar->open(SideBar::TRACK_CONSOLE);
		}else if (!selection->track){
			win->side_bar->open(SideBar::SONG_CONSOLE);
		}
		selection->type = Selection::TYPE_NONE;
	}
}

void ViewModeDefault::onRightButtonDown()
{
	selectUnderMouse();

	// pop up menu...
	view->updateMenu();

	if (selection->type == Selection::TYPE_SAMPLE)
		view->menu_sample->openPopup(view->win, 0, 0);
	else if (selection->type == Selection::TYPE_MARKER)
		view->menu_marker->openPopup(view->win, 0, 0);
	else if ((selection->type == Selection::TYPE_TRACK) or (selection->type == Selection::TYPE_TRACK_HANDLE) or (selection->type == Selection::TYPE_SELECTION_START) or (selection->type == Selection::TYPE_SELECTION_END)){
		view->menu_track->enable("track_edit_midi", view->cur_track->type == Track::TYPE_MIDI);
		view->menu_track->openPopup(view->win, 0, 0);
	}else if (!selection->track)
		view->menu_song->openPopup(view->win, 0, 0);
}

void ViewModeDefault::onRightButtonUp()
{
}

void ViewModeDefault::onMouseMove()
{
	bool _force_redraw_ = false;

	if (HuiGetEvent()->lbut){

		// cheap auto scrolling
		if (hover->allow_auto_scroll()){
			if (view->mx < 50)
				cam->move(-10 / cam->scale);
			if (view->mx > view->area.width() - 50)
				cam->move(10 / cam->scale);
		}

		view->selectionUpdatePos(view->selection);
	}else{
		Selection hover_old = *hover;
		*hover = getHover();
		_force_redraw_ |= hover_changed(*hover, hover_old);
	}


	// drag & drop
	if (selection->type == Selection::TYPE_SELECTION_END){

		Selection mo = getHover();
		if (mo.track)
			view->sel.add(mo.track);

		applyBarriers(selection->pos);
		view->sel_raw.set_end(selection->pos);
		view->updateSelection();
		//_force_redraw_ = true;
		_force_redraw_ = false;
		int x, w;
		int r = 4;
		if (HuiGetEvent()->dx < 0){
			x = view->mx - r;
			w = - HuiGetEvent()->dx + 2*r;
		}else{
			x = view->mx + r;
			w = - HuiGetEvent()->dx - 2*r;
		}
		win->redrawRect("area", x, view->area.y1, w, view->area.height());
	}else if (selection->type == Selection::TYPE_PLAYBACK){
		view->stream->seek(selection->pos);
		_force_redraw_ = true;
	}else if (selection->type == Selection::TYPE_SAMPLE){
		applyBarriers(selection->pos);
		int dpos = (float)selection->pos - selection->sample_offset - selection->sample->pos;
		if (cur_action)
			cur_action->set_param_and_notify(view->song, dpos);
		_force_redraw_ = true;
	}

	// selection:
	if (!HuiGetEvent()->lbut){
		mouse_possibly_selecting = -1;
	}
	if (mouse_possibly_selecting >= 0)
		mouse_possibly_selecting += fabs(HuiGetEvent()->dx);
	if (mouse_possibly_selecting > view->mouse_min_move_to_select){
		view->sel_raw.offset = mouse_possibly_selecting_start;
		view->sel_raw.length = selection->pos - mouse_possibly_selecting_start;
		setBarriers(selection);
		view->updateSelection();
		selection->type = Selection::TYPE_SELECTION_END;
		hover->type = Selection::TYPE_SELECTION_END;
		_force_redraw_ = true;
		mouse_possibly_selecting = -1;
	}

	if (_force_redraw_)
		view->forceRedraw();
}

void ViewModeDefault::onMouseWheel()
{
	HuiEvent *e = HuiGetEvent();
	if (fabs(e->scroll_y) > 0.1f)
		cam->zoom(exp(e->scroll_y * view->ZoomSpeed * 0.3f));
	if (fabs(e->scroll_x) > 0.1f)
		cam->move(e->scroll_x / cam->scale * view->ScrollSpeed * 0.03f);
}

void ViewModeDefault::onKeyDown(int k)
{

// view
	// moving
	float dt = 0.05f;
	if (k == KEY_RIGHT)
		cam->move(view->ScrollSpeed * dt / cam->scale);
	if (k == KEY_LEFT)
		cam->move(- view->ScrollSpeed * dt / cam->scale);
	if (k == KEY_NEXT)
		cam->move(view->ScrollSpeedFast * dt / cam->scale);
	if (k == KEY_PRIOR)
		cam->move(- view->ScrollSpeedFast * dt / cam->scale);
	// zoom
	if (k == KEY_ADD)
		cam->zoom(exp(  view->ZoomSpeed));
	if (k == KEY_SUBTRACT)
		cam->zoom(exp(- view->ZoomSpeed));

	if (k == KEY_SPACE){
		if (view->stream->isPlaying()){
			view->stream->pause();
		}else{
			win->onPlay();
		}
	}
	view->updateMenu();
}

void ViewModeDefault::onKeyUp(int k)
{
}

void ViewModeDefault::updateTrackHeights()
{
	int n_ch = 1;
	for (AudioViewTrack *t: view->vtrack){
		t->height_min = view->TIME_SCALE_HEIGHT * 2;
		if (t->track->type == Track::TYPE_AUDIO)
			t->height_wish = view->MAX_TRACK_CHANNEL_HEIGHT * n_ch;
		else if (t->track->type == Track::TYPE_MIDI)
			t->height_wish = view->MAX_TRACK_CHANNEL_HEIGHT;
		else
			t->height_wish =view->TIME_SCALE_HEIGHT * 2;
	}
}



void ViewModeDefault::drawMidi(Painter *c, AudioViewTrack *t, const MidiData &midi, bool as_reference, int shift)
{
	int mode = which_midi_mode(t->track);
	if (mode == view->MIDI_MODE_LINEAR)
		t->drawMidiLinear(c, midi, as_reference, shift);
	else if (mode == view->MIDI_MODE_TAB)
		t->drawMidiTab(c, midi, as_reference, shift);
	else // if (mode == view->VIEW_MIDI_CLASSICAL)
		t->drawMidiClassical(c, midi, as_reference, shift);
}

void ViewModeDefault::drawTrackBackground(Painter *c, AudioViewTrack *t)
{
	t->drawBlankBackground(c);

	color cc = t->getBackgroundColor();
	view->drawGridTime(c, t->area, cc, false);
	t->drawGridBars(c, cc, (t->track->type == Track::TYPE_TIME), 0);
}

void ViewModeDefault::drawTrackData(Painter *c, AudioViewTrack *t)
{
	// midi
	if (t->track->type == Track::TYPE_MIDI)
		drawMidi(c, t, t->track->midi, false, 0);

	// audio buffer
	t->drawTrackBuffers(c, view->cam.pos);

	// samples
	for (SampleRef *s: t->track->samples)
		t->drawSample(c, s);

	// marker
	t->marker_areas.resize(t->track->markers.num);
	foreachi(TrackMarker *m, t->track->markers, i)
		t->drawMarker(c, m, i, (view->hover.type == Selection::TYPE_MARKER) and (view->hover.track == t->track) and (view->hover.index == i));
}

void ViewModeDefault::setBarriers(Selection *s)
{
	s->barrier.clear();
	if (s->type == s->TYPE_NONE)
		return;

	int dpos = 0;
	if (s->type == s->TYPE_SAMPLE)
		dpos = s->sample_offset;

	for (Track *t: song->tracks){
		// add subs
		for (SampleRef *sam: t->samples){
			s->barrier.add(sam->pos + dpos);
		}

		// time bar...
		Array<Beat> beats = song->bars.getBeats(cam->range(), true);
		for (Beat &b: beats)
			s->barrier.add(b.range.offset);
	}

	// selection marker
	if (!view->sel.range.empty()){
		s->barrier.add(view->sel_raw.start());
		if (mouse_possibly_selecting < 0)
			s->barrier.add(view->sel_raw.end());
	}
}

void ViewModeDefault::applyBarriers(int &pos)
{
	int dmin = view->BARRIER_DIST;
	bool found = false;
	int new_pos;
	for (int b: selection->barrier){
		int dist = fabs(cam->sample2screen(b) - cam->sample2screen(pos));
		if (dist < dmin){
			//msg_write(format("barrier:  %d  ->  %d", pos, b));
			new_pos = b;
			found = true;
			dmin = dist;
		}
	}
	if (found)
		pos = new_pos;
}

Selection ViewModeDefault::getHover()
{
	Selection s;
	int mx = view->mx;
	int my = view->my;

	// track?
	foreachi(AudioViewTrack *t, view->vtrack, i){
		if (view->mouseOverTrack(t)){
			s.vtrack = t;
			s.index = i;
			s.track = t->track;
			s.type = Selection::TYPE_TRACK;
			if (view->mx < t->area.x1 + view->TRACK_HANDLE_WIDTH)
				s.show_track_controls = t->track;
		}
	}

	// selection boundaries?
	view->selectionUpdatePos(s);
	if ((my <= view->TIME_SCALE_HEIGHT) or (view->win->getKey(KEY_SHIFT))){
		if (view->mouse_over_time(view->sel_raw.end())){
			s.type = Selection::TYPE_SELECTION_END;
			return s;
		}
		if (view->mouse_over_time(view->sel_raw.start())){
			s.type = Selection::TYPE_SELECTION_START;
			return s;
		}
		if (view->stream->isPlaying()){
			if (view->mouse_over_time(view->stream->getPos())){
				s.type = Selection::TYPE_PLAYBACK;
				return s;
			}
		}
	}

	// mute button?
	if (s.track){
		AudioViewTrack *t = s.vtrack;
		if ((mx >= t->area.x1 + 5) and (mx < t->area.x1 + 17) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34)){
			s.type = Selection::TYPE_MUTE;
			return s;
		}
		if ((song->tracks.num > 1) and (mx >= t->area.x1 + 22) and (mx < t->area.x1 + 34) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34)){
			s.type = Selection::TYPE_SOLO;
			return s;
		}
	}

	// sub?
	if (s.track){

		// markers
		for (int i=0; i<min(s.track->markers.num, view->vtrack[s.index]->marker_areas.num); i++){
			if (view->vtrack[s.index]->marker_areas[i].inside(mx, my)){
				s.type = Selection::TYPE_MARKER;
				s.index = i;
				return s;
			}
		}

		// TODO: prefer selected subs
		for (SampleRef *ss: s.track->samples){
			int offset = view->mouseOverSample(ss);
			if (offset >= 0){
				s.sample = ss;
				s.type = Selection::TYPE_SAMPLE;
				s.sample_offset = offset;
				return s;
			}
		}
	}

	// time scale
	if (my < view->TIME_SCALE_HEIGHT){
		s.type = Selection::TYPE_TIME;
		return s;
	}

	// track handle
	if ((s.track) and (mx < view->area.x1 + view->TRACK_HANDLE_WIDTH)){
		s.type = Selection::TYPE_TRACK_HANDLE;
		return s;
	}

	return s;
}

void ViewModeDefault::setCursorPos(int pos)
{
	if (view->stream->isPlaying()){
		if (view->renderer->range().is_inside(pos)){
			view->stream->seek(pos);
			selection->type = Selection::TYPE_PLAYBACK;
			view->forceRedraw();
			return;
		}else{
			view->stream->stop();
			/*view->stream->seek(pos);
			selection->type = Selection::TYPE_PLAYBACK;
			view->forceRedraw();
			return;*/
		}
	}
	mouse_possibly_selecting = 0;
	view->sel_raw = Range(pos, 0);
	view->updateSelection();
}

void ViewModeDefault::selectUnderMouse()
{
	*hover = getHover();
	*selection = *hover;
	Track *t = selection->track;
	SampleRef *s = selection->sample;
	bool control = win->getKey(KEY_CONTROL);

	// track
	if (selection->track)
		view->setCurTrack(selection->track);
	if ((selection->type == Selection::TYPE_TRACK) or (selection->type == Selection::TYPE_TRACK_HANDLE)){
		view->selectTrack(t, control);
		if (!control)
			view->unselectAllSamples();
	}

	// sub
	view->setCurSample(s);
	if (selection->type == Selection::TYPE_SAMPLE){
		view->selectSample(s, control);
	}
}

int ViewModeDefault::which_midi_mode(Track *t)
{
	if (view->midi_view_mode == view->MIDI_MODE_LINEAR)
			return view->MIDI_MODE_LINEAR;
	if ((view->midi_view_mode == view->MIDI_MODE_TAB) and (t->instrument.string_pitch.num > 0))
		return view->MIDI_MODE_TAB;
	return view->MIDI_MODE_CLASSICAL;
}
