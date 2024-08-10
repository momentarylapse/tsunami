#include "ViewModeEditAudio.h"
#include "../sidebar/AudioEditorConsole.h"
#include "../sidebar/SideBar.h"
#include "../helper/Drawing.h"
#include "../audioview/AudioView.h"
#include "../audioview/graph/AudioViewLayer.h"
#include "../MouseDelayPlanner.h"
#include "../TsunamiWindow.h"
#include "../../data/base.h"
#include "../../data/TrackLayer.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../processing/audio/BufferInterpolator.h"
#include "../../processing/audio/BufferPitchShift.h"

namespace tsunami {

struct RubberPoint {
	int source, target;
};

enum RubberHoverType {
	Source,
	Target,
	None = -1
};

struct RubberData {
	Array<RubberPoint> points;
	int hover = -1;
	RubberHoverType hover_type = RubberHoverType::None;
	int selected = -1;
	RubberHoverType selected_type = RubberHoverType::None;
};
static RubberData rubber;

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

class MouseDelayRubberPoint : public scenegraph::MouseDelayAction {
public:
	AudioView *view;
	RubberPoint *p;
	RubberHoverType type;
	MouseDelayRubberPoint(AudioView *v, RubberPoint *_p, RubberHoverType _type) {
		view = v;
		p = _p;
		type = _type;
	}
	void on_start(const vec2 &m) override {
	}
	void on_update(const vec2 &m) override {
		if (type == RubberHoverType::Target) {
			p->target = view->cam.screen2sample(m.x);
		} else {
			int d = view->cam.screen2sample(m.x) - p->source;
			p->source += d;
			p->target += d;
		}
	}
	void on_draw_post(Painter *c) override {
		//c->draw_str({100, 100}, "MOVING");
	}
	void on_clean_up() override {
	}
};



ViewModeEditAudio::ViewModeEditAudio(AudioView *view) :
	ViewModeDefault(view)
{
	mode_name = "audio";
	edit_mode = EditMode::Select;
	
	edit_radius = 50;
}


void ViewModeEditAudio::on_start() {
	set_side_bar(SideBar::Index::AudioEditorConsole);
}

void ViewModeEditAudio::on_end() {
}

void ViewModeEditAudio::on_key_down(int k) {
	if (k == hui::KEY_W)
		edit_radius *= 1.1f;
	if (k == hui::KEY_S)
		edit_radius = max(edit_radius / 1.1f, 5.0f);
	if (edit_mode == EditMode::Rubber) {
		if (k == hui::KEY_X) {
			if (rubber.selected >= 0) {
				rubber.points.erase(rubber.selected);
				rubber.selected = rubber.hover = -1;;
			}
		}
		if (k == hui::KEY_X + hui::KEY_SHIFT) {
			rubber.points.clear();
			rubber.selected = rubber.hover = -1;
		}
		if (k == hui::KEY_RETURN)
			apply_stretch();
	}
	view->force_redraw();
}

float ViewModeEditAudio::layer_suggested_height(AudioViewLayer *l) {
	if (editing(l))
		return 200;
	return ViewModeDefault::layer_suggested_height(l);
}

Range ViewModeEditAudio::range_source() {
	if (!view->sel.range().is_empty())
		return view->sel.range();
	int r = cam->dscreen2sample(edit_radius);
	return Range::to(view->sel.range().start()-r, view->sel.range().start()+r);
}

Range ViewModeEditAudio::range_target() {
	int r = cam->dscreen2sample(edit_radius);
	return Range::to(view->hover().pos-r, view->hover().pos+r);
}

void ViewModeEditAudio::draw_post(Painter *p) {
	float y1 = view->cur_vlayer()->area.y1;
	float y2 = view->cur_vlayer()->area.y2;
	float x1, x2;
	
	if (edit_mode == EditMode::Clone) {
		// input
		view->cam.range2screen(range_source(), x1, x2);
		p->set_color(color(0.2f, 0, 1, 0));
		p->draw_rect(rect(x1, x2, y1, y2));
	}
	
	if ((edit_mode == EditMode::Clone) or (edit_mode == EditMode::Smoothen)) {
		// output
		view->cam.range2screen(range_target(), x1, x2);
		p->set_color(color(0.2f, 1, 0, 0));
		p->draw_rect(rect(x1, x2, y1, y2));
	}

	if (edit_mode == EditMode::Rubber) {
		foreachi (auto &q, rubber.points, i) {
			// source
			color c = theme.red.with_alpha(0.7f);
			p->set_line_width(2);
			if ((rubber.hover_type == RubberHoverType::Source) and (rubber.hover == i))
				c.a = 1; //theme.hoverify(c);
			if ((rubber.selected_type == RubberHoverType::Source) and (rubber.selected == i))
				p->set_line_width(4);
			p->set_color(c);
			x1 = view->cam.sample2screen(q.source);
			p->draw_line({x1, y1}, {x1, y2});

			// target
			c = theme.green.with_alpha(0.7f);
			p->set_line_width(2);
			if ((rubber.hover_type == RubberHoverType::Target) and (rubber.hover == i))
				c.a = 1; //theme.hoverify(c);
			if ((rubber.selected_type == RubberHoverType::Target) and (rubber.selected == i))
				p->set_line_width(4);
			p->set_color(c);
			x2 = view->cam.sample2screen(q.target);
			p->draw_line({x2, y1}, {x2, y2});

			p->set_line_width(2);
			p->set_color(theme.text);
			draw_arrow(p, vec2(x1, y1 + (y2-y1)*0.25f), vec2(x2, y1 + (y2-y1)*0.25f), 18);
			draw_arrow(p, vec2(x1, y1 + (y2-y1)*0.75f), vec2(x2, y1 + (y2-y1)*0.75f), 18);
			p->set_line_width(1);
		}
	}
}

void ViewModeEditAudio::set_edit_mode(EditMode mode) {
	edit_mode = mode;
	view->force_redraw();
	out_changed();
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


void ViewModeEditAudio::on_mouse_move(const vec2& m) {
	rubber.hover = -1;
	rubber.hover_type = RubberHoverType::None;

	if (!cur_vlayer()->is_cur_hover())
		return;

	if (edit_mode == EditMode::Rubber) {
		foreachi(auto &q, rubber.points, i) {
			float sx = view->cam.sample2screen(q.source);
			float tx = view->cam.sample2screen(q.target);
			if (fabs(tx - m.x) < 10) {
				rubber.hover = i;
				rubber.hover_type = RubberHoverType::Target;
			} else if (fabs(sx - m.x) < 10) {
				rubber.hover = i;
				rubber.hover_type = RubberHoverType::Source;
			}
		}
	}

	//view->force_redraw();
}

void ViewModeEditAudio::left_click_handle_void(AudioViewLayer *vlayer, const vec2 &m) {
	if (!editing(vlayer)) {
		ViewModeDefault::left_click_handle_void(vlayer, m);
		return;
	} else if (!view->sel.has(vlayer->layer)) {
		ViewModeDefault::left_click_handle_void(vlayer, m);
		return;
	}
	
	if (edit_mode == EditMode::Clone) {
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
	} else if (edit_mode == EditMode::Rubber) {
		int smx = view->cam.screen2sample(m.x);

		rubber.selected = rubber.hover;
		rubber.selected_type = rubber.hover_type;

		if (rubber.hover >= 0) {
			view->mdp_run(new MouseDelayRubberPoint(view, &rubber.points[rubber.hover], rubber.hover_type));
		} else if (!view->sel.range().is_inside(smx)) {
			ViewModeDefault::left_click_handle_void(vlayer, m);
			return;
		} else if (rubber.hover < 0) {
			rubber.points.add({smx, smx});
			rubber.hover = rubber.points.num - 1;
			rubber.hover_type = RubberHoverType::Target;
		}

	} else { // SELECT
		ViewModeDefault::left_click_handle_void(vlayer, m);
	}
}

void ViewModeEditAudio::apply_stretch() {

	AudioBuffer buf;

	auto r = view->sel.range();
	auto *a = cur_vlayer()->layer->edit_buffers(buf, r);

	AudioBuffer out;
	out.resize(buf.length);

	Array<RubberPoint> points = rubber.points;
	for (int i=0; i<points.num; i++)
		for (int j=i+1; j<points.num; j++)
			if (points[i].source > points[j].source)
				points.swap(i, j);

	int offset_s = 0;
	int offset_t = 0;
	for (int i=0; i<points.num+1; i++) {
		int next_s = buf.length;
		int next_t = buf.length;
		if (i < points.num) {
			next_s = points[i].source - r.offset;
			next_t = points[i].target - r.offset;
		}

		Range rs = Range::to(offset_s, next_s);
		Range rt = Range::to(offset_t, next_t);
		//msg_write(rs.str() + " -> " + rt.str());

		AudioBuffer bs;
		bs.set_as_ref(buf, rs.offset, rs.length);

		AudioBuffer bt;
		bt.resize(rt.length);


		BufferInterpolator::interpolate(bs, bt, BufferInterpolator::Method::Cubic);

		if (flag_pitch_compensate)
			BufferPitchShift::pitch_shift(bt, (float)rt.length / (float)rs.length);
		out.set(bt, rt.offset);

		offset_s = next_s;
		offset_t = next_t;
	}
	buf.set(out, 0);
	cur_vlayer()->layer->edit_buffers_finish(a);
}

string ViewModeEditAudio::get_tip() {
	if (edit_mode == EditMode::Rubber) {
		if (view->sel.range().is_empty())
			return "select range for stretching";
		return "EXPERIMENTAL    click in selection to add point    drag to move point    delete point [X]    clear [Shift+X]    apply [Return]    track [Alt+↑,↓]";
	}
	return "EXPERIMENTAL    radius [W,S]    track [Alt+↑,↓]";
}

}

