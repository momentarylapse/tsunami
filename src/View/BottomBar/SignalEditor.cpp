/*
 * SignalEditor.cpp
 *
 *  Created on: 30.03.2018
 *      Author: michi
 */

#include "SignalEditor.h"

#include "../../Module/Port/MidiPort.h"
#include "../AudioView.h"
#include "../../Session.h"
#include "../../Plugins/PluginManager.h"
#include "../../Module/Module.h"
#include "../../lib/math/complex.h"
#include "../../Module/SignalChain.h"
#include "../../TsunamiWindow.h"
#include "../SideBar/SideBar.h"
#include "../SideBar/ModuleConsole.h"
#include "../../lib/math/interpolation.h"
//template class Interpolator<complex>;



const float MODULE_WIDTH = 160;
const float MODULE_HEIGHT = 25;

static rect module_rect(Module *m)
{
	return rect(m->module_x, m->module_x + MODULE_WIDTH, m->module_y, m->module_y + MODULE_HEIGHT);
}

static float module_port_in_x(Module *m)
{
	return m->module_x - 5;
}

static float module_port_in_y(Module *m, int index)
{
	return m->module_y + MODULE_HEIGHT/2 + (index - (float)(m->port_in.num-1)/2)*20;
}

static float module_port_out_x(Module *m)
{
	return m->module_x + MODULE_WIDTH + 5;
}

static float module_port_out_y(Module *m, int index)
{
	return m->module_y + MODULE_HEIGHT/2 + (index - (float)(m->port_out.num-1)/2)*20;
}




class SignalEditorTab : public hui::Panel
{
public:
	SignalEditorTab(SignalEditor *ed, SignalChain *_chain)
	{
		addGrid("", 0, 0, "grid");
		setTarget("grid");
		addDrawingArea("!expandx,expandy,grabfocus", 0, 0, "area");

		eventXP("area", "hui:draw", std::bind(&SignalEditorTab::onDraw, this, std::placeholders::_1));
		eventX("area", "hui:mouse-move", std::bind(&SignalEditorTab::onMouseMove, this));
		eventX("area", "hui:left-button-down", std::bind(&SignalEditorTab::onLeftButtonDown, this));
		eventX("area", "hui:left-button-up", std::bind(&SignalEditorTab::onLeftButtonUp, this));
		eventX("area", "hui:right-button-down", std::bind(&SignalEditorTab::onRightButtonDown, this));
		eventX("area", "hui:key-down", std::bind(&SignalEditorTab::onKeyDown, this));


		event("signal_chain_add_audio_source", std::bind(&SignalEditorTab::onAddAudioSource, this));
		event("signal_chain_add_audio_effect", std::bind(&SignalEditorTab::onAddAudioEffect, this));
		event("signal_chain_add_audio_input", std::bind(&SignalEditorTab::onAddAudioInputStream, this));
		event("signal_chain_add_audio_joiner", std::bind(&SignalEditorTab::onAddAudioJoiner, this));
		event("signal_chain_add_audio_sucker", std::bind(&SignalEditorTab::onAddAudioSucker, this));
		event("signal_chain_add_audio_visualizer", std::bind(&SignalEditorTab::onAddAudioVisualizer, this));
		event("signal_chain_add_midi_source", std::bind(&SignalEditorTab::onAddMidiSource, this));
		event("signal_chain_add_midi_effect", std::bind(&SignalEditorTab::onAddMidiEffect, this));
		event("signal_chain_add_midi_input", std::bind(&SignalEditorTab::onAddMidiInputStream, this));
		event("signal_chain_add_synthesizer", std::bind(&SignalEditorTab::onAddSynthesizer, this));
		event("signal_chain_add_pitch_detector", std::bind(&SignalEditorTab::onAddPitchDetector, this));
		event("signal_chain_add_beat_source", std::bind(&SignalEditorTab::onAddBeatSource, this));
		event("signal_chain_add_beat_midifier", std::bind(&SignalEditorTab::onAddBeatMidifier, this));
		event("signal_chain_reset", std::bind(&SignalEditorTab::onReset, this));
		event("signal_chain_activate", std::bind(&SignalEditorTab::onActivate, this));
		event("signal_chain_delete", std::bind(&SignalEditorTab::onDelete, this));
		event("signal_chain_save", std::bind(&SignalEditorTab::onSave, this));
		event("signal_module_delete", std::bind(&SignalEditorTab::onModuleDelete, this));
		event("signal_module_configure", std::bind(&SignalEditorTab::onModuleConfigure, this));

		event("signal_chain_new", std::bind(&SignalEditor::onNew, ed));
		event("signal_chain_load", std::bind(&SignalEditor::onLoad, ed));

		editor = ed;
		view = ed->view;
		session = ed->session;

		chain = _chain;
		chain->subscribe(this, std::bind(&SignalEditorTab::onChainUpdate, this));
	}
	virtual ~SignalEditorTab()
	{
		chain->unsubscribe(this);
	}

	SignalEditor *editor;
	Session *session;
	AudioView *view;
	SignalChain *chain;

	struct Selection
	{
		Selection()
		{
			type = -1;
			module = NULL;
			port = port_type = -1;
			target_module = NULL;
			target_port = -1;
			dx = dy = 0;
		}
		int type;
		Module *module;
		int port, port_type;
		float dx, dy;
		enum{
			TYPE_MODULE,
			TYPE_PORT_IN,
			TYPE_PORT_OUT,
		};
		Module *target_module;
		int target_port;
	};
	//Selection getHover(float mx, float my);
	Selection hover, sel;


	Selection getHover(float mx, float my)
	{
		Selection s;
		s.dx = mx;
		s.dy = my;
		for (auto *m: chain->modules){
			rect r = module_rect(m);
			if (r.inside(mx, my)){
				s.type = Selection::TYPE_MODULE;
				s.module = m;
				s.dx = m->module_x - mx;
				s.dy = m->module_y - my;
				return s;
			}
			for (int i=0; i<m->port_in.num; i++){
				float y = module_port_in_y(m, i);
				float x = module_port_in_x(m);
				if (abs(x - mx) < 10 and abs(y - my) < 10){
					s.type = Selection::TYPE_PORT_IN;
					s.module = m;
					s.port = i;
					s.port_type = m->port_in[i].type;
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
					s.port_type = m->port_out[i].type;
					s.dx = x;
					s.dy = y;
					return s;
				}
			}
		}
		return s;
	}


	color signal_color(int type, bool hover = false)
	{
		if (type == Module::SignalType::AUDIO)
			return hover ? view->colors.red_hover : view->colors.red;
		if (type == Module::SignalType::MIDI)
			return hover ? view->colors.green_hover : view->colors.green;
		if (type == Module::SignalType::BEATS)
			return hover ? view->colors.blue_hover : view->colors.blue;
		return hover ? view->colors.white_hover : view->colors.white;
	}

	void draw_cable(Painter *p, SignalChain::Cable *c)
	{
		complex p0 = complex(module_port_out_x(c->source), module_port_out_y(c->source, c->source_port));
		complex p1 = complex(module_port_in_x(c->target), module_port_in_y(c->target, c->target_port));

		float length = (p1 - p0).abs();
		Interpolator<complex> inter(Interpolator<complex>::TYPE_CUBIC_SPLINE);
		inter.add2(p0, complex(length,0));
		inter.add2(p1, complex(length,0));

		p->setColor(signal_color(c->type));

		complex qq;
		for (float t=0; t<1.0f; t+=0.025f){
			complex q = inter.get(t);
			if (t > 0)
				p->drawLine(qq.x, qq.y, q.x, q.y);
			qq = q;
		}
		complex m = inter.get(0.5f);
		complex d = inter.getTang(0.5);
		d /= d.abs();
		complex e = d * c_i;
		Array<complex> pp;
		float arrow_length = min(length / 7, 18.0f);
		pp.add(m + d * arrow_length);
		pp.add(m - d * arrow_length + e * arrow_length / 2);
		pp.add(m - d * arrow_length - e * arrow_length / 2);
		p->drawPolygon(pp);
		//p->dr
	}

	void draw_module(Painter *p, Module *m)
	{
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
		string type = m->module_subtype;
		if (type == "")
			type = m->type_to_name(m->module_type);
		float ww = p->getStrWidth(type);
		p->drawStr(m->module_x + MODULE_WIDTH/2 - ww/2, m->module_y + 4, type);
		p->setFont("", 12, false, false);
	}

	void draw_ports(Painter *p, Module *m)
	{
		foreachi(auto &pd, m->port_in, i){
			bool hovering = (hover.type == Selection::TYPE_PORT_IN and hover.module == m and hover.port == i);
			p->setColor(signal_color(pd.type, hovering));
			float r = hovering ? 6 : 4;
			p->drawCircle(module_port_in_x(m), module_port_in_y(m, i), r);
		}
		foreachi(auto &pd, m->port_out, i){
			bool hovering = (hover.type == Selection::TYPE_PORT_OUT and hover.module == m and hover.port == i);
			p->setColor(signal_color(pd.type, hovering));
			float r = hovering ? 6 : 4;
			p->drawCircle(module_port_out_x(m), module_port_out_y(m, i), r);
		}
	}

	void onDraw(Painter* p)
	{
		int w = p->width;
		int h = p->height;
		p->setColor(view->colors.background);
		p->drawRect(0, 0, w, h);
		p->setFontSize(12);

		for (auto *m: chain->modules)
			draw_module(p, m);

		for (auto *c: chain->cables)
			draw_cable(p, c);

		for (auto *m: chain->modules)
			draw_ports(p, m);

		if (sel.type == sel.TYPE_PORT_IN or sel.type == sel.TYPE_PORT_OUT){
			p->setColor(White);
			if (hover.target_module){
				p->setLineWidth(5);
				Module *t = hover.target_module;
				if (hover.type == hover.TYPE_PORT_IN)
					p->drawLine(sel.dx, sel.dy, module_port_out_x(t), module_port_out_y(t, hover.target_port));
				else
					p->drawLine(sel.dx, sel.dy, module_port_in_x(t), module_port_in_y(t, hover.target_port));
				p->setLineWidth(1);
			}else
				p->drawLine(sel.dx, sel.dy, hui::GetEvent()->mx, hui::GetEvent()->my);
		}
	}

	void onChainUpdate()
	{
		redraw("area");
	}


	void onLeftButtonDown()
	{
		hover = getHover(hui::GetEvent()->mx, hui::GetEvent()->my);
		sel = hover;
		apply_sel();
		if (sel.type == sel.TYPE_PORT_IN){
			chain->disconnect_target(sel.module, sel.port);
		}else if (sel.type == sel.TYPE_PORT_OUT){
			chain->disconnect_source(sel.module, sel.port);
		}
		redraw("area");
	}

	void onLeftButtonUp()
	{
		if (sel.type == sel.TYPE_PORT_IN or sel.type == sel.TYPE_PORT_OUT){
			if (hover.target_module){
				if (sel.type == sel.TYPE_PORT_IN){
					chain->disconnect_source(hover.target_module, hover.target_port);
					chain->connect(hover.target_module, hover.target_port, sel.module, sel.port);
				}else if (sel.type == sel.TYPE_PORT_OUT){
					chain->disconnect_target(hover.target_module, hover.target_port);
					chain->connect(sel.module, sel.port, hover.target_module, hover.target_port);
				}
			}
			sel = Selection();
			apply_sel();
		}
		redraw("area");
	}

	void onMouseMove()
	{
		float mx = hui::GetEvent()->mx;
		float my = hui::GetEvent()->my;
		if (hui::GetEvent()->lbut){
			if (sel.type == sel.TYPE_MODULE){
				auto *m = sel.module;
				m->module_x = mx + sel.dx;
				m->module_y = my + sel.dy;
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

	void onRightButtonDown()
	{
		int mx = hui::GetEvent()->mx;
		int my = hui::GetEvent()->my;
		hover = getHover(mx, my);
		sel = hover;
		apply_sel();

		if (hover.type == hover.TYPE_MODULE){
			editor->menu_module->openPopup(this, mx, my);
		}else if (hover.type < 0){
			editor->menu_chain->openPopup(this, mx, my);
		}
	}

	void onKeyDown()
	{
		int key = hui::GetEvent()->key_code;
		if (key == hui::KEY_DELETE){
			if (sel.type == sel.TYPE_MODULE){
				chain->remove(sel.module);
				hover = sel = Selection();
			}
		}
	}

	void apply_sel()
	{
		session->win->side_bar->module_console->setModule(sel.module);

	}

	void onActivate()
	{
	}

	void onDelete()
	{
		hui::RunLater(0.001f, [&](){ editor->deleteChain(chain); });
	}



	void onAddAudioSource()
	{
		string name = session->plugin_manager->ChooseModule(win, session, Module::Type::AUDIO_SOURCE);
		if (name.num > 0){
			auto *m = chain->addAudioSource(name);
			m->module_x = sel.dx;
			m->module_y = sel.dy;
		}
	}

	void onAddAudioEffect()
	{
		string name = session->plugin_manager->ChooseModule(win, session, Module::Type::AUDIO_EFFECT);
		if (name.num > 0){
			auto *m = chain->addAudioEffect(name);
			m->module_x = sel.dx;
			m->module_y = sel.dy;
		}
	}

	void onAddAudioVisualizer()
	{
		string name = session->plugin_manager->ChooseModule(win, session, Module::Type::AUDIO_VISUALIZER);
		if (name.num > 0){
			auto *m = chain->addAudioVisualizer(name);
			m->module_x = sel.dx;
			m->module_y = sel.dy;
		}
	}

	void onAddAudioJoiner()
	{
		auto *m = chain->addAudioJoiner();
		m->module_x = sel.dx;
		m->module_y = sel.dy;
	}

	void onAddAudioSucker()
	{
		auto *m = chain->addAudioSucker();
		m->module_x = sel.dx;
		m->module_y = sel.dy;
	}

	void onAddAudioInputStream()
	{
		auto *m = chain->addAudioInputStream();
		m->module_x = sel.dx;
		m->module_y = sel.dy;
	}

	void onAddMidiSource()
	{
		string name = session->plugin_manager->ChooseModule(win, session, Module::Type::MIDI_SOURCE);
		if (name.num > 0){
			auto *m = chain->addMidiSource(name);
			m->module_x = sel.dx;
			m->module_y = sel.dy;
		}
	}

	void onAddMidiEffect()
	{
		string name = session->plugin_manager->ChooseModule(win, session, Module::Type::MIDI_EFFECT);
		if (name.num > 0){
			auto *m = chain->addMidiEffect(name);
			m->module_x = sel.dx;
			m->module_y = sel.dy;
		}
	}

	void onAddSynthesizer()
	{
		string name = session->plugin_manager->ChooseModule(win, session, Module::Type::SYNTHESIZER);
		if (name.num > 0){
			auto *m = chain->addSynthesizer(name);
			m->module_x = sel.dx;
			m->module_y = sel.dy;
		}
	}

	void onAddMidiInputStream()
	{
		auto *m = chain->addMidiInputStream();
		m->module_x = sel.dx;
		m->module_y = sel.dy;
	}

	void onAddPitchDetector()
	{
		auto *m = chain->addPitchDetector();
		m->module_x = sel.dx;
		m->module_y = sel.dy;
	}

	void onAddBeatSource()
	{
		string name = session->plugin_manager->ChooseModule(win, session, Module::Type::BEAT_SOURCE);
		if (name.num > 0){
			auto *m = chain->addBeatSource(name);
			m->module_x = sel.dx;
			m->module_y = sel.dy;
		}
	}

	void onAddBeatMidifier()
	{
		auto *m = chain->addBeatMidifier();
		m->module_x = sel.dx;
		m->module_y = sel.dy;
	}

	void onReset()
	{
		chain->reset();
	}

	/*void onLoad()
	{
		if (hui::FileDialogOpen(win, "", "", "*.chain", "*.chain"))
			chain->load(hui::Filename);
	}*/

	void onSave()
	{
		if (hui::FileDialogSave(win, "", "", "*.chain", "*.chain"))
			chain->save(hui::Filename);
	}



	void onModuleDelete()
	{
		if (sel.type == sel.TYPE_MODULE){
			chain->remove(sel.module);
			hover = sel = Selection();
			apply_sel();
		}
	}

	void onModuleConfigure()
	{
		apply_sel();
		session->win->side_bar->open(SideBar::MODULE_CONSOLE);
	}

};


SignalEditor::SignalEditor(Session *session) :
	BottomBar::Console(_("Signal Chain"), session)
{
	addTabControl("!left\\aaa", 0, 0, "selector");

	menu_chain = hui::CreateResourceMenu("popup_signal_chain_menu");
	menu_module = hui::CreateResourceMenu("popup_signal_module_menu");

	addChain(session->signal_chain);
}

SignalEditor::~SignalEditor()
{
	delete menu_chain;
	delete menu_module;
}

void SignalEditor::addChain(SignalChain *c)
{
	auto *tab = new SignalEditorTab(this, c);
	int index = tabs.num;
	string grid_id = "grid-" + i2s(index);
	if (index > 0)
		addString("selector", c->name);
	else
		changeString("selector", index, c->name);
	setTarget("selector");
	addGrid("", index, 0, grid_id);
	embed(tab, grid_id, 0, 0);
	tabs.add(tab);

	setInt("selector", index);
}

void SignalEditor::onNew()
{
	addChain(new SignalChain(session, "new"));
}

void SignalEditor::onLoad()
{
	if (hui::FileDialogOpen(win, "", "", "*.chain", "*.chain")){
		auto *c = new SignalChain(session, "new");
		c->load(hui::Filename);
		addChain(c);
	}
}

void SignalEditor::deleteChain(SignalChain *c)
{
	foreachi(auto *t, tabs, i)
		if (t->chain == c){
			if (i == 0){
				session->e(_("not allowed to delete the main signal chain"));
			}else{
				delete t;
				delete c;
				tabs.erase(i);
				removeString("selector", i);
				setInt("selector", 0);
			}
		}
}
