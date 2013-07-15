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
#include "../Action/Track/Sample/ActionTrackMoveSample.h"
#include "../Audio/AudioInput.h"
#include "../Audio/AudioOutput.h"
#include "../Stuff/Log.h"
#include "../lib/math/math.h"

const int FONT_SIZE_NO_FILE = 12;
const int FONT_SIZE = 10;
const int MAX_TRACK_CHANNEL_HEIGHT = 125;
const float LINE_WIDTH = 1.0f;

const float BORDER_FACTOR = 1.0f / 15.0f;

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
	track = NULL;
	sample = NULL;
	pos = 0;
	sample_offset = 0;
	show_track_controls = NULL;
}

AudioView::AudioView(HuiWindow *parent, AudioFile *_audio) :
	Observable("AudioView"),
	SUB_FRAME_HEIGHT(20),
	TIME_SCALE_HEIGHT(20),
	BarrierDist(5)
{
	ColorBackground = White;
	ColorBackgroundCurWave = color(1, 0.93f, 0.93f, 1);
	ColorBackgroundCurTrack = color(1, 0.88f, 0.88f, 1);
	ColorGrid = color(1, 0.75f, 0.75f, 0.9f);
	ColorSelectionInternal = color(0.2f, 0.2f, 0.2f, 0.8f);
	ColorSelectionBoundary = color(1, 0.2f, 0.2f, 0.8f);
	ColorSelectionBoundaryMO = color(1, 0.8f, 0.2f, 0.2f);
	ColorPreviewMarker = color(1, 0, 0.7f, 0);
	ColorCaptureMarker = color(1, 0.7f, 0, 0);
	ColorWave = Gray;
	ColorWaveCur = color(1, 0.3f, 0.3f, 0.3f);
	ColorSub = color(1, 0.6f, 0.6f, 0);
	ColorSubMO = color(1, 0.6f, 0, 0);
	ColorSubNotCur = color(1, 0.4f, 0.4f, 0.4f);

	DrawingWidth = 800;

	show_mono = HuiConfigReadBool("View.Mono", false);
	grid_mode = HuiConfigReadInt("View.GridMode", GRID_MODE_TIME);
	DetailSteps = HuiConfigReadInt("View.DetailSteps", 1);
	MouseMinMoveToSelect = HuiConfigReadInt("View.MouseMinMoveToSelect", 5);
	PreviewSleepTime = HuiConfigReadInt("PreviewSleepTime", 10);
	ScrollSpeed = HuiConfigReadInt("View.ScrollSpeed", 300);
	ScrollSpeedFast = HuiConfigReadInt("View.ScrollSpeedFast", 3000);
	ZoomSpeed = HuiConfigReadFloat("View.ZoomSpeed", 0.1f);
	PeakMode = HuiConfigReadInt("View.PeakMode", BufferBox::PEAK_MODE_SQUAREMEAN);
	Antialiasing = HuiConfigReadBool("View.Antialiasing", false);

	image_unmuted.Load(HuiAppDirectoryStatic + "Data/volume.tga");
	image_muted.Load(HuiAppDirectoryStatic + "Data/mute.tga");
	image_solo.Load(HuiAppDirectoryStatic + "Data/solo.tga");
	image_track_audio.Load(HuiAppDirectoryStatic + "Data/track-audio.tga");
	image_track_time.Load(HuiAppDirectoryStatic + "Data/track-time.tga");
	image_track_midi.Load(HuiAppDirectoryStatic + "Data/track-midi.tga");

	MousePossiblySelecting = -1;
	cur_action = NULL;

	tsunami->SetBorderWidth(0);
	parent->SetTarget("main_table", 0);
	parent->AddDrawingArea("!grabfocus", 0, 0, 0, 0, "area");

	cur_track = NULL;
	cur_sample = NULL;
	cur_level = 0;

	audio = _audio;

	audio->area = rect(0, 0, 0, 0);
	Subscribe(audio);
	Subscribe(tsunami->output);
	Subscribe(tsunami->input);

	// events
	parent->EventMX("area", "hui:redraw", this, &AudioView::OnDraw);
	parent->EventMX("area", "hui:mouse-move", this, &AudioView::OnMouseMove);
	parent->EventMX("area", "hui:left-button-down", this, &AudioView::OnLeftButtonDown);
	parent->EventMX("area", "hui:left-double-click", this, &AudioView::OnLeftDoubleClick);
	parent->EventMX("area", "hui:left-button-up", this, &AudioView::OnLeftButtonUp);
	parent->EventMX("area", "hui:middle-button-down", this, &AudioView::OnMiddleButtonDown);
	parent->EventMX("area", "hui:middle-button-up", this, &AudioView::OnMiddleButtonUp);
	parent->EventMX("area", "hui:right-button-down", this, &AudioView::OnRightButtonDown);
	parent->EventMX("area", "hui:right-button-up", this, &AudioView::OnRightButtonUp);
	//parent->EventMX("area", "hui:key-down", this, &AudioView::OnKeyDown);
	parent->EventMX("area", "hui:key-down", this, &AudioView::OnKeyDown);
	parent->EventMX("area", "hui:key-up", this, &AudioView::OnKeyUp);
	parent->EventMX("area", "hui:mouse-wheel", this, &AudioView::OnMouseWheel);

	//ForceRedraw();
	UpdateMenu();

	track_dialog = new TrackDialog(tsunami);
	audio_file_dialog = new AudioFileDialog(tsunami, audio);
}

AudioView::~AudioView()
{
	Unsubscribe(audio);
	Unsubscribe(tsunami->output);
	Unsubscribe(tsunami->input);

	HuiConfigWriteBool("View.Mono", show_mono);
	HuiConfigWriteInt("View.GridMode", grid_mode);
	HuiConfigWriteInt("View.DetailSteps", DetailSteps);
	HuiConfigWriteInt("View.MouseMinMoveToSelect", MouseMinMoveToSelect);
	HuiConfigWriteInt("View.ScrollSpeed", ScrollSpeed);
	HuiConfigWriteInt("View.ScrollSpeedFast", ScrollSpeedFast);
	HuiConfigWriteFloat("View.ZoomSpeed", ZoomSpeed);
	HuiConfigWriteBool("View.Antialiasing", Antialiasing);
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

int AudioView::MouseOverSample(SampleRef *s)
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

	// track?
	foreach(Track *t, audio->track){
		if (MouseOverTrack(t)){
			s.track = t;
			s.type = SEL_TYPE_TRACK;
			if (mx < t->area.x1 + 100)
				s.show_track_controls = t;
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

	// mute button?
	if (s.track){
		if ((mx >= s.track->area.x1 + 5) && (mx < s.track->area.x1 + 17) && (my >= s.track->area.y1 + 22) && (my < s.track->area.y1 + 34)){
			s.type = SEL_TYPE_MUTE;
			return s;
		}
		if ((audio->track.num > 1) && (mx >= s.track->area.x1 + 22) && (mx < s.track->area.x1 + 34) && (my >= s.track->area.y1 + 22) && (my < s.track->area.y1 + 34)){
			s.type = SEL_TYPE_SOLO;
			return s;
		}
	}

	// sub?
	if (s.track){
		// TODO: prefer selected subs
		foreach(SampleRef *ss, s.track->sample){
			int offset = MouseOverSample(ss);
			if (offset >= 0){
				s.sample = ss;
				s.type = SEL_TYPE_SAMPLE;
				s.sample_offset = offset;
			}
		}
	}

	return s;
}


void AudioView::SelectUnderMouse()
{
	msg_db_f("SelectUnderMouse", 2);
	Hover = GetMouseOver();
	Selection = Hover;
	Track *t = Selection.track;
	SampleRef *s = Selection.sample;
	bool control = tsunami->GetKey(KEY_CONTROL);

	// track
	if (Selection.track)
		SetCurTrack(Selection.track);
	if (Selection.type == SEL_TYPE_TRACK){
		SelectTrack(t, control);
		if (!control)
			audio->UnselectAllSubs();
	}

	// sub
	if (Selection.type == SEL_TYPE_SAMPLE){
		SelectSample(s, control);
		if (s->is_selected)
			SetCurSample(s);
	}
}

void AudioView::SetBarriers(SelectionType *s)
{
	msg_db_f("SetBarriers", 2);
	s->barrier.clear();
	if (s->type == SEL_TYPE_NONE)
		return;

	int dpos = 0;
	if (s->type == SEL_TYPE_SAMPLE)
		dpos = s->sample_offset;

	foreach(Track *t, audio->track){
		// add subs
		foreach(SampleRef *sam, t->sample){
			s->barrier.add(sam->pos + dpos);
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
}

void AudioView::ApplyBarriers(int &pos)
{
	msg_db_f("ApplyBarriers", 2);
	foreach(int b, Selection.barrier){
		int dpos = sample2screen(b) - sample2screen(pos);
		if (abs(dpos) <= BarrierDist){
			//msg_write(format("barrier:  %d  ->  %d", pos, b));
			pos = b;
		}
	}
}

bool hover_changed(AudioView::SelectionType &hover, AudioView::SelectionType &hover_old)
{
	return (hover.type != hover_old.type) || (hover.sample != hover_old.sample) || (hover.show_track_controls != hover_old.show_track_controls);
}

void AudioView::OnMouseMove()
{
	msg_db_f("OnMouseMove", 2);
	SetMouse();
	bool _force_redraw_ = false;

	if (HuiGetEvent()->lbut){
		SelectionUpdatePos(Selection);
	}else{
		SelectionType hover_old = Hover;
		Hover = GetMouseOver();
		_force_redraw_ |= hover_changed(Hover, hover_old);
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
	}else if (Selection.type == SEL_TYPE_SAMPLE){
		ApplyBarriers(Selection.pos);
		int dpos = (float)Selection.pos - Selection.sample_offset - Selection.sample->pos;
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
}



void AudioView::OnLeftButtonDown()
{
	msg_db_f("OnLBD", 2);
	SetMouse();
	SelectUnderMouse();
	UpdateMenu();

	if (!audio->used)
		return;

	// selection:
	//   start after lb down and moving
	if ((Selection.type == SEL_TYPE_TRACK) || (Selection.type == SEL_TYPE_TIME)){
		MousePossiblySelecting = 0;
		int pos = screen2sample(mx);
		audio->sel_raw = Range(pos, 0);
		audio->UpdateSelection();
		MousePossiblySelectingStart = pos;
	}else if (Selection.type == SEL_TYPE_SELECTION_START){
		// switch end / start
		Selection.type = SEL_TYPE_SELECTION_END;
		Hover.type = SEL_TYPE_SELECTION_END;
		audio->sel_raw.invert();
	}else if (Selection.type == SEL_TYPE_MUTE){
		Selection.track->SetMuted(!Selection.track->muted);
	}else if (Selection.type == SEL_TYPE_SOLO){
		foreach(Track *t, audio->track)
			t->is_selected = (t == Selection.track);
		if (Selection.track->muted)
			Selection.track->SetMuted(false);
	}else if (Selection.type == SEL_TYPE_SAMPLE){
		cur_action = new ActionTrackMoveSample(audio);
	}

	SetBarriers(&Selection);

	ForceRedraw();
	UpdateMenu();
}



void AudioView::OnLeftButtonUp()
{
	msg_db_f("OnLBU", 2);
	if (Selection.type == SEL_TYPE_SAMPLE)
		if (cur_action)
			audio->Execute(cur_action);
	cur_action = NULL;

	// TODO !!!!!!!!
	Selection.type = SEL_TYPE_NONE;
	ForceRedraw();
	UpdateMenu();
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
			if (Selection.type == SEL_TYPE_SAMPLE)
				ExecuteSubDialog(tsunami);
			else if (Selection.type == SEL_TYPE_TRACK)
				ExecuteTrackDialog(tsunami);
			else if (!Selection.track)
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
	int k = HuiGetEvent()->key_code;

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

	if (k == KEY_SPACE){
		if (tsunami->output->IsPlaying()){
			tsunami->output->Pause();
		}else
			tsunami->output->Play(audio, true);
	}
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

inline void draw_line_buffer(HuiPainter *c, int width, double view_pos, double zoom, float hf, float x, float y0, const Array<float> &buf, int offset)
{
	int nl = 0;
	int i0 = max((double) x          / zoom + view_pos - offset    , 0);
	int i1 = min((double)(x + width) / zoom + view_pos - offset + 2, buf.num);

	if (i1 - i0 > 4000)
		return;

	for (int i=i0;i<i1;i++){

		double p = x + ((double)(i + offset) + 0.5 - view_pos) * zoom;
		tx[nl] = (float)p;
		ty[nl] = y0 + buf[i] * hf;
		if (zoom > 5)
			c->DrawCircle(p, ty[nl], 2);
		nl ++;
	}
	c->DrawLines(tx, ty, nl -1);
}

inline void draw_peak_buffer(HuiPainter *c, int width, int di, double view_pos_rel, double zoom, float f, float hf, float x, float y0, const string &buf, int offset)
{
	int nl = 0;
	double dpos = 1.0 / zoom;
	// pixel position
	// -> buffer position
	double p0 = view_pos_rel;
	for (int i=0;i<width+di;i+=di){

		double p = p0 + dpos * (double)i + 0.5;
		int ip = (int)(p - offset)/f;
		if ((ip >= 0) && (ip < buf.num))
		if (((int)(p) < offset + buf.num*f) && (p >= offset)){
			tx[nl] = (float)x+i;
			float dy = ((float)((unsigned char)buf[ip])/255.0f) * hf;
			ty[nl]  = y0 + dy;
			//ty2[nl] = y0 - dy;
			nl ++;
		}
	}
	for (int i=0;i<nl;i++){
		tx[nl + i] = tx[nl - i - 1];
		ty[nl + i] = y0 *2 - ty[nl - i - 1] - 1;
	}
	c->DrawPolygon(tx, ty, nl*2);
//	c->DrawLines(tx, ty, nl*2 -1);
	//c->DrawLines(tx, ty2, nl -1);
}

void AudioView::UpdateBufferZoom()
{
	prefered_buffer_level = 0;
	buffer_zoom_factor = 1.0;

	// which level of detail?
	if (view_zoom < 0.8f)
		for (int i=24-1;i>=0;i--){
			double _f = pow(2, (double)i);
			if (_f > 1.0 / view_zoom){
				prefered_buffer_level = i;
				buffer_zoom_factor = _f;
			}
		}
}

void AudioView::DrawBuffer(HuiPainter *c, const rect &r, BufferBox &b, double view_pos_rel, const color &col)
{
	msg_db_f("DrawBuffer", 1);

	// zero heights of both channels
	float y0r = r.y1 + r.height() / 4;
	float y0l = r.y1 + r.height() * 3 / 4;

	float hf = r.height() / 4;

	if (show_mono){
		y0r = r.y1 + r.height() / 2;
		hf *= 2;
	}


	int di = DetailSteps;
	c->SetColor(col);
	int l = min(prefered_buffer_level - 1, b.peak.num / 2);
	if (l >= 1){//f < MIN_MAX_FACTOR){
		draw_peak_buffer(c, r.width(), di, view_pos_rel, view_zoom, buffer_zoom_factor, hf, r.x1, y0r, b.peak[l*2-2], b.offset);
		if (!show_mono)
			draw_peak_buffer(c, r.width(), di, view_pos_rel, view_zoom, buffer_zoom_factor, hf, r.x1, y0l, b.peak[l*2-1], b.offset);
	}else{
		draw_line_buffer(c, r.width(), view_pos_rel, view_zoom, hf, r.x1, y0r, b.r, b.offset);
		if (!show_mono)
			draw_line_buffer(c, r.width(), view_pos_rel, view_zoom, hf, r.x1, y0l, b.l, b.offset);
	}
}

void AudioView::DrawTrackBuffers(HuiPainter *c, const rect &r, Track *t, double view_pos_rel, const color &col)
{
	msg_db_f("DrawTrackBuffers", 1);

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
		color cc = ColorWave;
		if ((level_no == cur_level))// || (!t->parent))
			cc = col;
		foreach(BufferBox &b, lev.buffer)
			DrawBuffer(c, r, b, view_pos_rel, cc);
	}
}

void AudioView::DrawSampleFrame(HuiPainter *c, const rect &r, SampleRef *s, const color &col, int delay)
{
	// frame
	int asx = clampi(sample2screen(s->pos + delay), r.x1, r.x2);
	int aex = clampi(sample2screen(s->pos + s->buf.num + delay), r.x1, r.x2);

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

void AudioView::DrawSample(HuiPainter *c, const rect &r, SampleRef *s)
{
	color col = ColorSub;
	//bool is_cur = ((s == cur_sub) && (t->IsSelected));
	if (!s->is_selected)
		col = ColorSubNotCur;
	if (Hover.sample == s)
		col = ColorSubMO;
	//col.a = 0.2f;

	DrawSampleFrame(c, r, s, col, 0);

	color col2 = col;
	col2.a *= 0.5f;
	for (int i=0;i<s->rep_num;i++)
		DrawSampleFrame(c, r, s, col2, (i + 1) * s->rep_delay);

	// buffer
	DrawBuffer(c, r, s->buf, view_pos - (double)s->pos, col);

	int asx = clampi(sample2screen(s->pos), r.x1, r.x2);
	if (s->is_selected)//((is_cur) || (a->sub_mouse_over == s))
		c->DrawStr(asx, r.y2 - SUB_FRAME_HEIGHT, s->origin->name);
}

void DrawStrBg(HuiPainter *c, float x, float y, const string &str, const color &fg, const color &bg)
{
	color bg2 = bg;
	bg2.a = 0.6f;
	c->SetColor(bg2);
	c->DrawRect(x, y, c->GetStrWidth(str), FONT_SIZE * 1.5f);
	c->SetColor(fg);
	c->DrawStr(x, y, str);
}


void AudioView::DrawMidi(HuiPainter *c, const rect &r, MidiData &midi, color col)
{
	c->SetLineWidth(3.0f);
	foreach(MidiNote &n, midi){
		c->SetColor(SetColorHSB(1, (float)(n.pitch % 12) / 12.0f, 0.6f, 1));
		float x1 = sample2screen(n.range.offset);
		float x2 = sample2screen(n.range.end());
		float h = r.y2 - clampf((float)n.pitch / 80.0f - 0.3f, 0, 1) * r.height();
		//c->DrawRect(rect(x1, x2, r.y1, r.y2));
		c->DrawLine(x1, h, x2, h);
	}
	c->SetLineWidth(LINE_WIDTH);
}

void AudioView::DrawTrack(HuiPainter *c, const rect &r, Track *t, color col, int track_no)
{
	msg_db_f("DrawTrack", 1);

	DrawMidi(c, r, t->midi, col);

	DrawTrackBuffers(c, r, t, view_pos, col);

	foreach(SampleRef *s, t->sample)
		DrawSample(c, r, s);

	//c->SetColor((track_no == a->CurTrack) ? Black : ColorWaveCur);
//	c->SetColor(ColorWaveCur);
	c->SetFont("", -1, (t == cur_track), false);
	DrawStrBg(c, r.x1 + 23, r.y1 + 3, t->GetNiceName(), ColorWaveCur, ColorBackgroundCurWave);
	c->SetFont("", -1, false, false);

	if (t->type == t->TYPE_TIME)
		c->DrawImage(r.x1 + 5, r.y1 + 5, image_track_time);
	else if (t->type == t->TYPE_MIDI)
		c->DrawImage(r.x1 + 5, r.y1 + 5, image_track_midi);
	else
		c->DrawImage(r.x1 + 5, r.y1 + 5, image_track_audio);

	if ((t->muted) || (Hover.show_track_controls == t))
		c->DrawImage(r.x1 + 5, r.y1 + 22, t->muted ? image_muted : image_unmuted);
	if ((audio->track.num > 1) && (Hover.show_track_controls == t))
		c->DrawImage(r.x1 + 22, r.y1 + 22, image_solo);
}

void AudioView::DrawGrid(HuiPainter *c, const rect &r, const color &bg, bool show_time)
{
	if (grid_mode == GRID_MODE_TIME)
		DrawGridTime(c, r, bg, show_time);
	else if (grid_mode == GRID_MODE_BARS)
		DrawGridBars(c, r, bg, show_time);
}

void AudioView::DrawGridTime(HuiPainter *c, const rect &r, const color &bg, bool show_time)
{
	double dl = MIN_GRID_DIST / view_zoom; // >= 10 pixel
	double dt = dl / audio->sample_rate;
	double exp_s = ceil(log10(dt));
	double exp_s_mod = exp_s - log10(dt);
	dt = pow(10, exp_s);
	dl = dt * audio->sample_rate;
//	double dw = dl * a->view_zoom;
	int nx0 = floor(screen2sample(r.x1 - 1) / dl);
	int nx1 = ceil(screen2sample(r.x2) / dl);
	color c1 = ColorInterpolate(bg, ColorGrid, exp_s_mod);
	color c2 = ColorGrid;
	for (int n=nx0;n<nx1;n++){
		c->SetColor(((n % 10) == 0) ? c2 : c1);
		int xx = sample2screen(n * dl);
		c->DrawLine(xx, r.y1, xx, r.y2);
	}
	if (show_time){
		if ((tsunami->output->IsPlaying()) && (tsunami->output->GetAudio() == audio)){
			color cc = ColorPreviewMarker;
			cc.a = 0.25f;
			c->SetColor(cc);
			float x0 = sample2screen(tsunami->output->GetRange().start());
			float x1 = sample2screen(tsunami->output->GetRange().end());
			c->DrawRect(x0, r.y1, x1 - x0, r.y1 + TIME_SCALE_HEIGHT);
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

void AudioView::DrawGridBars(HuiPainter *c, const rect &r, const color &bg, bool show_time)
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
	color c1 = ColorInterpolate(bg, ColorGrid, 0.5f);
	foreach(Beat &b, beats){
		c->SetColor((b.beat_no == 0) ? ColorGrid : c1);
		int xx = sample2screen(b.pos);
		c->DrawLine(xx, r.y1, xx, r.y2);
	}
	if (!show_time)
		return;
	c->SetColor(ColorGrid);
	foreach(Beat &b, beats){
		if (b.beat_no == 0){
			int xx = sample2screen(b.pos);
			c->DrawStr(xx + 2, r.y1, i2s(b.bar_no + 1));
		}
	}
}

void AudioView::CheckConsistency()
{
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
}

void AudioView::OnUpdate(Observable *o)
{
	//msg_write("view: " + o->GetName() + " - " + o->GetMessage());

	CheckConsistency();

	if (o->GetName() == "AudioFile"){
		if (o->GetMessage() == "New"){
			OptimizeView();
		}else{
			ForceRedraw();
			UpdateMenu();
		}
	}else if (o->GetName() == "AudioOutput"){
		if ((tsunami->output->IsPlaying()) && (tsunami->output->GetAudio() == audio))
			MakeSampleVisible(tsunami->output->GetPos());
		ForceRedraw();
	}else if (o->GetName() == "AudioInput"){
		if (tsunami->input->IsCapturing())
			MakeSampleVisible(audio->selection.start() + tsunami->input->GetSampleCount());
		ForceRedraw();
	}
}

void plan_track_sizes(const rect &r, AudioFile *a, int TIME_SCALE_HEIGHT)
{
	int n_ch = tsunami->view->show_mono ? 1 : 2;

	int h_wish = TIME_SCALE_HEIGHT;
	int h_fix = TIME_SCALE_HEIGHT;
	int n_var = 0;
	foreach(Track *t, a->track){
		if (t->type == t->TYPE_AUDIO){
			h_wish += MAX_TRACK_CHANNEL_HEIGHT * n_ch;
			n_var += n_ch;
		}else if (t->type == t->TYPE_MIDI){
			h_wish += MAX_TRACK_CHANNEL_HEIGHT;
			n_var ++;
		}else{
			h_wish += TIME_SCALE_HEIGHT * 2;
			h_fix += TIME_SCALE_HEIGHT * 2;
		}
	}

	int y0 = r.y1 + TIME_SCALE_HEIGHT;
	int opt_channel_height = MAX_TRACK_CHANNEL_HEIGHT;
	if (h_wish > r.height())
		opt_channel_height = (r.height() - h_fix) / n_var;
	foreachi(Track *t, a->track, i){
		float h = TIME_SCALE_HEIGHT*2;
		if (t->type == t->TYPE_AUDIO)
			h = opt_channel_height * n_ch;
		else if (t->type == t->TYPE_MIDI)
			h = opt_channel_height;
		t->area = rect(r.x1, r.x2, y0, y0 + h);
		y0 += h;
	}
}

void AudioView::DrawTimeLine(HuiPainter *c, int pos, int type, color &col, bool show_time)
{
	int p = sample2screen(pos);
	if ((p >= audio->area.x1) && (p <= audio->area.x2)){
		c->SetColor((type == Hover.type) ? ColorSelectionBoundaryMO : col);
		c->DrawLine(p, audio->area.y1, p, audio->area.y2);
		if (show_time)
			c->DrawStr(p, (audio->area.y1 + audio->area.y2) / 2, audio->get_time_str(pos));
	}
}

void AudioView::DrawEmptyAudioFile(HuiPainter *c, const rect &r)
{
	color col = ColorBackgroundCurWave;
	c->SetColor(col);
	c->DrawRect(r.x1, r.y2, r.width(), r.height());
	c->SetColor(ColorWaveCur);
	c->SetFontSize(FONT_SIZE_NO_FILE);
	c->DrawStr(r.x1 + r.width() / 2 - 50, r.y1 + r.height() / 2 - 10, _("keine Datei"));
}

void AudioView::DrawBackground(HuiPainter *c, const rect &r)
{
	int yy = audio->track.back()->area.y2;

	// time scale
	c->SetColor(ColorBackgroundCurWave);
	c->DrawRect(r.x1, r.y1, r.width(), TIME_SCALE_HEIGHT);
	DrawGrid(c, rect(r.x1, r.x2, r.y1, r.y1 + TIME_SCALE_HEIGHT), ColorBackgroundCurWave, true);

	// tracks
	foreach(Track *t, audio->track){
		color cc = (t->is_selected) ? ColorBackgroundCurTrack : ColorBackgroundCurWave;
		c->SetColor(cc);
		c->DrawRect(t->area);
		if (t->type == t->TYPE_TIME)
			DrawGridBars(c, t->area, cc, grid_mode == GRID_MODE_TIME);
		else
			DrawGrid(c, t->area, cc);
	}

	// free space below tracks
	if (yy < r.y2){
		c->SetColor(ColorBackground);
		rect rr = rect(r.x1, r.x2, yy, r.y2);
		c->DrawRect(rr);
		DrawGrid(c, rr, ColorBackground, false);
	}

	// lines between tracks
	c->SetColor(ColorGrid);
	foreach(Track *t, audio->track)
		c->DrawLine(0, t->area.y1, r.width(), t->area.y1);
	if (yy < r.y2)
		c->DrawLine(0, yy, r.width(), yy);

	//DrawGrid(c, r, ColorBackgroundCurWave, true);
}

void AudioView::DrawSelection(HuiPainter *c, const rect &r)
{
	int sx1 = sample2screen(audio->selection.start());
	int sx2 = sample2screen(audio->selection.end());
	int sxx1 = clampi(sx1, r.x1, r.x2);
	int sxx2 = clampi(sx2, r.x1, r.x2);
	c->SetColor(ColorSelectionInternal);
	foreach(Track *t, audio->track)
		if (t->is_selected)
			c->DrawRect(rect(sxx1, sxx2, t->area.y1, t->area.y2));
	DrawTimeLine(c, audio->sel_raw.start(), SEL_TYPE_SELECTION_START, ColorSelectionBoundary);
	DrawTimeLine(c, audio->sel_raw.end(), SEL_TYPE_SELECTION_END, ColorSelectionBoundary);
}

void AudioView::DrawAudioFile(HuiPainter *c, const rect &r)
{
	audio->area = r;

	// empty file
	if (!audio->used){
		DrawEmptyAudioFile(c, r);
		return;
	}

	plan_track_sizes(r, audio, TIME_SCALE_HEIGHT);
	UpdateBufferZoom();

	// background
	DrawBackground(c, r);


	c->SetColor(ColorWaveCur);

	// tracks
	foreachi(Track *tt, audio->track, i)
		DrawTrack(c, tt->area, tt, ColorWaveCur, i);


	// selection
	DrawSelection(c, r);

	// playing position
	if ((tsunami->output->IsPlaying()) && (tsunami->output->GetAudio() == audio)){
		DrawTimeLine(c, tsunami->output->GetRange().start(), SEL_TYPE_PLAYBACK_START, ColorPreviewMarker);
		DrawTimeLine(c, tsunami->output->GetRange().end(), SEL_TYPE_PLAYBACK_END, ColorPreviewMarker);
		if (!tsunami->input->IsCapturing())
			DrawTimeLine(c, tsunami->output->GetPos(), SEL_TYPE_PLAYBACK, ColorPreviewMarker, true);
	}

	// capturing position
	if (tsunami->input->IsCapturing())
		DrawTimeLine(c, audio->selection.start() + tsunami->input->GetSampleCount(), SEL_TYPE_PLAYBACK, ColorCaptureMarker, true);
}

int frame=0;

void AudioView::OnDraw()
{
	msg_db_f("OnDraw", 1);
	force_redraw = false;

	HuiPainter *c = tsunami->BeginDraw("area");
	DrawingWidth = c->width;
	c->SetFontSize(FONT_SIZE);
	c->SetLineWidth(LINE_WIDTH);
	c->SetAntialiasing(Antialiasing);
	//c->SetColor(ColorWaveCur);

	DrawAudioFile(c, rect(0, c->width, 0, c->height));

	//c->DrawStr(100, 100, i2s(frame++));

	c->End();
}

void AudioView::OptimizeView()
{
	msg_db_f("OptimizeView", 1);
	if (audio->area.x2 <= 0)
		audio->area.x2 = DrawingWidth;

	Range r = audio->GetRange();

	if (r.num == 0)
		r.num = 10 * audio->sample_rate;
	int border = r.num * BORDER_FACTOR;
	r.offset -= border;
	r.num += border * 2;
	view_zoom = audio->area.width() / (double)r.length();
	view_pos = (double)r.start();
	ForceRedraw();
}

void AudioView::UpdateMenu()
{
	// view
	tsunami->Check("view_mono", show_mono);
	tsunami->Check("view_stereo", !show_mono);
	tsunami->Check("view_grid_time", grid_mode == GRID_MODE_TIME);
	tsunami->Check("view_grid_bars", grid_mode == GRID_MODE_BARS);
	tsunami->Check("view_peaks_max", PeakMode == BufferBox::PEAK_MODE_MAXIMUM);
	tsunami->Check("view_peaks_mean", PeakMode == BufferBox::PEAK_MODE_SQUAREMEAN);
	tsunami->Enable("zoom_in", audio->used);
	tsunami->Enable("zoom_out", audio->used);
	tsunami->Enable("view_optimal", audio->used);
	tsunami->Enable("view_samples", false);//tsunami->cur_audio->used);
}

void AudioView::SetPeaksMode(int mode)
{
	PeakMode = mode;
	if (audio->used){
		audio->InvalidateAllPeaks();
		audio->UpdatePeaks(PeakMode);
	}
	ForceRedraw();
	UpdateMenu();
}

void AudioView::SetShowMono(bool mono)
{
	show_mono = mono;
	ForceRedraw();
	//Notify("Settings");
	UpdateMenu();
}

void AudioView::ZoomIn()
{
	Zoom(2.0f);
}

void AudioView::ZoomOut()
{
	Zoom(0.5f);
}
void AudioView::MakeSampleVisible(int sample)
{
	double x = sample2screen(sample);
	if ((x > audio->area.x2) || (x < audio->area.x1)){
		view_pos = sample - audio->area.width() / view_zoom * BORDER_FACTOR;
		ForceRedraw();
	}
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
	SetCurSample(NULL);
}

void AudioView::SetGridMode(int mode)
{
	grid_mode = mode;
	ForceRedraw();
	UpdateMenu();
}



void AudioView::SelectSample(SampleRef *s, bool diff)
{
	if (!s)
		return;
	if (diff){
		s->is_selected = !s->is_selected;
	}else{
		if (!s->is_selected)
			s->owner->UnselectAllSubs();

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

void AudioView::SetCurSample(SampleRef *s)
{
	cur_sample = s;
}


void AudioView::SetCurTrack(Track *t)
{
	cur_track = t;
	track_dialog->SetTrack(cur_track);
}



double AudioView::screen2sample(double _x)
{
	return (_x - audio->area.x1) / view_zoom + view_pos;
}

double AudioView::sample2screen(double s)
{
	return audio->area.x1 + (s - view_pos) * view_zoom;
}

void AudioView::Zoom(float f)
{
	// max zoom: 20 pixel per sample
	double zoom_new = clampf(view_zoom * f, 0.000001, 20.0);

	view_pos += (mx - audio->area.x1) * (1.0/view_zoom - 1.0/zoom_new);
	view_zoom = zoom_new;
	ForceRedraw();
}

void AudioView::Move(float dpos)
{
	view_pos += dpos;
	ForceRedraw();
}

void AudioView::ExecuteTrackDialog(HuiWindow *win)
{
	win->HideControl("track_dialog_table", false);
	win->HideControl("audio_dialog_table", true);
}



void AudioView::ExecuteSubDialog(HuiWindow *win)
{
	if (!cur_sample){
		tsunami->log->Error(_("Kein Sample ausgew&ahlt"));
		return;
	}
	SubDialog *dlg = new SubDialog(win, false, cur_sample);
	dlg->Run();
}



void AudioView::ExecuteAudioDialog(HuiWindow *win)
{
	win->HideControl("track_dialog_table", true);
	win->HideControl("audio_dialog_table", false);
}


