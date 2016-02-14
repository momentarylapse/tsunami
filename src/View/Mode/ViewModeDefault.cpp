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
#include "../../Audio/AudioStream.h"
#include "../../Action/Track/Sample/ActionTrackMoveSample.h"
#include "../../Audio/AudioStream.h"
#include "../../Audio/Renderer/SongRenderer.h"
#include "math.h"

ViewModeDefault::ViewModeDefault(AudioView *view) :
	ViewMode(view)
{
	cur_action = NULL;
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
		foreach(Track *t, song->tracks)
			t->is_selected = (t == selection->track);
		if (selection->track->muted)
			selection->track->setMuted(false);
	}else if (selection->type == Selection::TYPE_SAMPLE){
		cur_action = new ActionTrackMoveSample(view->song);
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
	else if ((selection->type == Selection::TYPE_TRACK) or (selection->type == Selection::TYPE_TRACK_HANDLE)){
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
		if (hover->allowAutoScroll()){
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
			mo.track->is_selected = true;

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
		view->sel_raw.num = selection->pos - mouse_possibly_selecting_start;
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
	cam->zoom(exp(view->ZoomSpeed * HuiGetEvent()->dz));
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
	foreach(AudioViewTrack *t, view->vtrack){
		t->height_min = view->TIME_SCALE_HEIGHT * 2;
		if (t->track->type == Track::TYPE_AUDIO)
			t->height_wish = view->MAX_TRACK_CHANNEL_HEIGHT * n_ch;
		else if (t->track->type == Track::TYPE_MIDI)
			t->height_wish = view->MAX_TRACK_CHANNEL_HEIGHT;
		else
			t->height_wish =view->TIME_SCALE_HEIGHT * 2;
	}
}



void ViewModeDefault::drawGridBars(HuiPainter *c, const rect &r, const color &bg, bool show_time)
{
	if (song->bars.num == 0)
		return;
	int s0 = cam->screen2sample(r.x1 - 1);
	int s1 = cam->screen2sample(r.x2);
	//c->SetLineWidth(2.0f);
	Array<float> dash, no_dash;
	dash.add(6);
	dash.add(4);
	//Array<Beat> beats = t->bar.GetBeats(Range(s0, s1 - s0));
	Array<Bar> bars = song->bars.getBars(Range(s0, s1 - s0));
	foreach(Bar &b, bars){
		int xx = cam->sample2screen(b.range.offset);

		float dx_bar = cam->dsample2screen(b.range.num);
		float dx_beat = dx_bar / b.num_beats;
		float f1 = min(1.0f, dx_bar / 40.0f);
		if ((b.index % 5) == 0)
			f1 = 1;
		float f2 = min(1.0f, dx_beat / 25.0f);

		if (f1 >= 0.1f){
			c->setColor(ColorInterpolate(bg, view->colors.text_soft1, f1));
			c->setLineDash(no_dash, r.y1);
			c->drawLine(xx, r.y1, xx, r.y2);
		}

		if (f2 >= 0.1f){
			color c1 = ColorInterpolate(bg, view->colors.text_soft1, f2);
			float beat_length = (float)b.range.num / (float)b.num_beats;
			c->setLineDash(dash, r.y1);
			for (int i=0; i<b.num_beats; i++){
				float beat_offset = b.range.offset + (float)i * beat_length;
				if (i == 0)
					continue;
				c->setColor(c1);
				int x = cam->sample2screen(beat_offset);
				c->drawLine(x, r.y1, x, r.y2);
			}
		}

		if ((show_time) and (f1 > 0.9f)){
			c->setColor(view->colors.text_soft1);
			c->drawStr(xx + 2, r.y1, i2s(b.index + 1));
		}
	}
	c->setLineDash(no_dash, 0);
	c->setLineWidth(view->LINE_WIDTH);
}

void ViewModeDefault::drawTrackBackground(HuiPainter *c, AudioViewTrack *t)
{
	color cc = (t->track->is_selected) ? view->colors.background_track_selected : view->colors.background_track;
	c->setColor(cc);
	c->drawRect(t->area);

	view->drawGridTime(c, t->area, cc, false);
	drawGridBars(c, t->area, cc, (t->track->type == Track::TYPE_TIME));
}

void ViewModeDefault::drawTrackData(HuiPainter *c, AudioViewTrack *t)
{
	// midi
	if ((t->track->type == Track::TYPE_MIDI) or (t->track->midi.num > 0))
		t->drawMidi(c, t->track->midi, 0);

	// audio buffer
	t->drawTrackBuffers(c, view->cam.pos);

	// samples
	foreach(SampleRef *s, t->track->samples)
		t->drawSample(c, s);

	// marker
	t->marker_areas.resize(t->track->markers.num);
	foreachi(TrackMarker &m, t->track->markers, i)
		if (!m.text.match(":*:"))
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

	foreach(Track *t, song->tracks){
		// add subs
		foreach(SampleRef *sam, t->samples){
			s->barrier.add(sam->pos + dpos);
		}

		// time bar...
		Array<Beat> beats = song->bars.getBeats(cam->range());
		foreach(Beat &b, beats)
			s->barrier.add(b.range.offset);
	}

	// selection marker
	if (!view->sel_range.empty()){
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
	foreach(int b, selection->barrier){
		int dist = abs(cam->sample2screen(b) - cam->sample2screen(pos));
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
		foreach(SampleRef *ss, s.track->samples){
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
			song->unselectAllSamples();
	}

	// sub
	view->setCurSample(s);
	if (selection->type == Selection::TYPE_SAMPLE){
		view->selectSample(s, control);
	}
}
