/*
 * LogNotifier.cpp
 *
 *  Created on: 27.03.2023
 *      Author: michi
 */

#include "LogNotifier.h"
#include "../audioview/AudioView.h"
#include "../ColorScheme.h"
#include "../helper/Drawing.h"
#include "../TsunamiWindow.h"
#include "../../Session.h"
#include "../../stuff/Log.h"
#include "../../lib/image/Painter.h"

namespace tsunami {

class LogInfoBox : public scenegraph::NodeFree {
public:
	explicit LogInfoBox(Session *_session) : scenegraph::NodeFree() {
		session = _session;
		align.horizontal = AlignData::Mode::Fill;
		align.vertical = AlignData::Mode::Bottom;
		align.dy = -200;
		align.h = 120;
		align.dz = 1000;
		set_perf_name("info-box");
	}

	bool on_left_button_down(const vec2 &m) override {
		// FIXME this is ugly...
		if (message.responses.num == 1) {
			hui::Event e = hui::Event(message.responses[0].explode(":")[0], "hui::activate");
			e.is_default = true;
			session->win->_send_event_(&e);
		}
		message.ttl = -1;
		hidden = true;
		return true;
	}
	bool on_right_button_down(const vec2 &m) override {
		return true;
	}
	bool allow_handle_click_when_gaining_focus() const override { return false; }

	void on_draw(Painter *p) override {
		if (message.ttl < 0)
			return;
		string header;
		string msg = message.text;
		float alpha = clamp(message.ttl / 2.0f, 0.0f, 1.0f);
		color c = theme.background_overlay;
		if (message.type == Log::Type::Error) {
			header = "Error";
			c = color::interpolate(theme.background, Red, 0.3f);
		} else if (message.type == Log::Type::Question) {
			header = "Question";
			c = color::interpolate(theme.background, Orange, 0.3f);
		} else {
			header = "Info";
			//c = color::interpolate(theme.background, Orange, 0.3f);
		}

		// multi-line -> header/msg
		auto xx = msg.explode("\n");
		if (xx.num > 1) {
			header = xx[0];
			msg = xx[1];
		}

		if (is_cur_hover())
			c = theme.hoverify(c);
		p->set_color(c.with_alpha(alpha));
		p->draw_rect(area);
		p->set_color(theme.text.with_alpha(alpha));
		p->set_font_size(17);
		p->draw_str(area.center() - vec2(p->get_str_width(header) / 2, 25),  header);
		p->set_font_size(13);
		p->draw_str(area.center() - vec2(p->get_str_width(msg) / 2, -10),  msg);
		p->set_font_size(theme.FONT_SIZE);

	}
	HoverData get_hover_data(const vec2 &m) override {
		auto h = session->view->hover_time(m);
		h.node = this;
		return h;
	}

	bool progress(float dt) {
		if (message.ttl > 0) {
			message.ttl -= dt;
			return true;
		}
		hidden = true;
		return false;
	}

	Session *session;
	struct Message {
		string text;
		Log::Type type;
		Array<string> responses;
		float ttl = -1;
	};
	Message message;
	void add_message(const Log::Message& m) {
		message.text = m.text;
		message.ttl = 10;
		message.type = m.type;
		message.responses = m.responses;
		hidden = false;
		request_redraw();
	}
};


LogNotifier::LogNotifier(Session *_session) : scenegraph::NodeFree() {
	session = _session;
	align.horizontal = AlignData::Mode::Fill;
	align.vertical = AlignData::Mode::Fill;
	align.dz = 1000;
	set_perf_name("notifier");

	info_box = new LogInfoBox(session);
	add_child(info_box);

	session->log->out_add_message >> create_sink([this] {
		auto m = session->log->latest(session);
		if (m.type == Log::Type::Status)
			set_status(m.text);
		else if (m.type == Log::Type::Error or m.type == Log::Type::Question)
			info_box->add_message(m);
	});
}

LogNotifier::~LogNotifier() {
	session->log->unsubscribe(this);
}

void LogNotifier::on_draw(Painter* p) {
	if (status.ttl > 0)
		draw_status(p, status);
}

void LogNotifier::draw_status(Painter *c, Message &m) {
	float a = min(m.ttl*8, 1.0f);
	a = pow(a, 0.4f);
	color c1 = theme.high_contrast_a.with_alpha(a);
	color c2 = theme.high_contrast_b.with_alpha(a);
	c->set_font_size(theme.FONT_SIZE * 1.3f * m.size * a);
	draw_boxed_str(c, area.center(), m.text, c1, c2, TextAlign::Center);
	c->set_font_size(theme.FONT_SIZE);
}

bool LogNotifier::progress(float dt) {
	bool animating = info_box->progress(dt);
	if (status.ttl > 0) {
		status.ttl -= 0.03f;
		animating = true;
	}
	return animating;
}

void LogNotifier::set_status(const string& text, float size) {
	status.text = text;
	status.ttl = 0.2f + (float)text.num * 0.1f;
	status.size = size;
	request_redraw();
}

}
