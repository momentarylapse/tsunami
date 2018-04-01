/*
 * SignalEditor.cpp
 *
 *  Created on: 30.03.2018
 *      Author: michi
 */

#include "SignalEditor.h"
#include "../AudioView.h"
#include "../../Session.h"
#include "../../Device/OutputStream.h"
#include "../../Device/InputStreamAudio.h"
#include "../../Device/InputStreamMidi.h"
#include "../../Plugins/Effect.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../../Audio/Source/SongRenderer.h"
#include "../../Audio/PeakMeter.h"

class ModuleSongRenderer : public SignalEditor::Module
{
public:
	SongRenderer *renderer;
	ModuleSongRenderer(SongRenderer *r)
	{
		renderer = r;
	}
	virtual string type(){ return "SongRenderer"; }
	virtual AudioSource *get_audio_source(){ return renderer; }
};

class ModulePeakMeter : public SignalEditor::Module
{
public:
	PeakMeter *peak_meter;
	ModulePeakMeter(PeakMeter *p)
	{
		peak_meter = p;
	}
	virtual string type(){ return "PeakMeter"; }
	virtual void set_audio_source(AudioSource *s){ if (peak_meter) peak_meter->setSource(s); }
};

class ModuleOutputStream : public SignalEditor::Module
{
public:
	OutputStream *stream;
	ModuleOutputStream(OutputStream *s)
	{
		stream = s;
	}
	virtual string type(){ return "OutputStream"; }
	virtual void set_audio_source(AudioSource *s){ if (stream) stream->setSource(s); }
};

SignalEditor::SignalEditor(Session *session) :
	BottomBar::Console(_("Signal Chain"), session)
{
	addDrawingArea("!expandx,expandy", 0, 0, "area");

	eventXP("area", "hui:draw", std::bind(&SignalEditor::onDraw, this, std::placeholders::_1));

	modules.add(new ModuleSongRenderer(view->renderer));
	modules.add(new ModulePeakMeter(view->peak_meter));
	modules.add(new ModuleOutputStream(view->stream));

	foreachi (Module *m, modules, i){
		m->x = 50 + i * 230;
		m->y = 50;
	}
}

SignalEditor::~SignalEditor()
{
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

	for (auto *m: modules){
		p->setColor(view->colors.text_soft1);
		p->setFill(false);
		p->drawRect(m->x, m->y, 160, 25);
		p->setFill(true);
		float ww = p->getStrWidth(m->type());
		p->drawStr(m->x + 80 - ww/2, m->y + 4, m->type());
	}
}
