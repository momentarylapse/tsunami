#include "ViewModeEditAudio.h"
#include "../SideBar/AudioEditorConsole.h"
#include "../AudioView.h"
#include "../Node/AudioViewLayer.h"
#include "../../Data/base.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/Audio/AudioBuffer.h"


/*class MouseDelayClone : public MouseDelayAction {
public:
	AudioView *view;
	int start_pos;
	MouseDelayClone(AudioView *v) {
		view = v;
	}
	void on_start() override {
	}
	void on_update() override {
	}
	void on_draw_post(Painter *c) override {
	}
	void on_clean_up() override {
	}
};*/




ViewModeEditAudio::ViewModeEditAudio(AudioView *view) :
	ViewModeDefault(view)
{
	edit_mode = EditMode::SELECT;
	side_bar_console = SideBar::AUDIO_EDITOR_CONSOLE;
	
	edit_radius = 50;
}

ViewModeEditAudio::~ViewModeEditAudio() {
}


void ViewModeEditAudio::on_start() {
}
void ViewModeEditAudio::on_end() {
}

void ViewModeEditAudio::on_key_down(int k) {
	if (k == hui::KEY_W)
		edit_radius *= 1.1f;
	if (k == hui::KEY_S)
		edit_radius = max(edit_radius / 1.1f, 5.0f);
	view->force_redraw();
}

float ViewModeEditAudio::layer_suggested_height(AudioViewLayer *l) {
	if (editing(l))
		return 200;
	return ViewModeDefault::layer_suggested_height(l);
}

void ViewModeEditAudio::on_cur_layer_change() {
}

Range ViewModeEditAudio::range_source() {
	int r = cam->dscreen2sample(edit_radius);
	return RangeTo(view->sel.range().start()-r, view->sel.range().start()+r);
}

Range ViewModeEditAudio::range_target() {
	int r = cam->dscreen2sample(edit_radius);
	return RangeTo(view->hover().pos-r, view->hover().pos+r);
}

void ViewModeEditAudio::draw_post(Painter *p) {
	float y1 = view->cur_vlayer()->area.y1;
	float y2 = view->cur_vlayer()->area.y2;
	float x1, x2;
	
	if (edit_mode == EditMode::CLONE) {
		// input
		view->cam.range2screen(range_source(), x1, x2);
		p->set_color(color(0.2f, 0, 1, 0));
		p->draw_rect(rect(x1, x2, y1, y2));
	}
	
	if (edit_mode != EditMode::SELECT) {
		// output
		view->cam.range2screen(range_target(), x1, x2);
		p->set_color(color(0.2f, 1, 0, 0));
		p->draw_rect(rect(x1, x2, y1, y2));
	}
}

void ViewModeEditAudio::set_edit_mode(EditMode mode) {
	edit_mode = mode;
}



AudioViewLayer *ViewModeEditAudio::cur_vlayer() {
	return view->cur_vlayer();
}
TrackLayer *ViewModeEditAudio::cur_layer() {
	return view->cur_layer();
}
bool ViewModeEditAudio::editing(AudioViewLayer *l) {
	return l == cur_vlayer();
}


float step(float t) {
	return 1 - 1/(exp((t-0.5)*12) + 1);
}

void ViewModeEditAudio::left_click_handle_void(AudioViewLayer *vlayer) {
	if (!editing(vlayer))
		return;
	
	if (edit_mode == EditMode::CLONE) {
		AudioBuffer source;
		source.resize(range_source().length);
		vlayer->layer->read_buffers_fixed(source, range_source());
		
		AudioBuffer out;
		auto *a = vlayer->layer->edit_buffers(out, range_target());
		int N = out.length;
		int ll = min(2000, N/4);
		for (int c=0; c<out.channels; c++)
			for (int i=0; i<N; i++) {
				float a = 1;
				if (i < ll)
					a = step((float)i / (float)ll);
				else if (i > N - ll)
					a = step((float)(N - i) / (float)ll);
				out.c[c][i] = a * source.c[c][i] + (1-a) * out.c[c][i];
			}
		vlayer->layer->edit_buffers_finish(a);
	}
}

string ViewModeEditAudio::get_tip() {
	return "AAAAAAAAAAA  audio editing!!!!!!    W,S -> radius";
}

