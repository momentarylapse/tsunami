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

void SignalEditor::onDraw(Painter* p)
{
	int w = p->width;
	int h = p->height;
	p->setColor(view->colors.background);
	p->drawRect(0, 0, w, h);
	p->setFontSize(12);

	for (auto *m: chain->modules){
		p->setColor(view->colors.text_soft1);
		p->setFill(false);
		p->drawRect(m->x, m->y, 160, 25);
		p->setFill(true);
		float ww = p->getStrWidth(m->type());
		p->drawStr(m->x + 80 - ww/2, m->y + 4, m->type());
	}
}

void SignalEditor::onChainUpdate()
{
	redraw("area");
}
