/*
 * SignalEditor.cpp
 *
 *  Created on: 30.03.2018
 *      Author: michi
 */

#include "SignalEditor.h"

#include "../../Midi/MidiPort.h"
#include "../AudioView.h"
#include "../../Session.h"
#include "../../Plugins/PluginManager.h"
#include "../../Module/Module.h"
#include "../Dialog/ConfigurableSelectorDialog.h"
#include "../../lib/math/complex.h"
#include "../../Module/SignalChain.h"
#include "../../TsunamiWindow.h"
#include "../SideBar/SideBar.h"
#include "../SideBar/ModuleConsole.h"



void apply_sel(SignalEditor *e)
{
	SignalChain::_Module *m = (SignalChain::_Module*)e->sel.module;
	Module *c = NULL;
	if (m)
		c = m->configurable();
	e->session->win->side_bar->module_console->setModule(c);

}

SignalEditor::Selection::Selection()
{
	type = -1;
	module = NULL;
	port = port_type = -1;
	target_module = NULL;
	target_port = -1;
	dx = dy = 0;
}

SignalEditor::SignalEditor(Session *session) :
	BottomBar::Console(_("Signal Chain"), session)
{
	addDrawingArea("!expandx,expandy,grabfocus", 0, 0, "area");

	menu_chain = hui::CreateResourceMenu("popup_signal_chain_menu");
	menu_module = hui::CreateResourceMenu("popup_signal_module_menu");

	eventXP("area", "hui:draw", std::bind(&SignalEditor::onDraw, this, std::placeholders::_1));
	eventX("area", "hui:mouse-move", std::bind(&SignalEditor::onMouseMove, this));
	eventX("area", "hui:left-button-down", std::bind(&SignalEditor::onLeftButtonDown, this));
	eventX("area", "hui:left-button-up", std::bind(&SignalEditor::onLeftButtonUp, this));
	eventX("area", "hui:right-button-down", std::bind(&SignalEditor::onRightButtonDown, this));
	eventX("area", "hui:key-down", std::bind(&SignalEditor::onKeyDown, this));
	event("signal_chain_add_audio_source", std::bind(&SignalEditor::onAddAudioSource, this));
	event("signal_chain_add_audio_effect", std::bind(&SignalEditor::onAddAudioEffect, this));
	event("signal_chain_add_audio_input", std::bind(&SignalEditor::onAddAudioInputStream, this));
	event("signal_chain_add_audio_joiner", std::bind(&SignalEditor::onAddAudioJoiner, this));
	event("signal_chain_add_midi_source", std::bind(&SignalEditor::onAddMidiSource, this));
	event("signal_chain_add_midi_effect", std::bind(&SignalEditor::onAddMidiEffect, this));
	event("signal_chain_add_midi_input", std::bind(&SignalEditor::onAddMidiInputStream, this));
	event("signal_chain_add_synthesizer", std::bind(&SignalEditor::onAddSynthesizer, this));
	event("signal_chain_add_pitch_detector", std::bind(&SignalEditor::onAddPitchDetector, this));
	event("signal_chain_add_beat_source", std::bind(&SignalEditor::onAddBeatSource, this));
	event("signal_chain_add_beat_midifier", std::bind(&SignalEditor::onAddBeatMidifier, this));
	event("signal_chain_reset", std::bind(&SignalEditor::onReset, this));
	event("signal_chain_load", std::bind(&SignalEditor::onLoad, this));
	event("signal_chain_save", std::bind(&SignalEditor::onSave, this));
	event("signal_module_delete", std::bind(&SignalEditor::onModuleDelete, this));
	event("signal_module_configure", std::bind(&SignalEditor::onModuleConfigure, this));

	chain = session->signal_chain;
	chain->subscribe(this, std::bind(&SignalEditor::onChainUpdate, this));
}

SignalEditor::~SignalEditor()
{
	chain->unsubscribe(this);
	delete menu_chain;
	delete menu_module;
}

void SignalEditor::onLeftButtonDown()
{
	hover = getHover(hui::GetEvent()->mx, hui::GetEvent()->my);
	sel = hover;
	apply_sel(this);
	if (sel.type == sel.TYPE_PORT_IN){
		chain->disconnect_target((SignalChain::_Module*)sel.module, sel.port);
	}else if (sel.type == sel.TYPE_PORT_OUT){
		chain->disconnect_source((SignalChain::_Module*)sel.module, sel.port);
	}
	redraw("area");
}

void SignalEditor::onLeftButtonUp()
{
	if (sel.type == sel.TYPE_PORT_IN or sel.type == sel.TYPE_PORT_OUT){
		if (hover.target_module){
			if (sel.type == sel.TYPE_PORT_IN){
				chain->disconnect_source((SignalChain::_Module*)hover.target_module, hover.target_port);
				chain->connect((SignalChain::_Module*)hover.target_module, hover.target_port, (SignalChain::_Module*)sel.module, sel.port);
			}else if (sel.type == sel.TYPE_PORT_OUT){
				chain->disconnect_target((SignalChain::_Module*)hover.target_module, hover.target_port);
				chain->connect((SignalChain::_Module*)sel.module, sel.port, (SignalChain::_Module*)hover.target_module, hover.target_port);
			}
		}
		sel = Selection();
		apply_sel(this);
	}
	redraw("area");
}

void SignalEditor::onMouseMove()
{
	float mx = hui::GetEvent()->mx;
	float my = hui::GetEvent()->my;
	if (hui::GetEvent()->lbut){
		if (sel.type == sel.TYPE_MODULE){
			auto *m = (SignalChain::_Module*)sel.module;
			m->x = mx + sel.dx;
			m->y = my + sel.dy;
		}else if (sel.type == sel.TYPE_PORT_IN or sel.type == sel.TYPE_PORT_OUT){
			hover.target_module = NULL;
			auto h = getHover(mx, my);
			if (h.module != sel.module and h.port_type == sel.port_type){
				if (h.type == sel.TYPE_PORT_IN and sel.type == sel.TYPE_PORT_OUT){
					hover.target_module = h.module;
					hover.target_port = h.port;
				}
				if (h.type == sel.TYPE_PORT_OUT and sel.type == sel.TYPE_PORT_IN){
					hover.target_module = h.module;
					hover.target_port = h.port;
				}
			}
		}
	}else{
		hover = getHover(mx, my);
	}
	redraw("area");
}

const float MODULE_WIDTH = 160;
const float MODULE_HEIGHT = 25;

static rect module_rect(SignalChain::_Module *m)
{
	return rect(m->x, m->x + MODULE_WIDTH, m->y, m->y + MODULE_HEIGHT);
}

static float module_port_in_x(SignalChain::_Module *m)
{
	return m->x - 5;
}

static float module_port_in_y(SignalChain::_Module *m, int index)
{
	return m->y + MODULE_HEIGHT/2 + (index - (float)(m->port_in.num-1)/2)*20;
}

static float module_port_out_x(SignalChain::_Module *m)
{
	return m->x + MODULE_WIDTH + 5;
}

static float module_port_out_y(SignalChain::_Module *m, int index)
{
	return m->y + MODULE_HEIGHT/2 + (index - (float)(m->port_out.num-1)/2)*20;
}

static color signal_color(int type)
{
	if (type == Track::Type::AUDIO)
		return Red;
	if (type == Track::Type::MIDI)
		return Green;
	if (type == Track::Type::TIME)
		return Blue;
	return White;
}

void SignalEditor::onRightButtonDown()
{
	int mx = hui::GetEvent()->mx;
	int my = hui::GetEvent()->my;
	hover = getHover(mx, my);
	sel = hover;
	apply_sel(this);

	if (hover.type == hover.TYPE_MODULE){
		menu_module->openPopup(this, mx, my);
	}else if (hover.type < 0){
		menu_chain->openPopup(this, mx, my);
	}
}

void SignalEditor::onKeyDown()
{
	int key = hui::GetEvent()->key_code;
	if (key == hui::KEY_DELETE){
		if (sel.type == sel.TYPE_MODULE){
			chain->remove((SignalChain::_Module*)sel.module);
			hover = sel = Selection();
		}
	}
}

void SignalEditor::onAddAudioSource()
{
	auto *dlg = new ConfigurableSelectorDialog(win, Module::Type::AUDIO_SOURCE, session);
	dlg->run();
	if (dlg->_return.num > 0){
		auto *m = chain->addAudioSource(dlg->_return);
		m->x = sel.dx;
		m->y = sel.dy;
	}
	delete(dlg);
}

void SignalEditor::onAddAudioEffect()
{
	auto *dlg = new ConfigurableSelectorDialog(win, Module::Type::AUDIO_EFFECT, session);
	dlg->run();
	if (dlg->_return.num > 0){
		auto *m = chain->addAudioEffect(dlg->_return);
		m->x = sel.dx;
		m->y = sel.dy;
	}
	delete(dlg);
}

void SignalEditor::onAddAudioJoiner()
{
	auto *m = chain->addAudioJoiner();
	m->x = sel.dx;
	m->y = sel.dy;
}

void SignalEditor::onAddAudioInputStream()
{
	auto *m = chain->addAudioInputStream();
	m->x = sel.dx;
	m->y = sel.dy;
}

void SignalEditor::onAddMidiSource()
{
	auto *dlg = new ConfigurableSelectorDialog(win, Module::Type::MIDI_SOURCE, session);
	dlg->run();
	if (dlg->_return.num > 0){
		auto *m = chain->addMidiSource(dlg->_return);
		m->x = sel.dx;
		m->y = sel.dy;
	}
	delete(dlg);
}

void SignalEditor::onAddMidiEffect()
{
	auto *dlg = new ConfigurableSelectorDialog(win, Module::Type::MIDI_EFFECT, session);
	dlg->run();
	if (dlg->_return.num > 0){
		auto *m = chain->addMidiEffect(dlg->_return);
		m->x = sel.dx;
		m->y = sel.dy;
	}
	delete(dlg);
}

void SignalEditor::onAddSynthesizer()
{
	auto *dlg = new ConfigurableSelectorDialog(win, Module::Type::SYNTHESIZER, session);
	dlg->run();
	if (dlg->_return.num > 0){
		auto *m = chain->addSynthesizer(dlg->_return);
		m->x = sel.dx;
		m->y = sel.dy;
	}
	delete(dlg);
}

void SignalEditor::onAddMidiInputStream()
{
	auto *m = chain->addMidiInputStream();
	m->x = sel.dx;
	m->y = sel.dy;
}

void SignalEditor::onAddPitchDetector()
{
	auto *m = chain->addPitchDetector();
	m->x = sel.dx;
	m->y = sel.dy;
}

void SignalEditor::onAddBeatSource()
{
	auto *dlg = new ConfigurableSelectorDialog(win, Module::Type::BEAT_SOURCE, session);
	dlg->run();
	if (dlg->_return.num > 0){
		auto *m = chain->addBeatSource(dlg->_return);
		m->x = sel.dx;
		m->y = sel.dy;
	}
	delete(dlg);
}

void SignalEditor::onAddBeatMidifier()
{
	auto *m = chain->addBeatMidifier();
	m->x = sel.dx;
	m->y = sel.dy;
}

void SignalEditor::onModuleDelete()
{
	if (sel.type == sel.TYPE_MODULE){
		chain->remove((SignalChain::_Module*)sel.module);
		hover = sel = Selection();
		apply_sel(this);
	}
}

void SignalEditor::onModuleConfigure()
{
	apply_sel(this);
	session->win->side_bar->open(SideBar::MODULE_CONSOLE);
}

void SignalEditor::onReset()
{
	chain->reset();
}

void SignalEditor::onLoad()
{
	if (hui::FileDialogOpen(win, "", "", "*", "*"))
		chain->load(hui::Filename);
}

void SignalEditor::onSave()
{
	if (hui::FileDialogSave(win, "", "", "*", "*"))
		chain->save(hui::Filename);
}

SignalEditor::Selection SignalEditor::getHover(float mx, float my)
{
	Selection s;
	s.dx = mx;
	s.dy = my;
	for (auto *m: chain->modules){
		rect r = module_rect(m);
		if (r.inside(mx, my)){
			s.type = Selection::TYPE_MODULE;
			s.module = m;
			s.dx = m->x - mx;
			s.dy = m->y - my;
			return s;
		}
		for (int i=0; i<m->port_in.num; i++){
			float y = module_port_in_y(m, i);
			float x = module_port_in_x(m);
			if (abs(x - mx) < 10 and abs(y - my) < 10){
				s.type = Selection::TYPE_PORT_IN;
				s.module = m;
				s.port = i;
				s.port_type = m->port_in[i];
				s.dx = x;
				s.dy = y;
				return s;
			}
		}
		for (int i=0; i<m->port_out.num; i++){
			float y = module_port_out_y(m, i);
			float x = module_port_out_x(m);
			if (abs(x - mx) < 10 and abs(y - my) < 10){
				s.type = Selection::TYPE_PORT_OUT;
				s.module = m;
				s.port = i;
				s.port_type = m->port_out[i];
				s.dx = x;
				s.dy = y;
				return s;
			}
		}
	}
	return s;
}

void draw_cable(Painter *p, SignalChain::Cable *c)
{
	complex p0 = complex(module_port_out_x(c->source), module_port_out_y(c->source, c->source_port));
	complex p1 = complex(module_port_in_x(c->target), module_port_in_y(c->target, c->target_port));
	p->setColor(signal_color(c->type));
	p->drawLine(p0.x, p0.y, p1.x, p1.y);
	complex m = (p0 + p1) / 2;
	complex d = (p1 - p0) / (p1 - p0).abs();
	complex e = d * c_i;
	Array<complex> pp;
	pp.add(m + d * 12);
	pp.add(m - d * 12 + e * 5);
	pp.add(m - d * 12 - e * 5);
	p->drawPolygon(pp);
	//p->dr
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
		if (hover.type == Selection::TYPE_MODULE and hover.module == m)
			p->setColor(ColorInterpolate(view->colors.background_track_selected, view->colors.hover, 0.25f));
		p->setRoundness(view->CORNER_RADIUS);
		p->drawRect(module_rect(m));
		p->setRoundness(0);
		if (sel.type == sel.TYPE_MODULE and sel.module == m){
			p->setColor(view->colors.text);
			p->setFont("", 12, true, false);
		}else{
			p->setColor(view->colors.text_soft1);
		}
		float ww = p->getStrWidth(m->type());
		p->drawStr(m->x + MODULE_WIDTH/2 - ww/2, m->y + 4, m->type());
		p->setFont("", 12, false, false);

		foreachi(int t, m->port_in, i){
			p->setColor(signal_color(t));
			float r = 4;
			if (hover.type == Selection::TYPE_PORT_IN and hover.module == m and hover.port == i)
				r = 8;
			p->drawCircle(module_port_in_x(m), module_port_in_y(m, i), r);
		}
		foreachi(int t, m->port_out, i){
			p->setColor(signal_color(t));
			float r = 4;
			if (hover.type == Selection::TYPE_PORT_OUT and hover.module == m and hover.port == i)
				r = 8;
			p->drawCircle(module_port_out_x(m), module_port_out_y(m, i), r);
		}
	}

	for (auto *c: chain->cables)
		draw_cable(p, c);

	if (sel.type == sel.TYPE_PORT_IN or sel.type == sel.TYPE_PORT_OUT){
		p->setColor(White);
		if (hover.target_module){
			p->setLineWidth(5);
			SignalChain::_Module *t = (SignalChain::_Module*)hover.target_module;
			if (hover.type == hover.TYPE_PORT_IN)
				p->drawLine(sel.dx, sel.dy, module_port_out_x(t), module_port_out_y(t, hover.target_port));
			else
				p->drawLine(sel.dx, sel.dy, module_port_in_x(t), module_port_in_y(t, hover.target_port));
			p->setLineWidth(1);
		}else
			p->drawLine(sel.dx, sel.dy, hui::GetEvent()->mx, hui::GetEvent()->my);
	}
}

void SignalEditor::onChainUpdate()
{
	redraw("area");
}
