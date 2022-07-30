#include "ViewModeEditAudio.h"
#include "../sidebar/AudioEditorConsole.h"
#include "../sidebar/SideBar.h"
#include "../audioview/AudioView.h"
#include "../audioview/graph/AudioViewLayer.h"
#include "../MouseDelayPlanner.h"
#include "../../data/base.h"
#include "../../data/TrackLayer.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../data/audio/BufferInterpolator.h"
#include "../../lib/fft/fft.h"
#include "../../lib/math/complex.h"
#include "../../TsunamiWindow.h"

struct RubberPoint {
	int source, target;
};

enum RubberHoverType {
	SOURCE,
	TARGET,
	NONE = -1
};

struct RubberData {
	Array<RubberPoint> points;
	int hover = -1;
	RubberHoverType hover_type = RubberHoverType::NONE;
	int selected = -1;
	RubberHoverType selected_type = RubberHoverType::NONE;
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

class MouseDelayRubberPoint : public MouseDelayAction {
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
		if (type == RubberHoverType::TARGET) {
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
	edit_mode = EditMode::SELECT;
	
	edit_radius = 50;
}


void ViewModeEditAudio::on_start() {
	set_side_bar(SideBar::AUDIO_EDITOR_CONSOLE);
}

void ViewModeEditAudio::on_end() {
}

void ViewModeEditAudio::on_key_down(int k) {
	if (k == hui::KEY_W)
		edit_radius *= 1.1f;
	if (k == hui::KEY_S)
		edit_radius = max(edit_radius / 1.1f, 5.0f);
	if (edit_mode == EditMode::RUBBER) {
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
	return RangeTo(view->sel.range().start()-r, view->sel.range().start()+r);
}

Range ViewModeEditAudio::range_target() {
	int r = cam->dscreen2sample(edit_radius);
	return RangeTo(view->hover().pos-r, view->hover().pos+r);
}

void draw_arrow(Painter *p, const vec2 &a, const vec2 &b) {
	float l = (b-a).length();
	if (l < 0.0001f)
		return;
	vec2 dir = (b-a) / l;
	vec2 e = vec2(dir.y, -dir.x);
	float r = min(l, 15.0f);
	p->draw_line(a, b);
	p->draw_line(b, b - r * (dir + e));
	p->draw_line(b, b - r * (dir - e));
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
	
	if ((edit_mode == EditMode::CLONE) or (edit_mode == EditMode::SMOOTHEN)) {
		// output
		view->cam.range2screen(range_target(), x1, x2);
		p->set_color(color(0.2f, 1, 0, 0));
		p->draw_rect(rect(x1, x2, y1, y2));
	}

	if (edit_mode == EditMode::RUBBER) {
		foreachi (auto &q, rubber.points, i) {
			// source
			color c = theme.red.with_alpha(0.7f);
			p->set_line_width(2);
			if ((rubber.hover_type == RubberHoverType::SOURCE) and (rubber.hover == i))
				c.a = 1; //theme.hoverify(c);
			if ((rubber.selected_type == RubberHoverType::SOURCE) and (rubber.selected == i))
				p->set_line_width(4);
			p->set_color(c);
			x1 = view->cam.sample2screen(q.source);
			p->draw_line({x1, y1}, {x1, y2});

			// target
			c = theme.green.with_alpha(0.7f);
			p->set_line_width(2);
			if ((rubber.hover_type == RubberHoverType::TARGET) and (rubber.hover == i))
				c.a = 1; //theme.hoverify(c);
			if ((rubber.selected_type == RubberHoverType::TARGET) and (rubber.selected == i))
				p->set_line_width(4);
			p->set_color(c);
			x2 = view->cam.sample2screen(q.target);
			p->draw_line({x2, y1}, {x2, y2});

			p->set_line_width(2);
			p->set_color(theme.text);
			draw_arrow(p, vec2(x1, y1 + (y2-y1)*0.25f), vec2(x2, y1 + (y2-y1)*0.25f));
			draw_arrow(p, vec2(x1, y1 + (y2-y1)*0.75f), vec2(x2, y1 + (y2-y1)*0.75f));
			p->set_line_width(1);
		}
	}
}

void ViewModeEditAudio::set_edit_mode(EditMode mode) {
	edit_mode = mode;
	view->force_redraw();
	notify();
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


void ViewModeEditAudio::on_mouse_move() {
	float mx = view->m.x;
	int smx = view->cam.screen2sample(mx);
	rubber.hover = -1;
	rubber.hover_type = RubberHoverType::NONE;

	if (!cur_vlayer()->is_cur_hover())
		return;

	if (edit_mode == EditMode::RUBBER) {
		foreachi(auto &q, rubber.points, i) {
			float sx = view->cam.sample2screen(q.source);
			float tx = view->cam.sample2screen(q.target);
			if (fabs(tx - mx) < 10) {
				rubber.hover = i;
				rubber.hover_type = RubberHoverType::TARGET;
			} else if (fabs(sx - mx) < 10) {
				rubber.hover = i;
				rubber.hover_type = RubberHoverType::SOURCE;
			}
		}
	}

	//view->force_redraw();
}

void ViewModeEditAudio::left_click_handle_void(AudioViewLayer *vlayer) {
	if (!editing(vlayer)) {
		ViewModeDefault::left_click_handle_void(vlayer);
		return;
	} else if (!view->sel.has(vlayer->layer)) {
		ViewModeDefault::left_click_handle_void(vlayer);
		return;
	}
	
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
	} else if (edit_mode == EditMode::RUBBER) {
		float mx = view->m.x;
		int smx = view->cam.screen2sample(mx);

		rubber.selected = rubber.hover;
		rubber.selected_type = rubber.hover_type;

		if (rubber.hover >= 0) {
			view->mdp_run(new MouseDelayRubberPoint(view, &rubber.points[rubber.hover], rubber.hover_type));
		} else if (!view->sel.range().is_inside(smx)) {
			ViewModeDefault::left_click_handle_void(vlayer);
			return;
		} else if (rubber.hover < 0) {
			rubber.points.add({smx, smx});
			rubber.hover = rubber.points.num - 1;
			rubber.hover_type = RubberHoverType::TARGET;
		}

	} else { // SELECT
		ViewModeDefault::left_click_handle_void(vlayer);
	}
}

void ca_split(Array<complex> &z, Array<float> &x, Array<float> &y) {
	x.resize(z.num);
	y.resize(z.num);
	for (int i=0; i<z.num; i++) {
		x[i] = z[i].x;
		y[i] = z[i].y;
	}
}

void ca_join(Array<complex> &z, Array<float> &x, Array<float> &y) {
	z.resize(x.num);
	for (int i=0; i<z.num; i++)
		z[i] = complex(x[i], y[i]);
}

void pitch_shift_channel(Array<float> &buf, float factor) {
	Array<complex> z, zz;
	fft::r2c(buf, z);

	Array<float> x, y;
	ca_split(z, x, y);

	Array<float> xx, yy;
	xx.resize(x.num * factor);
	yy.resize(y.num * factor);
	BufferInterpolator::interpolate_channel_cubic(x, xx);
	BufferInterpolator::interpolate_channel_cubic(y, yy);
	xx.resize(x.num);
	yy.resize(y.num);
	ca_join(zz, xx, yy);
	for (auto &e: zz)
		e /= buf.num * factor;

	/*zz.resize(z.num);

	for (int i=0; i<z.num; i++)
		zz[i] = complex(0,0);

	for (int i=0; i<z.num; i++) {
		int j = (float)i * factor;
		if (j >= z.num)
			break;
		zz[j] += z[i] / z.num / 2;
	}*/

	fft::c2r_inv(zz, buf);
}

void pitch_shift(AudioBuffer &buf, float factor) {
	for (int i=0; i<buf.channels; i++)
		pitch_shift_channel(buf.c[i], factor);
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

		Range rs = RangeTo(offset_s, next_s);
		Range rt = RangeTo(offset_t, next_t);
		//msg_write(rs.str() + " -> " + rt.str());

		AudioBuffer bs;
		bs.set_as_ref(buf, rs.offset, rs.length);

		AudioBuffer bt;
		bt.resize(rt.length);


		BufferInterpolator::interpolate(bs, bt, BufferInterpolator::Method::CUBIC);

		if (flag_pitch_compensate)
			pitch_shift(bt, (float)rt.length / (float)rs.length);
		out.set(bt, rt.offset);

		offset_s = next_s;
		offset_t = next_t;
	}
	buf.set(out, 0);
	cur_vlayer()->layer->edit_buffers_finish(a);
}

string ViewModeEditAudio::get_tip() {
	if (edit_mode == EditMode::RUBBER) {
		if (view->sel.range().is_empty())
			return "select range for stretching";
		return "EXPERIMENTAL    click in selection to add point    drag to move point    delete point [X]    clear [Shift+X]    apply [Return]    track [Alt+↑,↓]";
	}
	return "EXPERIMENTAL    radius [W,S]    track [Alt+↑,↓]";
}

