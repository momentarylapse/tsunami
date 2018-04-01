/*
 * SignalEditor.cpp
 *
 *  Created on: 30.03.2018
 *      Author: michi
 */

#include "SignalEditor.h"
#include "../AudioView.h"
#include "../../Session.h"
#include "../../Stuff/SignalChain.h"


SignalEditor::SignalEditor(Session *session) :
	BottomBar::Console(_("Signal Chain"), session)
{
	addDrawingArea("!expandx,expandy", 0, 0, "area");

	eventXP("area", "hui:draw", std::bind(&SignalEditor::onDraw, this, std::placeholders::_1));

	chain = session->signal_chain;
	chain->subscribe(this, std::bind(&SignalEditor::onChainUpdate, this));
}

SignalEditor::~SignalEditor()
{
	chain->unsubscribe(this);
}

void SignalEditor::onLeftButtonDown()
{
}

void SignalEditor::onLeftButtonUp()
{
}

void SignalEditor::onMouseMove()
{
}

const float MODULE_WIDTH = 160;
const float MODULE_HEIGHT = 25;

static rect module_rect(SignalChain::Module *m)
{
	return rect(m->x, m->x + MODULE_WIDTH, m->y, m->y + MODULE_HEIGHT);
}

static color signal_color(int type)
{
	if (type == Track::Type::AUDIO)
		return Red;
	if (type == Track::Type::MIDI)
		return Green;
	return White;
}

void SignalEditor::onDraw(Painter* p)
{
	int w = p->width;
	int h = p->height;
	p->setColor(view->colors.background);
	p->drawRect(0, 0, w, h);
	p->setFontSize(12);

	for (auto *m: chain->modules){
		p->setColor(view->colors.background_track_selected);
		p->setRoundness(view->CORNER_RADIUS);
		p->drawRect(module_rect(m));
		p->setRoundness(0);
		float ww = p->getStrWidth(m->type());
		p->setColor(view->colors.text_soft1);
		p->drawStr(m->x + MODULE_WIDTH/2 - ww/2, m->y + 4, m->type());

		foreachi(int t, m->port_in, i){
			p->setColor(signal_color(t));
			p->drawCircle(m->x - 5, m->y + MODULE_HEIGHT/2 + i*15, 4);
		}
		foreachi(int t, m->port_out, i){
			p->setColor(signal_color(t));
			p->drawCircle(m->x + MODULE_WIDTH + 5, m->y + MODULE_HEIGHT/2 + i*15, 4);
		}
	}

	for (auto *c: chain->cables){
		float x0 = c->source->x + MODULE_WIDTH + 5;
		float y0 = c->source->y + MODULE_HEIGHT/2;
		float x1 = c->target->x - 5;
		float y1 = c->target->y + MODULE_HEIGHT/2;
		p->setColor(signal_color(c->type));
		p->drawLine(x0, y0, x1, y1);
	}
}

void SignalEditor::onChainUpdate()
{
	redraw("area");
}
