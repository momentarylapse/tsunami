/*
 * AudioView.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "AudioView.h"
#include "../Tsunami.h"
#include "../View/Dialog/AudioFileDialog.h"
#include "../View/Dialog/TrackDialog.h"
#include "../View/Dialog/SubDialog.h"

const int FONT_SIZE_NO_FILE = 12;
const int FONT_SIZE = 10;
const int MAX_TRACK_HEIGHT = 250;


int get_track_index_save(Track *t)
{
	if (t){
		foreachi(Track *tt, tsunami->audio->track, i)
			if (t == tt)
				return i;
	}
	return -1;
}

AudioView::SelectionType::SelectionType()
{
	type = SEL_TYPE_NONE;
	track = sub = NULL;
	pos = 0;
	sub_offset = 0;
}

AudioView::AudioView(CHuiWindow *parent, AudioFile *_audio) :
	Observable("AudioView"),
	SUB_FRAME_HEIGHT(20),
	TIME_SCALE_HEIGHT(20),
	BarrierDist(5)
{
	ColorBackground = White;
	ColorBackgroundCurWave = color(1, 0.93f, 0.93f, 1);
	ColorBackgroundCurTrack = color(1, 0.88f, 0.88f, 1);
	ColorGrid = color(1, 0.75f, 0.75f, 0.9f);
	ColorSelectionInternal = color(1, 0.7f, 0.7f, 0.9f);
	ColorSelectionBoundary = Blue;
	ColorSelectionBoundaryMO = Red;
	ColorPreviewMarker = color(1,0, 0.8f, 0);
	ColorWave = Gray;
	ColorWaveCur = color(1, 0.3f, 0.3f, 0.3f);
	ColorSub = color(1, 0.6f, 0.6f, 0);
	ColorSubMO = color(1, 0.6f, 0, 0);
	ColorSubNotCur = color(1, 0.4f, 0.4f, 0.4f);

	DrawingWidth = 800;

	show_mono = false;
	grid_mode = GRID_MODE_TIME;
	DetailSteps = HuiConfigReadInt("View.DetailSteps", 1);
	MouseMinMoveToSelect = HuiConfigReadInt("View.MouseMinMoveToSelect", 5);
	PreviewSleepTime = HuiConfigReadInt("PreviewSleepTime", 10);
	ScrollSpeed = HuiConfigReadInt("View.ScrollSpeed", 300);
	ScrollSpeedFast = HuiConfigReadInt("View.ScrollSpeedFast", 3000);
	ZoomSpeed = HuiConfigReadFloat("View.ZoomSpeed", 0.1f);
	PeakMode = HuiConfigReadInt("View.PeakMode", BufferBox::PEAK_MODE_SQUAREMEAN);


	MousePossiblySelecting = -1;
	cur_action = NULL;

	tsunami->SetBorderWidth(0);
	parent->SetTarget("main_table", 0);
	parent->AddDrawingArea("", 0, 0, 0, 0, "area");

	cur_track = NULL;
	cur_sub = NULL;
	cur_level = 0;

	audio = _audio;

	audio->area = rect(0, 0, 0, 0);
	Subscribe(audio);

	// events
	parent->EventMX("area", "hui:redraw", this, (void(HuiEventHandler::*)())&AudioView::OnDraw);
	parent->EventMX("area", "hui:mouse-move", this, (void(HuiEventHandler::*)())&AudioView::OnMouseMove);
	parent->EventMX("area", "hui:left-button-down", this, (void(HuiEventHandler::*)())&AudioView::OnLeftButtonDown);
	parent->EventMX("area", "hui:left-double-click", this, (void(HuiEventHandler::*)())&AudioView::OnLeftDoubleClick);
	parent->EventMX("area", "hui:left-button-up", this, (void(HuiEventHandler::*)())&AudioView::OnLeftButtonUp);
	parent->EventMX("area", "hui:middle-button-down", this, (void(HuiEventHandler::*)())&AudioView::OnMiddleButtonDown);
	parent->EventMX("area", "hui:middle-button-up", this, (void(HuiEventHandler::*)())&AudioView::OnMiddleButtonUp);
	parent->EventMX("area", "hui:right-button-down", this, (void(HuiEventHandler::*)())&AudioView::OnRightButtonDown);
	parent->EventMX("area", "hui:right-button-up", this, (void(HuiEventHandler::*)())&AudioView::OnRightButtonUp);
	//parent->EventMX("area", "hui:key-down", this, (void(HuiEventHandler::*)())&AudioView::OnKeyDown);
	parent->EventM("hui:key-down", this, (void(HuiEventHandler::*)())&AudioView::OnKeyDown);
	parent->EventM("hui:key-up", this, (void(HuiEventHandler::*)())&AudioView::OnKeyUp);
	parent->EventMX("area", "hui:mouse-wheel", this, (void(HuiEventHandler::*)())&AudioView::OnMouseWheel);


	HuiAddCommandM("select_none", "", -1, this, (void(HuiEventHandler::*)())&AudioView::OnSelectNone);
	HuiAddCommandM("select_all", "", KEY_A + KEY_CONTROL, this, (void(HuiEventHandler::*)())&AudioView::OnSelectAll);
	HuiAddCommandM("select_nothing", "", -1, this, (void(HuiEventHandler::*)())&AudioView::OnSelectNothing);
	HuiAddCommandM("view_mono", "", -1, this, (void(HuiEventHandler::*)())&AudioView::OnViewMono);
	HuiAddCommandM("view_grid_time", "", -1, this, (void(HuiEventHandler::*)())&AudioView::OnViewGridTime);
	HuiAddCommandM("view_grid_bars", "", -1, this, (void(HuiEventHandler::*)())&AudioView::OnViewGridBars);
	HuiAddCommandM("view_peaks_max", "", -1, this, (void(HuiEventHandler::*)())&AudioView::OnViewPeaksMax);
	HuiAddCommandM("view_peaks_mean", "", -1, this, (void(HuiEventHandler::*)())&AudioView::OnViewPeaksMean);
	HuiAddCommandM("view_optimal", "", -1, this, (void(HuiEventHandler::*)())&AudioView::OnViewOptimal);
	HuiAddCommandM("zoom_in", "", -1, this, (void(HuiEventHandler::*)())&AudioView::OnZoomIn);
	HuiAddCommandM("zoom_out", "", -1, this, (void(HuiEventHandler::*)())&AudioView::OnZoomOut);
	HuiAddCommandM("jump_other_file", "", -1, this, (void(HuiEventHandler::*)())&AudioView::OnJumpOtherFile);

	//ForceRedraw();
	UpdateMenu();

	track_dialog = new TrackDialog(tsunami);
	audio_file_dialog = new AudioFileDialog(tsunami, audio);
}

AudioView::~AudioView()
{
	Unsubscribe(audio);

	HuiConfigWriteInt("View.DetailSteps", DetailSteps);
	HuiConfigWriteInt("View.MouseMinMoveToSelect", MouseMinMoveToSelect);
	HuiConfigWriteInt("View.ScrollSpeed", ScrollSpeed);
	HuiConfigWriteInt("View.ScrollSpeedFast", ScrollSpeedFast);
	HuiConfigWriteFloat("View.ZoomSpeed", ZoomSpeed);
}


void AudioView::SetMouse()
{
	mx = HuiGetEvent()->mx;
	my = HuiGetEvent()->my;
}

bool AudioView::MouseOverTrack(Track *t)
{
	return t->area.inside(mx, my);
}

int AudioView::MouseOverSub(Track *s)
{
	if ((mx >= s->area.x1) && (mx < s->area.x2)){
		int offset = screen2sample(mx) - s->pos;
		if ((my >= s->area.y1) && (my < s->area.y1 + SUB_FRAME_HEIGHT))
			return offset;
		if ((my >= s->area.y2 - SUB_FRAME_HEIGHT) && (my < s->area.y2))
			return offset;
	}
	return -1;
}

void AudioView::SelectionUpdatePos(SelectionType &s)
{
	s.pos = screen2sample(mx);
}

bool mouse_over_time(AudioView *v, int pos)
{
	int ssx = v->sample2screen(pos);
	return ((v->mx >= ssx - 5) && (v->mx <= ssx + 5));
}

AudioView::SelectionType AudioView::GetMouseOver()
{
	SelectionType s;
	s.type = SEL_TYPE_NONE;
	s.track = NULL;
	s.sub = NULL;
	s.sub_offset = 0;

	// track?
	foreach(Track *t, audio->track){
		if (MouseOverTrack(t)){
			s.track = t;
			s.type = SEL_TYPE_TRACK;
		}
	}

	// selection boundaries?
	SelectionUpdatePos(s);
	if (!audio->selection.empty()){
		if (mouse_over_time(this, audio->sel_raw.start())){
			s.type = SEL_TYPE_SELECTION_START;
			return s;
		}
		if (mouse_over_time(this, audio->sel_raw.end())){
			s.type = SEL_TYPE_SELECTION_END;
			return s;
		}
	}
	if ((tsunami->output->IsPlaying()) && (tsunami->output->GetAudio() == audio)){
		if (mouse_over_time(this, tsunami->output->GetRange().start())){
			s.type = SEL_TYPE_PLAYBACK_START;
			return s;
		}
		if (mouse_over_time(this, tsunami->output->GetRange().end())){
			s.type = SEL_TYPE_PLAYBACK_END;
			return s;
		}
		if (mouse_over_time(this, tsunami->output->GetPos())){
			s.type = SEL_TYPE_PLAYBACK;
			return s;
		}
	}

	// sub?
	if (s.track){
		// TODO: prefer selected subs
		foreach(Track *ss, s.track->sub){
			int offset = MouseOverSub(ss);
			if (offset >= 0){
				s.sub = ss;
				s.type = SEL_TYPE_SUB;
				s.sub_offset = offset;
			}
		}
	}

	return s;
}


void AudioView::SelectUnderMouse()
{
	msg_db_r("SelectUnderMouse", 2);
	Hover = GetMouseOver();
	Selection = Hover;
	Track *t = Selection.track;
	Track *s = Selection.sub;
	bool control = tsunami->GetKey(KEY_CONTROL);

	// track
	if (Selection.type == SEL_TYPE_TRACK){
		SelectTrack(t, control);
		if (t->is_selected)
			SetCurTrack(t);
		if (!control)
			audio->UnselectAllSubs();
	}

	// sub
	if (Selection.type == SEL_TYPE_SUB){
		SelectSub(s, control);
		if (s->is_selected)
			SetCurSub(s);
	}
	msg_db_l(2);
}

void AudioView::SetBarriers(SelectionType *s)
{
	msg_db_r("SetBarriers", 2);
	s->barrier.clear();
	if (s->type == SEL_TYPE_NONE){
		msg_db_l(2);
		return;
	}

	int dpos = 0;
	if (s->type == SEL_TYPE_SUB)
		dpos = s->sub_offset;

	foreach(Track *t, audio->track){
		// add subs
		foreach(Track *sub, t->sub){
			s->barrier.add(sub->pos + dpos);
		}

		// time bar...
		int x0 = 0;
		foreach(Bar &b, t->bar){
			for (int i=0;i<b.num_beats;i++)
				s->barrier.add(x0 + (int)((float)b.length * i / (float)b.num_beats) + dpos);
			x0 += b.length;
		}
	}

	// selection marker
	if (!audio->selection.empty()){
		s->barrier.add(audio->sel_raw.start());
		if (MousePossiblySelecting < 0)
			s->barrier.add(audio->sel_raw.end());
	}
	msg_db_l(2);
}

void AudioView::ApplyBarriers(int &pos)
{
	msg_db_r("ApplyBarriers", 2);
	foreach(int b, Selection.barrier){
		int dpos = sample2screen(b) - sample2screen(pos);
		if (abs(dpos) <= BarrierDist){
			//msg_write(format("barrier:  %d  ->  %d", pos, b));
			pos = b;
		}
	}
	msg_db_l(2);
}

void AudioView::OnMouseMove()
{
	msg_db_r("OnMouseMove", 2);
	SetMouse();
	bool _force_redraw_ = false;

	if (HuiGetEvent()->lbut){
		SelectionUpdatePos(Selection);
	}else{
		SelectionType mo_old = Hover;
		Hover = GetMouseOver();
		_force_redraw_ |= (Hover.type != mo_old.type) || (Hover.sub != mo_old.sub);
	}


	// drag & drop
	if (Selection.type == SEL_TYPE_SELECTION_END){
		SelectionType mo = GetMouseOver();
		if (mo.track)
			mo.track->is_selected = true;

		ApplyBarriers(Selection.pos);
		audio->sel_raw.set_end(Selection.pos);
		audio->UpdateSelection();
		//_force_redraw_ = true;
		_force_redraw_ = false;
		int x, w;
		int r = 4;
		if (HuiGetEvent()->dx < 0){
			x = mx - r;
			w = - HuiGetEvent()->dx + 2*r;
		}else{
			x = mx + r;
			w = - HuiGetEvent()->dx - 2*r;
		}
		tsunami->RedrawRect("area", x, audio->area.y1, w, audio->area.height());
	}else if (Selection.type == SEL_TYPE_PLAYBACK_START){
		tsunami->output->SetRangeStart(Selection.pos);
		_force_redraw_ = true;
	}else if (Selection.type == SEL_TYPE_PLAYBACK_END){
		tsunami->output->SetRangeEnd(Selection.pos);
		_force_redraw_ = true;
	}else if (Selection.type == SEL_TYPE_PLAYBACK){
		tsunami->output->Seek(Selection.pos);
		_force_redraw_ = true;
	}else if (Selection.type == SEL_TYPE_SUB){
		ApplyBarriers(Selection.pos);
		int dpos = (float)Selection.pos - Selection.sub_offset - Selection.sub->pos;
		if (cur_action)
			cur_action->set_param_and_notify(audio, dpos);
		_force_redraw_ = true;
	}

	// selection:
	if (!HuiGetEvent()->lbut){
		MousePossiblySelecting = -1;
	}
	if (MousePossiblySelecting >= 0)
		MousePossiblySelecting += abs(HuiGetEvent()->dx);
	if (MousePossiblySelecting > MouseMinMoveToSelect){
		audio->sel_raw.offset = MousePossiblySelectingStart;
		audio->sel_raw.num = Selection.pos - MousePossiblySelectingStart;
		SetBarriers(&Selection);
		audio->UpdateSelection();
		Selection.type = SEL_TYPE_SELECTION_END;
		Hover.type = SEL_TYPE_SELECTION_END;
		_force_redraw_ = true;
		MousePossiblySelecting = -1;
	}

	if (_force_redraw_)
		ForceRedraw();

	msg_db_l(2);
}



void AudioView::OnLeftButtonDown()
{
	msg_db_r("OnLBD", 2);
	SelectUnderMouse();
	UpdateMenu();

	if (!audio->used){
		msg_db_l(2);
		return;
	}

	// selection:
	//   start after lb down and moving
	if ((Selection.type == SEL_TYPE_TRACK) || (Selection.type == SEL_TYPE_TIME)){
		MousePossiblySelecting = 0;
		int pos = screen2sample(mx);
		MousePossiblySelectingStart = pos;
	}else if (Selection.type == SEL_TYPE_SELECTION_START){
		// switch end / start
		Selection.type = SEL_TYPE_SELECTION_END;
		Hover.type = SEL_TYPE_SELECTION_END;
		audio->sel_raw.invert();
	}else if (Selection.type == SEL_TYPE_SUB){
		cur_action = new ActionSubTrackMove(audio);
	}

	SetBarriers(&Selection);

	ForceRedraw();
	UpdateMenu();
	msg_db_l(2);
}



void AudioView::OnLeftButtonUp()
{
	msg_db_r("OnLBU", 2);
	if (Selection.type == SEL_TYPE_SUB)
		if (cur_action)
			audio->Execute(cur_action);
	cur_action = NULL;

	// TODO !!!!!!!!
	Selection.type = SEL_TYPE_NONE;
	ForceRedraw();
	UpdateMenu();
	msg_db_l(2);
}



void AudioView::OnMiddleButtonDown()
{
	SelectUnderMouse();

	SelectNone();
	UpdateMenu();
}



void AudioView::OnMiddleButtonUp()
{
}



void AudioView::OnRightButtonDown()
{
	SelectUnderMouse();

	// pop up menu...
	UpdateMenu();
}



void AudioView::OnRightButtonUp()
{
}



void AudioView::OnLeftDoubleClick()
{
	SelectUnderMouse();

	if (MousePossiblySelecting < MouseMinMoveToSelect)
		if (audio->used){
			if (Selection.type == SEL_TYPE_SUB)
				ExecuteSubDialog(tsunami);
			else if (Selection.type == SEL_TYPE_TRACK)
				ExecuteTrackDialog(tsunami);
			else
				ExecuteAudioDialog(tsunami);
			Selection.type = SEL_TYPE_NONE;
		}
	UpdateMenu();
}



void AudioView::OnCommand(const string & id)
{
}



void AudioView::OnKeyDown()
{
	int k = HuiGetEvent()->key;

// view
	// moving
	float dt = 0.05f;
	if (k == KEY_RIGHT)
		Move(ScrollSpeed * dt / view_zoom);
	if (k == KEY_LEFT)
		Move(- ScrollSpeed * dt / view_zoom);
	if (k == KEY_NEXT)
		Move(ScrollSpeedFast * dt / view_zoom);
	if (k == KEY_PRIOR)
		Move(- ScrollSpeedFast * dt / view_zoom);
	// zoom
	if (k == KEY_ADD)
		Zoom(exp(  ZoomSpeed));
	if (k == KEY_SUBTRACT)
		Zoom(exp(- ZoomSpeed));
	UpdateMenu();
}



void AudioView::OnKeyUp()
{
}



void AudioView::OnMouseWheel()
{
	Zoom(exp(ZoomSpeed * HuiGetEvent()->dz));
}


void AudioView::ForceRedraw()
{
	force_redraw = true;
	tsunami->Redraw("area");
}


#define MIN_GRID_DIST	10.0f

float tx[4096], ty[4096], ty2[4096];

inline void draw_line_buffer(HuiDrawingContext *c, int width, int di, float view_pos_rel, float zoom, float f, float hf, float x, float y0, const Array<float> &buf, int offset)
{
	int nl = 0;
	float dpos = (float)1/zoom/f;
	// pixel position
	// -> buffer position
	float p0 = view_pos_rel / f;
	for (int i=0;i<width+di;i+=di){

		float p = p0 + dpos * (float)i;
		int ip = (int)p - offset;
		//printf("%f  %f\n", p1, p2);
		if ((ip >= 0) && (ip < buf.num))
		if (((int)(p * f) < offset + buf.num) && (p >= offset)){
			tx[nl] = (float)x+i;
			ty[nl] = y0 + buf[ip] * hf;
//			msg_write(ip);
//			msg_write(f2s(buf[ip], 5));
			nl ++;
		}
		//p += dpos;
	}
	c->DrawLines(tx, ty, nl -1);
}

inline void draw_peak_buffer(HuiDrawingContext *c, int width, int di, float view_pos_rel, float zoom, float f, float hf, float x, float y0, const string &buf, int offset)
{
	int nl = 0;
	float dpos = (float)1/zoom;
	// pixel position
	// -> buffer position
	float p0 = view_pos_rel;
	for (int i=0;i<width+di;i+=di){

		float p = p0 + dpos * (float)i;
		int ip = (int)(p - offset)/f;
		//printf("%f  %f\n", p1, p2);
		if ((ip >= 0) && (ip < buf.num))
		if (((int)(p) < offset + buf.num*f) && (p >= offset)){
			tx[nl] = (float)x+i;
			float dy = ((float)((unsigned char)buf[ip])/255.0f) * hf;
			ty[nl]  = y0 + dy;
			//ty2[nl] = y0 - dy;
//			msg_write(ip);
//			msg_write(f2s(buf[ip], 5));
			nl ++;
		}
		//p += dpos;
	}
	for (int i=0;i<nl;i++){
		tx[nl + i] = tx[nl - i - 1];
		ty[nl + i] = y0 *2 - ty[nl - i - 1] - 1;
	}
	c->DrawPolygon(tx, ty, nl*2);
//	c->DrawLines(tx, ty, nl*2 -1);
	//c->DrawLines(tx, ty2, nl -1);
}

void AudioView::DrawBuffer(HuiDrawingContext *c, const rect &r, Track *t, int view_pos_rel, float zoom, const color &col)
{
	msg_db_r("DrawBuffer", 1);
	int l_best = 0;
	float f = 1.0f;

	// which level of detail?
	if (zoom < 0.8f)
		for (int i=24-1;i>=0;i--){
			float _f = (float)pow(2, (float)i);
			if (_f > 1.0f / zoom){
				l_best = i;
				f = _f;
			}
		}

	// zero heights of both channels
	float y0r = r.y1 + r.height() / 4;
	float y0l = r.y1 + r.height() * 3 / 4;

	float hf = r.height() / 4;

	if (show_mono){
		y0r = r.y1 + r.height() / 2;
		hf *= 2;
	}


	int di = DetailSteps;
	foreachi(TrackLevel &lev, t->level, level_no){
		if ((level_no == cur_level) || (!t->parent))
			c->SetColor(col);
		else
			c->SetColor(ColorWave);
		foreach(BufferBox &b, lev.buffer){
			int l = min(l_best - 1, b.peak.num / 2);
			if (l >= 1){//f < MIN_MAX_FACTOR){
				draw_peak_buffer(c, r.width(), di, view_pos_rel, zoom, f, hf, r.x1, y0r, b.peak[l*2-2], b.offset);
				if (!show_mono)
					draw_peak_buffer(c, r.width(), di, view_pos_rel, zoom, f, hf, r.x1, y0l, b.peak[l*2-1], b.offset);
			}else{
				draw_line_buffer(c, r.width(), di, view_pos_rel, zoom, 1, hf, r.x1, y0r, b.r, b.offset);
				if (!show_mono)
					draw_line_buffer(c, r.width(), di, view_pos_rel, zoom, 1, hf, r.x1, y0l, b.l, b.offset);
			}
		}
	}
	msg_db_l(1);
}

void AudioView::DrawSubFrame(HuiDrawingContext *c, const rect &r, Track *s, const color &col, int delay)
{
	// frame
	int asx = clampi(sample2screen(s->pos + delay), r.x1, r.x2);
	int aex = clampi(sample2screen(s->pos + s->length + delay), r.x1, r.x2);

	if (delay == 0)
		s->area = rect(asx, aex, r.y1, r.y2);


	color col2 = col;
	col2.a *= 0.2f;
	c->SetColor(col2);
	c->DrawRect(asx, r.y1,                    aex - asx, SUB_FRAME_HEIGHT);
	c->DrawRect(asx, r.y2 - SUB_FRAME_HEIGHT, aex - asx, SUB_FRAME_HEIGHT);

	c->SetColor(col);
	c->DrawLine(asx, r.y1, asx, r.y2);
	c->DrawLine(aex, r.y1, aex, r.y2);
	c->DrawLine(asx, r.y1, aex, r.y1);
	c->DrawLine(asx, r.y2, aex, r.y2);
}

void AudioView::DrawSub(HuiDrawingContext *c, const rect &r, Track *s)
{
	color col = ColorSub;
	//bool is_cur = ((s == cur_sub) && (t->IsSelected));
	if (!s->is_selected)
		col = ColorSubNotCur;
	if (Hover.sub == s)
		col = ColorSubMO;
	//col.a = 0.2f;

	DrawSubFrame(c, r, s, col, 0);

	color col2 = col;
	col2.a *= 0.5f;
	for (int i=0;i<s->rep_num;i++)
		DrawSubFrame(c, r, s, col2, (i + 1) * s->rep_delay);

	// buffer
	DrawBuffer(	c, r, s, int(view_pos - s->pos), view_zoom, col);

	int asx = clampi(sample2screen(s->pos), r.x1, r.x2);
	if (s->is_selected)//((is_cur) || (a->sub_mouse_over == s))
		c->DrawStr(asx, r.y2 - SUB_FRAME_HEIGHT, s->name);
}

void AudioView::DrawBars(HuiDrawingContext *c, const rect &r, Track *t, color col, int track_no, Array<Bar> &bc)
{
	int x0 = 0;
	int n = 1;
	foreachi(Bar &bar, bc, i){
		bar.x     = sample2screen(x0);
		bar.width = sample2screen(x0 + bar.length * bar.count) - bar.x;
		if (bar.type == bar.TYPE_BAR){
			for (int j=0;j<bar.count;j++){
				for (int i=0;i<bar.num_beats;i++){
					int bx = sample2screen(x0 + (int)((float)bar.length * i / bar.num_beats));

					color cc = (i == 0) ? Red : col;
					c->SetColor(cc);
					if (i == 0)
						c->DrawStr(bx + 2, r.y1 + r.height()/2, i2s(n));

					if ((bx >= r.x1) && (bx < r.x2))
						c->DrawLine(bx, r.y1, bx, r.y2);
				}
				x0 += bar.length;
				n ++;
			}
		}else if (bar.type == bar.TYPE_PAUSE){
			x0 += bar.length;
		}
	}
}

void DrawStrBg(HuiDrawingContext *c, float x, float y, const string &str, const color &fg, const color &bg)
{
	color bg2 = bg;
	bg2.a = 0.6f;
	c->SetColor(bg2);
	c->DrawRect(x, y, c->GetStrWidth(str), FONT_SIZE * 1.5f);
	c->SetColor(fg);
	c->DrawStr(x, y, str);
}

void AudioView::DrawTrack(HuiDrawingContext *c, const rect &r, Track *t, color col, int track_no)
{
	msg_db_r("DrawTrack", 1);

	DrawBuffer(	c, r, t,int(view_pos),view_zoom,col);

	DrawBars(c, r, t, col, track_no, t->bar);

	foreach(Track *s, t->sub)
		DrawSub(c, r, s);

	//c->SetColor((track_no == a->CurTrack) ? Black : ColorWaveCur);
//	c->SetColor(ColorWaveCur);
	c->SetFont("", -1, (t == cur_track), (t->type == Track::TYPE_TIME));
	DrawStrBg(c, r.x1 + 3, r.y1 + 3, t->GetNiceName(), ColorWaveCur, ColorBackgroundCurWave);
	c->SetFont("", -1, false, false);

	msg_db_l(1);
}

void AudioView::DrawGrid(HuiDrawingContext *c, const rect &r, const color &bg, bool show_time)
{
	if (grid_mode == GRID_MODE_TIME)
		DrawGridTime(c, r, bg, show_time);
	else if (grid_mode == GRID_MODE_BARS)
		DrawGridBars(c, r, bg, show_time);
}

void AudioView::DrawGridTime(HuiDrawingContext *c, const rect &r, const color &bg, bool show_time)
{
	float dl = MIN_GRID_DIST / view_zoom; // >= 10 pixel
	float dt = dl / audio->sample_rate;
	float exp_s = ceil(log10(dt));
	float exp_s_mod = exp_s - log10(dt);
	dt = pow(10, exp_s);
	dl = dt * audio->sample_rate;
//	float dw = dl * a->view_zoom;
	int nx0 = floor((float)screen2sample(r.x1 - 1) / (float)dl);
	int nx1 = ceil((float)screen2sample(r.x2) / (float)dl);
	color c1 = ColorInterpolate(bg, ColorGrid, exp_s_mod);
	color c2 = ColorGrid;
	for (int n=nx0;n<nx1;n++){
		c->SetColor(((n % 10) == 0) ? c2 : c1);
		int xx = sample2screen(n * dl);
		c->DrawLine(xx, r.y1, xx, r.y2);
	}
	if (show_time){
		if ((tsunami->output->IsPlaying()) && (tsunami->output->GetAudio() == audio)){
			c->SetColor(ColorPreviewMarker);
			float x0 = sample2screen(tsunami->output->GetRange().start());
			float x1 = sample2screen(tsunami->output->GetRange().end());
			c->DrawRect(x0, r.y1, x1 - x0, 5);
		}
		c->SetColor(ColorGrid);
		for (int n=nx0;n<nx1;n++){
			if ((sample2screen(dl) - sample2screen(0)) > 30){
				if ((((n % 10) % 3) == 0) && ((n % 10) != 9) && ((n % 10) != -9))
					c->DrawStr(sample2screen(n * dl) + 2, r.y1, audio->get_time_str_fuzzy(n * dl, dt * 3));
			}else{
				if ((n % 10) == 0)
					c->DrawStr(sample2screen(n * dl) + 2, r.y1, audio->get_time_str_fuzzy(n * dl, dt * 10));
			}
		}
	}
}

void AudioView::DrawGridBars(HuiDrawingContext *c, const rect &r, const color &bg, bool show_time)
{
	Track *t = NULL;
	foreach(Track *tt, audio->track)
		if (tt->type == tt->TYPE_TIME)
			t = tt;
	if (!t)
		return;
	int s0 = screen2sample(r.x1 - 1);
	int s1 = screen2sample(r.x2);
	Array<Beat> beats = t->bar.GetBeats(Range(s0, s1 - s0));
	foreach(Beat &b, beats){
		c->SetColor((b.beat_no == 0) ? Red : Black);
		int xx = sample2screen(b.pos);
		c->DrawLine(xx, r.y1, xx, r.y2);
	}
}

void AudioView::OnUpdate(Observable *o)
{
	//msg_write("view: " + o->GetName() + " - " + o->GetMessage());

	// check cur_track consistency
	int n = get_track_index_save(cur_track);
	if (n < 0)
		if (audio->track.num > 0)
			SetCurTrack(audio->track[0]);

	// check cur_level consistency
	if ((cur_level < 0) || (cur_level >= audio->level_name.num)){
		cur_level = 0;
		ForceRedraw();
	}

	if (o->GetMessage() == "New"){
		OptimizeView();
	}else{
		ForceRedraw();
		UpdateMenu();
	}
}

void plan_track_sizes(const rect &r, AudioFile *a, int TIME_SCALE_HEIGHT)
{
	int opt_track_height = MAX_TRACK_HEIGHT;
	if (tsunami->view->show_mono)
		opt_track_height /= 2;

	int h_wish = TIME_SCALE_HEIGHT;
	int h_fix = TIME_SCALE_HEIGHT;
	int n_var = 0;
	foreach(Track *t, a->track){
		if (t->type == t->TYPE_TIME){
			h_wish += TIME_SCALE_HEIGHT * 2;
			h_fix += TIME_SCALE_HEIGHT * 2;
		}else{
			h_wish += opt_track_height;
			n_var ++;
		}
	}

	int y0 = r.y1 + TIME_SCALE_HEIGHT;
	if (h_wish > r.height())
		opt_track_height = (r.height() - h_fix) / n_var;
	foreachi(Track *t, a->track, i){
		float h = (t->type == t->TYPE_TIME) ? TIME_SCALE_HEIGHT*2 : opt_track_height;
		t->area = rect(r.x1, r.x2, y0, y0 + h);
		y0 += h;
	}
}

void audio_draw_background(HuiDrawingContext *c, const rect &r, AudioFile *a, AudioView *v)
{
	int yy = a->track.back()->area.y2;
	//int trackheight = (a->num_tracks > 0) ? (height / a->num_tracks) : height;
	c->SetColor(v->ColorBackgroundCurWave);
	c->DrawRect(r.x1, r.y1, r.width(), v->TIME_SCALE_HEIGHT);
	v->DrawGrid(c, rect(r.x1, r.x2, r.y1, r.y1 + v->TIME_SCALE_HEIGHT), v->ColorBackgroundCurWave, true);
	foreach(Track *t, a->track){
		c->SetColor((t->is_selected) ? v->ColorBackgroundCurTrack : v->ColorBackgroundCurWave);
		c->DrawRect(t->area);
		v->DrawGrid(c, t->area, (t->is_selected) ? v->ColorBackgroundCurTrack : v->ColorBackgroundCurWave);
	}
	if (yy < r.y2){
		c->SetColor(v->ColorBackground);
		rect rr = rect(r.x1, r.x2, yy, r.y2);
		c->DrawRect(rr);
		v->DrawGrid(c, rr, v->ColorBackground, false);
	}

	// lines between tracks
	c->SetColor(v->ColorGrid);
	foreach(Track *t, a->track)
		c->DrawLine(0, t->area.y1, r.width(), t->area.y1);
	if (yy < r.y2)
		c->DrawLine(0, yy, r.width(), yy);
}

void AudioView::DrawTimeLine(HuiDrawingContext *c, int pos, int type, color &col, bool show_time)
{
	int p = sample2screen(pos);
	if ((p >= audio->area.x1) && (p <= audio->area.x2)){
		c->SetColor((type == Hover.type) ? ColorSelectionBoundaryMO : col);
		c->DrawLine(p, audio->area.y1, p, audio->area.y2);
		if (show_time)
			c->DrawStr(p, (audio->area.y1 + audio->area.y2) / 2, audio->get_time_str(pos));
	}
}

void AudioView::DrawAudioFile(HuiDrawingContext *c, const rect &r)
{
	audio->area = r;

	// empty file
	if (!audio->used){
		color col = ColorBackgroundCurWave;
		c->SetColor(col);
		c->DrawRect(r.x1, r.y2, r.width(), r.height());
		c->SetColor(ColorWaveCur);
		c->SetFontSize(FONT_SIZE_NO_FILE);
		c->DrawStr(r.x1 + r.width() / 2 - 50, r.y1 + r.height() / 2 - 10, _("keine Datei"));
		return;
	}

	plan_track_sizes(r, audio, TIME_SCALE_HEIGHT);

	// background
	audio_draw_background(c, r, audio, this);


	// selection
	if (!audio->selection.empty()){
		int sx1 = sample2screen(audio->sel_raw.start());
		int sx2 = sample2screen(audio->sel_raw.end());
		int sxx1 = clampi(sx1, r.x1, r.x2);
		int sxx2 = clampi(sx2, r.x1, r.x2);
		if (sxx1 > sxx2){
			int t = sxx1;	sxx1 = sxx2;	sxx2 = t;
			//bool bt = mo_s;	mo_s = mo_e;	mo_e = bt; // TODO ???
		}
		foreach(Track *t, audio->track)
			if (t->is_selected){
				c->SetColor(ColorSelectionInternal);
				rect rr = rect(sxx1, sxx2, t->area.y1, t->area.y2);
				c->DrawRect(rr);
				DrawGrid(c, rr, ColorSelectionInternal);
			}
		DrawTimeLine(c, audio->sel_raw.start(), SEL_TYPE_SELECTION_START, ColorSelectionBoundary);
		DrawTimeLine(c, audio->sel_raw.end(), SEL_TYPE_SELECTION_END, ColorSelectionBoundary);
	}

	//NixDrawStr(x,y,get_time_str((int)a->ViewPos,a));
	// playing position
	if ((tsunami->output->IsPlaying()) && (tsunami->output->GetAudio() == audio)){
		DrawTimeLine(c, tsunami->output->GetRange().start(), SEL_TYPE_PLAYBACK_START, ColorPreviewMarker);
		DrawTimeLine(c, tsunami->output->GetRange().end(), SEL_TYPE_PLAYBACK_END, ColorPreviewMarker);
		DrawTimeLine(c, tsunami->output->GetPos(), SEL_TYPE_PLAYBACK, ColorPreviewMarker, true);
	}

	c->SetColor(ColorWaveCur);

	// boundary of wave file
	/*if (a->min != a->max){
		c->DrawLine(sample2screen(a, a->min), y, sample2screen(a, a->min), y + height);
		c->DrawLine(sample2screen(a, a->max), y, sample2screen(a, a->max), y + height);
	}*/


	foreachi(Track *tt, audio->track, i)
		DrawTrack(c, tt->area, tt, ColorWaveCur, i);

}

int frame=0;

void AudioView::OnDraw()
{
	msg_db_r("OnDraw", 1);
	force_redraw = false;

	HuiDrawingContext *c = tsunami->BeginDraw("area");
	DrawingWidth = c->width;
	c->SetFontSize(FONT_SIZE);
	c->SetLineWidth(1.0f);
	c->SetAntialiasing(false);
	//c->SetColor(ColorWaveCur);

	DrawAudioFile(c, rect(0, c->width, 0, c->height));

	//c->DrawStr(100, 100, i2s(frame++));

	c->End();

	msg_db_l(1);
}

void AudioView::OptimizeView()
{
	msg_db_r("OptimizeView", 1);
	if (audio->area.x2 <= 0)
		audio->area.x2 = DrawingWidth;

	Range r = audio->GetRange();

	int length = r.length();
	if (length == 0)
		length = 10 * audio->sample_rate;
	view_zoom = audio->area.width() / (float)length;
	view_pos = (float)r.start();
	ForceRedraw();
	msg_db_l(1);
}

void AudioView::UpdateMenu()
{
	// edit
	tsunami->Enable("select_all", audio->used);
	tsunami->Enable("select_nothing", audio->used);
	// view
	tsunami->Check("view_mono", show_mono);
	tsunami->Check("view_grid_time", grid_mode == GRID_MODE_TIME);
	tsunami->Check("view_grid_bars", grid_mode == GRID_MODE_BARS);
	tsunami->Check("view_peaks_max", PeakMode == BufferBox::PEAK_MODE_MAXIMUM);
	tsunami->Check("view_peaks_mean", PeakMode == BufferBox::PEAK_MODE_SQUAREMEAN);
	tsunami->Enable("zoom_in", audio->used);
	tsunami->Enable("zoom_out", audio->used);
	tsunami->Enable("view_optimal", audio->used);
	tsunami->Enable("view_samples", false);//tsunami->cur_audio->used);
}

void AudioView::OnViewPeaksMax()
{
	PeakMode = BufferBox::PEAK_MODE_MAXIMUM;
	if (audio->used){
		audio->InvalidateAllPeaks();
		audio->UpdatePeaks(PeakMode);
	}
	ForceRedraw();
	UpdateMenu();
}

void AudioView::OnViewPeaksMean()
{
	PeakMode = BufferBox::PEAK_MODE_SQUAREMEAN;
	if (audio->used){
		audio->InvalidateAllPeaks();
		audio->UpdatePeaks(PeakMode);
	}
	ForceRedraw();
	UpdateMenu();
}

void AudioView::OnViewOptimal()
{
	OptimizeView();
}

void AudioView::OnSelectNone()
{
}

void AudioView::OnViewMono()
{
	show_mono = !show_mono;
	ForceRedraw();
	UpdateMenu();
}

void AudioView::OnZoomIn()
{
}

void AudioView::OnSelectAll()
{
	SelectAll();
}

void AudioView::SelectAll()
{
	audio->sel_raw = audio->GetRange();
	audio->UpdateSelection();
}

void AudioView::SelectNone()
{
	// select all/none
	audio->sel_raw.clear();
	audio->UpdateSelection();
	audio->UnselectAllSubs();
	SetCurSub(NULL);
}

void AudioView::OnZoomOut()
{
}

void AudioView::OnJumpOtherFile()
{
}

void AudioView::OnViewGridTime()
{
	grid_mode = GRID_MODE_TIME;
	ForceRedraw();
	UpdateMenu();
}

void AudioView::OnViewGridBars()
{
	grid_mode = GRID_MODE_BARS;
	ForceRedraw();
	UpdateMenu();
}

void AudioView::OnSelectNothing()
{
}



void AudioView::SelectSub(Track *s, bool diff)
{
	if (!s)
		return;
	if (diff){
		s->is_selected = !s->is_selected;
	}else{
		if (!s->is_selected)
			s->root->UnselectAllSubs();

		// select this sub
		s->is_selected = true;
	}
}

void AudioView::SelectTrack(Track *t, bool diff)
{
	if (!t)
		return;
	if (diff){
		bool is_only_selected = true;
		foreach(Track *tt, t->root->track)
			if ((tt->is_selected) && (tt != t))
				is_only_selected = false;
		t->is_selected = !t->is_selected || is_only_selected;
	}else{
		if (!t->is_selected){
			// unselect all tracks
			foreach(Track *tt, t->root->track)
				tt->is_selected = false;
		}

		// select this track
		t->is_selected = true;
	}
	t->root->UpdateSelection();
}

void AudioView::SetCurSub(Track *s)
{
	cur_sub = s;
}


void AudioView::SetCurTrack(Track *t)
{
	cur_track = t;
	track_dialog->SetTrack(cur_track);
}



int AudioView::screen2sample(int _x)
{
	return (int)( (_x - audio->area.x1) / view_zoom + view_pos );
}

int AudioView::sample2screen(int s)
{
	return (int)( audio->area.x1 + (s - view_pos) * view_zoom );
}

void AudioView::Zoom(float f)
{
	// max zoom: 8 pixel per sample
	// min zoom: whole file on 100 pixel
	int length = audio->GetRange().length();
	if (length == 0)
		length = 10 * audio->sample_rate;
	f = clampf(f, 100.0 / (length * view_zoom), 8.0f / view_zoom);
	view_zoom *= f;
	view_pos += float(mx - audio->area.x1) / (view_zoom / (f - 1));
	ForceRedraw();
}

void AudioView::Move(float dpos)
{
	view_pos += dpos;
	ForceRedraw();
}

void AudioView::ExecuteTrackDialog(CHuiWindow *win)
{
	win->HideControl("track_dialog_table", false);
	win->HideControl("audio_dialog_table", true);
}



void AudioView::ExecuteSubDialog(CHuiWindow *win)
{
	if (!cur_sub){
		tsunami->log->Error(_("Keine Ebene ausgew&ahlt"));
		return;
	}
	SubDialog *dlg = new SubDialog(win, false, cur_sub);
	dlg->Update();
	HuiWaitTillWindowClosed(dlg);
}



void AudioView::ExecuteAudioDialog(CHuiWindow *win)
{
	win->HideControl("track_dialog_table", true);
	win->HideControl("audio_dialog_table", false);
}


