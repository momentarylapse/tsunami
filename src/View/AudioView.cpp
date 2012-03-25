/*
 * AudioView.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "AudioView.h"
#include "../Tsunami.h"

AudioView::AudioView() :
	Observable("AudioView"),
	SUB_FRAME_HEIGHT(20),
	TIME_SCALE_HEIGHT(20)
{
	ColorBackground = White;
	ColorBackgroundCurWave = color(1, 0.93f, 0.93f, 1);
	ColorBackgroundCurTrack = color(1, 0.85f, 0.85f, 1);
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

	DetailSteps = HuiConfigReadInt("DetailSteps", 1);
	MouseMinMoveToSelect = HuiConfigReadInt("MouseMinMoveToSelect", 5);
	PreviewSleepTime = HuiConfigReadInt("PreviewSleepTime", 10);



	tsunami->AddDrawingArea("", 0, 0, 0, 0, "area");

	Subscribe(tsunami->audio[0]);
	Subscribe(tsunami->audio[1]);

	// events
	tsunami->EventMX("area", "hui:redraw", this, (void(HuiEventHandler::*)())&AudioView::OnDraw);
	tsunami->EventMX("area", "hui:mouse-move", this, (void(HuiEventHandler::*)())&AudioView::OnMouseMove);
	tsunami->EventMX("area", "hui:left-button-down", this, (void(HuiEventHandler::*)())&AudioView::OnLeftButtonDown);
	tsunami->EventMX("area", "hui:left-double-click", this, (void(HuiEventHandler::*)())&AudioView::OnLeftDoubleClick);
	tsunami->EventMX("area", "hui:left-button-up", this, (void(HuiEventHandler::*)())&AudioView::OnLeftButtonUp);
	tsunami->EventMX("area", "hui:middle-button-down", this, (void(HuiEventHandler::*)())&AudioView::OnMiddleButtonDown);
	tsunami->EventMX("area", "hui:middle-button-up", this, (void(HuiEventHandler::*)())&AudioView::OnMiddleButtonUp);
	tsunami->EventMX("area", "hui:right-button-down", this, (void(HuiEventHandler::*)())&AudioView::OnRightButtonDown);
	tsunami->EventMX("area", "hui:right-button-up", this, (void(HuiEventHandler::*)())&AudioView::OnRightButtonUp);
	//tsunami->EventMX("area", "hui:key-down", this, (void(HuiEventHandler::*)())&AudioView::OnKeyDown);
	tsunami->EventM("hui:key-down", this, (void(HuiEventHandler::*)())&AudioView::OnKeyDown);
	tsunami->EventMX("area", "hui:mouse-wheel", this, (void(HuiEventHandler::*)())&AudioView::OnMouseWheel);


	/*HuiAddCommandM("select_none", "", -1, this, (void(HuiEventHandler::*)())&AudioView::OnSelectNone);
	HuiAddCommandM("select_all", "", KEY_A + KEY_CONTROL, this, (void(HuiEventHandler::*)())&AudioView::OnSelectAll);
	HuiAddCommandM("select_nothing", "", -1, this, (void(HuiEventHandler::*)())&AudioView::OnSelectNothing);
	HuiAddCommandM("view_temp_file", "", -1, this, (void(HuiEventHandler::*)())&AudioView::OnViewTempFile);
	HuiAddCommandM("view_mono", "", -1, this, (void(HuiEventHandler::*)())&AudioView::OnViewMono);
	HuiAddCommandM("view_grid", "", -1, this, (void(HuiEventHandler::*)())&AudioView::OnViewGrid);
	HuiAddCommandM("view_optimal", "", -1, this, (void(HuiEventHandler::*)())&AudioView::OnViewOptimal);
	HuiAddCommandM("zoom_in", "", -1, this, (void(HuiEventHandler::*)())&AudioView::OnZoomIn);
	HuiAddCommandM("zoom_out", "", -1, this, (void(HuiEventHandler::*)())&AudioView::OnZoomOut);
	HuiAddCommandM("jump_other_file", "", -1, this, (void(HuiEventHandler::*)())&AudioView::OnJumpOtherFile);*/

	ForceRedraw();

	ShowTempFile = false;
	ShowMono = false;
	ShowGrid = true;
}

AudioView::~AudioView()
{
}

void AudioView::OnMouseWheel()
{
}



void AudioView::OnMiddleButtonUp()
{
}



void AudioView::OnCommand(const string & id)
{
}



void AudioView::OnMouseMove()
{
}



void AudioView::OnLeftButtonUp()
{
}



void AudioView::OnRightButtonUp()
{
}



void AudioView::OnMiddleButtonDown()
{
}


void AudioView::OnLeftDoubleClick()
{
}



void AudioView::OnKeyDown()
{
}



void AudioView::OnKeyUp()
{
}



void AudioView::OnRightButtonDown()
{
}



void AudioView::OnLeftButtonDown()
{
}


void AudioView::ForceRedraw()
{
	force_redraw = true;
	tsunami->Redraw("area");
}


#define MIN_GRID_DIST	10.0f

float tx[2048], ty[2048];

inline int fill_line_buffer(int width, int di, float pos, float zoom, float f, float hf, float x, float y0, const Array<float> &buf, int offset, float sign)
{
	int nl = 0;
	float dpos = (float)1/zoom/f;
	// pixel position
	// -> buffer position
	float p0 = pos / f;
	for (int i=0;i<width-1;i+=di){

		float p = p0 + dpos * (float)i;
		int ip = (int)p - offset;
		//printf("%f  %f\n", p1, p2);
		if ((ip >= 0) && (ip < buf.num))
		if (((int)(p * f) < offset + buf.num) && (p >= offset)){
			tx[nl] = (float)x+i;
			ty[nl] = y0 + buf[ip] * hf * sign;
//			msg_write(ip);
//			msg_write(f2s(buf[ip], 5));
			nl ++;
		}
		//p += dpos;
	}
	return nl - 1;
}

void AudioView::DrawBuffer(HuiDrawingContext *c, int x, int y, int width, int height, Track *t, int pos, float zoom, const color &col)
{
	msg_db_r("DrawBuffer", 1);
//	int l = 0;
	float f = 1.0f;

	// which level of detail?
	/*if (zoom < 0.8f)
		for (int i=NUM_PEAK_LEVELS-1;i>=0;i--){
			float _f = (float)pow(PeakFactor, (float)i);
			if (_f > 1.0f / zoom){
				l = i;
				f = _f;
			}
		}*/

	// zero heights of both channels
	float y0r = (float)y + (float)height / 4;
	float y0l = (float)y + (float)height * 3 / 4;

	float hf = (float)height / 4;

	if (ShowMono){
		y0r = (float)y + (float)height / 2;
		hf *= 2;
	}

	c->SetColor(col);

	int di = DetailSteps;
	int nl = 0;
	for (int i=0;i<t->buffer.num;i++){
	if (f < MIN_MAX_FACTOR){
		nl = fill_line_buffer(width, di, pos, zoom, f, hf, x, y0r, t->buffer[i].r, t->buffer[i].offset, -1);
		c->DrawLines(tx, ty, nl);
		if (!ShowMono){
			nl = fill_line_buffer(width, di, pos, zoom, f, hf, x, y0l, t->buffer[i].l, t->buffer[i].offset, -1);
			c->DrawLines(tx, ty, nl);
		}
	}else{
		nl = fill_line_buffer(width, di, pos, zoom, f, hf, x, y0r, t->buffer[i].r, t->buffer[i].offset, -1);
		c->DrawLines(tx, ty, nl);
		nl = fill_line_buffer(width, di, pos, zoom, f, hf, x, y0r, t->buffer[i].r, t->buffer[i].offset, +1);
		c->DrawLines(tx, ty, nl);
		if (!ShowMono){
			nl = fill_line_buffer(width, di, pos, zoom, f, hf, x, y0l, t->buffer[i].l, t->buffer[i].offset, -1);
			c->DrawLines(tx, ty, nl);
			nl = fill_line_buffer(width, di, pos, zoom, f, hf, x, y0l, t->buffer[i].l, t->buffer[i].offset, +1);
			c->DrawLines(tx, ty, nl);
		}
	}
	}
	msg_db_l(1);
}

int GetStrWidth(const string &s)
{
	return 80;
}

void AudioView::DrawSubFrame(HuiDrawingContext *c, int x, int y, int width, int height, Track *s, AudioFile *a, const color &col, int delay)
{
	// frame
	int asx = a->sample2screen(s->pos + delay);
	int aex = a->sample2screen(s->pos + s->length + delay);
	clampi(asx, x, x + width);
	clampi(aex, x, x + width);

	if (delay == 0){
		s->x = asx;
		s->width = aex - asx;
		s->y = y;
		s->height = height;
	}


	color col2 = col;
	col2.a *= 0.2f;
	c->SetColor(col2);
	c->DrawRect(asx, y,                             aex - asx, SUB_FRAME_HEIGHT);
	c->DrawRect(asx, y + height - SUB_FRAME_HEIGHT, aex - asx, SUB_FRAME_HEIGHT);

	c->SetColor(col);
	c->DrawLine(asx, y, asx, y + height);
	c->DrawLine(aex, y, aex, y + height);
	c->DrawLine(asx, y, aex, y);
	c->DrawLine(asx, y + height, aex, y + height);
}

void AudioView::DrawSub(HuiDrawingContext *c, int x, int y, int width, int height, Track *s, AudioFile *a)
{
	color col = ColorSub;
	//bool is_cur = ((s == cur_sub) && (t->IsSelected));
	if (!s->is_selected)
		col = ColorSubNotCur;
	if (s->is_mouse_over)
		col = ColorSubMO;
	//col.a = 0.2f;

	DrawSubFrame(c, x, y, width, height, s, a, col, 0);

	color col2 = col;
	col2.a *= 0.5f;
	for (int i=0;i<s->rep_num;i++)
		DrawSubFrame(c, x, y, width, height, s, a, col2, (i + 1) * s->rep_delay);

	// buffer
	if (a != tsunami->cur_audio)
		col = ColorWave;
	DrawBuffer(	c, x, y, width, height,
				s, int(a->view_pos - s->pos), a->view_zoom, col);

	int asx = a->sample2screen(s->pos);
	clampi(asx, x, x + width);
	if (s->is_selected)//((is_cur) || (a->sub_mouse_over == s))
		//NixDrawStr(asx, y + height/2 - 10, s->name);
		c->DrawStr(asx, y + height - SUB_FRAME_HEIGHT, s->name);
}

void AudioView::DrawBarCollection(HuiDrawingContext *c, int x, int y, int width, int height, Track *t, color col, AudioFile *a, int track_no, BarCollection *bc)
{
	int x0 = bc->pos;
	foreachi(bc->bar, bar, i){
		bar.x     = a->sample2screen(x0);
		bar.width = a->sample2screen(x0 + bar.length) - bar.x;
		if ((bar.x >= x) && (bar.x < x + width)){
			c->SetColor(col);
			c->DrawStr(bar.x + 2, y + height/2, i2s(i + 1));
		}
		for (int i=0;i<bar.num_beats;i++){
			color cc = (i == 0) ? Red : col;
			c->SetColor(cc);

			int bx = a->sample2screen(x0 + (int)((float)bar.length * i / bar.num_beats));

			if ((bx >= x) && (bx < x + width))
				c->DrawLine(bx, y, bx, y + height);
		}
		x0 += bar.length;
	}

	if (bc->bar.num > 0){
		bc->x = bc->bar[0].x;
		bc->width = bc->bar.back().x + bc->bar.back().width - bc->x;
	}
}

void AudioView::DrawTrack(HuiDrawingContext *c, int x, int y, int width, int height, Track *t, color col, AudioFile *a, int track_no)
{
	msg_db_r("DrawTrack", 1);
	t->x = x;
	t->width = width;


	//c->SetColor((track_no == a->CurTrack) ? Black : ColorWaveCur);
	c->SetColor(ColorWaveCur);
	c->SetFont("", -1, ((tsunami->cur_audio == a) && (track_no == a->cur_track)), (t->type == Track::TYPE_TIME));
	c->DrawStr(x + 10, y + height / 2 - 10, t->GetNiceName());
	c->SetFont("", -1, false, false);

	DrawBuffer(	c, x,y,width,height,
				t,int(a->view_pos),a->view_zoom,col);

	foreach(t->bar_col, bc)
		DrawBarCollection(c, x, y, width, height, t, col, a, track_no, &bc);

	foreach(t->sub, s)
		DrawSub(c, x, y, width, height, &s, a);
	msg_db_l(1);
}

void AudioView::DrawGrid(HuiDrawingContext *c, int x, int y, int width, int height, AudioFile *a, const color &bg, bool show_time)
{
	if (!ShowGrid)
		return;
	float dl = MIN_GRID_DIST / a->view_zoom; // >= 10 pixel
	float dt = dl / a->sample_rate;
	float exp_s = ceil(log10(dt));
	float exp_s_mod = exp_s - log10(dt);
	dt = pow(10, exp_s);
	dl = dt * a->sample_rate;
//	float dw = dl * a->view_zoom;
	int nx0 = a->screen2sample(x - 1) / dl + 1;
	int nx1 = a->screen2sample(x + width) / dl + 1;
	color c1 = ColorInterpolate(bg, ColorGrid, exp_s_mod);
	color c2 = ColorGrid;
	for (int n=nx0;n<nx1;n++){
		c->SetColor(((n % 10) == 0) ? c2 : c1);
		c->DrawLine(a->sample2screen(n * dl), y, a->sample2screen(n * dl), y + height);
	}
	if (show_time){
		c->SetColor(ColorGrid);
		for (int n=nx0;n<nx1;n++){
			if ((a->sample2screen(dl) - a->sample2screen(0)) > 30){
				if ((((n % 10) % 3) == 0) && ((n % 10) != 9) && ((n % 10) != -9))
					c->DrawStr(a->sample2screen(n * dl) + 2, y, a->get_time_str_fuzzy(n * dl, dt * 3));
			}else{
				if ((n % 10) == 0)
					c->DrawStr(a->sample2screen(n * dl) + 2, y, a->get_time_str_fuzzy(n * dl, dt * 10));
			}
		}
	}
}

void AudioView::OnUpdate(Observable *o)
{
	msg_write("view: " + o->GetName() + " - " + o->GetMessage());
	if (o->GetMessage() == "New")
		OptimizeView(dynamic_cast<AudioFile*>(o));
	else
		ForceRedraw();
}

void AudioView::DrawWaveFile(HuiDrawingContext *c, int x, int y, int width, int height, AudioFile *a)
{
	a->x = x;
	a->y = y;
	a->width = width;
	a->height = height;


	foreachi(a->track, t, i){
		t.y = (int)((float)y + TIME_SCALE_HEIGHT + (float)(height - TIME_SCALE_HEIGHT) / (float)a->track.num * i);
		t.height = (int)((float)y + TIME_SCALE_HEIGHT + (float)(height - TIME_SCALE_HEIGHT) / (float)a->track.num * (i + 1)) - t.y;
	}

	// background
	//int trackheight = (a->num_tracks > 0) ? (height / a->num_tracks) : height;
	if ((a == tsunami->cur_audio) && (a->used)){
		c->SetColor(ColorBackgroundCurWave);
		c->DrawRect(x, y, width, TIME_SCALE_HEIGHT);
		DrawGrid(c, x, y, width, TIME_SCALE_HEIGHT, a, ColorBackgroundCurWave, true);
		foreach(a->track, t){
			c->SetColor((t.is_selected) ? ColorBackgroundCurTrack : ColorBackgroundCurWave);
			c->DrawRect(x, t.y, width, t.height);
			DrawGrid(c, x, t.y, width, t.height, a, (t.is_selected) ? ColorBackgroundCurTrack : ColorBackgroundCurWave);
		}
	}else{
		color col = (a == tsunami->cur_audio) ? ColorBackgroundCurWave : ColorBackground;
		c->SetColor(col);
		c->DrawRect(x, y, width, height);
		if (a->used)
			DrawGrid(c, x, y, width, height, a, col, true);
	}

	if (!a->used){
		c->SetColor((a == tsunami->cur_audio) ? ColorWaveCur : ColorWave);
		c->SetFontSize(12);
		c->DrawStr(x + width / 2 - 50, y + height / 2 - 10, _("keine Datei"));
		return;
	}


	// selection
	if (a->selection){
		int sx1 = a->sample2screen(a->sel_start_raw);
		int sx2 = a->sample2screen(a->sel_end_raw);
		int sxx1 = sx1, sxx2 = sx2;
		clampi(sxx1, x, width + x);
		clampi(sxx2, x, width + x);
		bool mo_s = a->mo_sel_start;
		bool mo_e = a->mo_sel_end;
		if (sxx1 > sxx2){
			int t = sxx1;	sxx1 = sxx2;	sxx2 = t;
			//bool bt = mo_s;	mo_s = mo_e;	mo_e = bt; // TODO ???
		}
		foreach(a->track, t)
			if (t.is_selected){
				c->SetColor(ColorSelectionInternal);
				c->DrawRect(sxx1, t.y, sxx2 - sxx1, t.height);
				DrawGrid(c, sxx1, t.y, sxx2 - sxx1, t.height, a, ColorSelectionInternal);
			}
		if ((sx1>=x)&&(sx1<=x+width)){
			color col = mo_s ? ColorSelectionBoundaryMO : ColorSelectionBoundary;
			c->SetColor(col);
			c->DrawLine(sx1, y, sx1, y + height);
			//NixDrawStr(sx1,y,get_time_str(a->SelectionStart,a));
		}
		if ((sx2>=x)&&(sx2<=x+width)){
			color col = mo_e ? ColorSelectionBoundaryMO : ColorSelectionBoundary;
			c->SetColor(col);
			c->DrawLine(sx2, y, sx2, y + height);
			//NixDrawStr(sx2,y+height-TIME_SCALE_HEIGHT,get_time_str(a->SelectionEnd-a->SelectionStart,a));
		}
	}

	// TODO!!!!!!!!
	//NixDrawStr(x,y,get_time_str((int)a->ViewPos,a));
#if 0
	// playing position
	int pos = GetPreviewPos(a);
	if (pos != -1){
		int px = sample2screen(a, pos);
		c->SetColor(ColorPreviewMarker);
		c->DrawLine(px, y, px, y + height);
		c->DrawStr(px, y + height / 2, get_time_str(pos, a));
	}
#endif

	color col = (a==tsunami->cur_audio) ? ColorWaveCur : ColorWave;
	c->SetColor(col);

	// boundary of wave file
	/*if (a->min != a->max){
		c->DrawLine(sample2screen(a, a->min), y, sample2screen(a, a->min), y + height);
		c->DrawLine(sample2screen(a, a->max), y, sample2screen(a, a->max), y + height);
	}*/


	foreachi(a->track, tt, i)
		DrawTrack(c, x, tt.y, width, tt.height, &tt, col, a, i);

}

int frame=0;

void AudioView::OnDraw()
{
	msg_db_r("OnDraw", 1);
	force_redraw = false;

	HuiDrawingContext *c = tsunami->BeginDraw("area");
	DrawingWidth = c->width;
	c->SetFontSize(10);
	c->SetLineWidth(1.0f);
	c->SetAntialiasing(false);
	//c->SetColor(ColorWaveCur);

	int t0 = max(tsunami->audio[0]->track.num, 1);
	int t1 = max(tsunami->audio[1]->track.num, 1);
	float t = (float)t0 / (float)(t0 + t1);

	if (ShowTempFile){
		DrawWaveFile(c, 0, 0, c->width, c->height * t, tsunami->audio[0]);
		DrawWaveFile(c, 0, c->height * t, c->width, c->height * (1 - t), tsunami->audio[1]);
	}else
		DrawWaveFile(c, 0, 0, c->width, c->height, tsunami->audio[0]);

	//c->DrawStr(100, 100, i2s(frame++));

	c->End();

	msg_db_l(1);
}

void AudioView::OptimizeView(AudioFile *a)
{
	msg_db_r("OptimizeView", 1);
	if (a->width <= 0)
		a->width = DrawingWidth;

	int length = a->GetLength();
	if (length == 0)
		length = 10 * a->sample_rate;
	a->view_zoom = (float)a->width / (float)length;
	a->view_pos = (float)a->GetMin();
	ForceRedraw();
	msg_db_l(1);
}
