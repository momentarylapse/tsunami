/*
 * SignalChain.cpp
 *
 *  Created on: 01.04.2018
 *      Author: michi
 */

#include "SignalChain.h"
#include "Module.h"
#include "ModuleFactory.h"
#include "Port/MidiPort.h"
#include "Audio/AudioSource.h"
#include "Midi/MidiSource.h"
#include "Beats/BeatSource.h"
#include "../Session.h"
#include "../Plugins/PluginManager.h"
#include "../Device/OutputStream.h"
#include "../lib/file/file.h"
#include "../lib/xfile/xml.h"


const string SignalChain::MESSAGE_ADD_MODULE = "AddModule";
const string SignalChain::MESSAGE_DELETE_MODULE = "DeleteModule";
const string SignalChain::MESSAGE_ADD_CABLE = "AddCable";
const string SignalChain::MESSAGE_DELETE_CABLE = "DeleteCable";


const float DEFAULT_UPDATE_DT = 0.050f;
extern bool ugly_hack_slow;

SignalChain::SignalChain(Session *s, const string &_name) :
	Module(ModuleType::SIGNAL_CHAIN)
{
	session = s;
	name = _name;
	playback_active = false;
	hui_runner = -1;
	tick_dt = DEFAULT_UPDATE_DT;
	if (ugly_hack_slow)
		tick_dt *= 10;
}

SignalChain::~SignalChain()
{
	stop();
	for (Module *m: modules)
		m->unsubscribe(this);
	for (Module *m: modules)
		delete m;
	for (Cable *c: cables)
		delete c;
}
void SignalChain::set_tick_dt(float dt)
{
	tick_dt = dt;
}

void SignalChain::create_default_modules()
{
	auto *renderer = get_by_type(ModuleType::AUDIO_SOURCE, "SongRenderer");
	if (!renderer)
		renderer = add(ModuleType::AUDIO_SOURCE, "SongRenderer");
	auto *peak = get_by_type(ModuleType::AUDIO_VISUALIZER, "PeakMeter");
	if (!peak)
		peak = add(ModuleType::AUDIO_VISUALIZER, "PeakMeter");
	auto *output = get_by_type(ModuleType::OUTPUT_STREAM_AUDIO, "");
	if (!output)
		output = add(ModuleType::OUTPUT_STREAM_AUDIO);

	if (!from_source(renderer, 0) and !to_target(peak, 0))
		connect(renderer, 0, peak, 0);
	if (!from_source(peak, 0) and !to_target(output, 0))
		connect(peak, 0, output, 0);
}

Module *SignalChain::_add(Module *m)
{
	int i = modules.num;
	m->module_x = 50 + (i % 5) * 230;
	m->module_y = 50 + (i % 2) * 30 + 150*(i / 5);

	m->reset_state();
	modules.add(m);
	notify(MESSAGE_ADD_MODULE);
	m->subscribe(this, [&]{ on_module_play_end_of_stream(); }, Module::MESSAGE_PLAY_END_OF_STREAM);
	return m;
}

int SignalChain::module_index(Module *m)
{
	foreachi(Module *mm, modules, i)
		if (mm == m)
			return i;
	return -1;
}

bool is_system_module(Module *m)
{
	if (m == (Module*)m->session->output_stream)
		return true;
	if (m == (Module*)m->session->peak_meter)
		return true;
	if (m == (Module*)m->session->song_renderer)
		return true;
	return false;
}

Module *SignalChain::get_by_type(ModuleType type, const string &sub_type)
{
	for (Module *m: modules)
		if (m->module_type == type and m->module_subtype == sub_type)
			return m;
	return nullptr;
}

void SignalChain::remove(Module *m)
{
	int index = module_index(m);
	if (index < 0)
		return;
	if (is_system_module(m)){
		session->e(_("not allowed to delete system modules"));
		return;
	}

	m->unsubscribe(this);

	bool more = true;
	while (more){
		more = false;
		for (Cable *c: cables)
			if (c->source == m or c->target == m){
				disconnect(c);
				more = true;
				break;
			}
	}

	modules.erase(index);
	delete m;
	notify(MESSAGE_DELETE_MODULE);
}

void SignalChain::connect(Module *source, int source_port, Module *target, int target_port)
{
	target->plug(target_port, source, source_port);

	// TODO: check ports in use
	Cable *c = new Cable;
	c->type = source->port_out[source_port]->type;
	c->source = source;
	c->target = target;
	c->source_port = source_port;
	c->target_port = target_port;
	cables.add(c);

	notify(MESSAGE_ADD_CABLE);
}

void SignalChain::disconnect(SignalChain::Cable *c)
{
	if (!c)
		return;
	foreachi(Cable *cc, cables, i)
		if (cc == c){
			c->target->unplug(c->target_port);

			delete(c);
			cables.erase(i);
			notify(MESSAGE_DELETE_CABLE);
			break;
		}
}

SignalChain::Cable *SignalChain::from_source(Module *source, int port)
{
	for (Cable *c: cables)
		if (c->source == source and c->source_port == port)
			return c;
	return nullptr;
}

SignalChain::Cable *SignalChain::to_target(Module *target, int port)
{
	for (Cable *c: cables)
		if (c->target == target and c->target_port == port)
			return c;
	return nullptr;
}

void SignalChain::disconnect_source(Module *source, int port)
{
	disconnect(from_source(source, port));
}

void SignalChain::disconnect_target(Module *target, int port)
{
	disconnect(to_target(target, port));
}

void SignalChain::save(const string& filename)
{
	xml::Parser p;
	xml::Element root("chain");
	xml::Element hh("head");
	hh.add(xml::Element("version", "1.0"));
	hh.add(xml::Element("system", "false"));
	hh.add(xml::Element("name", name));
	root.add(hh);
	xml::Element mm("modules");
	for (Module *m: modules){
		xml::Element e("module");
		e.add(xml::Element("category", m->type_to_name(m->module_type)));
		e.add(xml::Element("class", m->module_subtype));
		e.add(xml::Element("position").with("x", f2s(m->module_x, 0)).with("y", f2s(m->module_y, 0)));
		if (m->allow_config_in_chain)
			e.add_attribute("configurable", "true");
		if (is_system_module(m))
			e.add_attribute("system", "true");
		if (m->config_to_string().num > 0)
			e.add(xml::Element("config", m->config_to_string()));
		mm.add(e);
	}
	root.add(mm);
	xml::Element cc("cables");
	for (Cable *c: cables){
		xml::Element e("cable");
		e.add(xml::Element("source").witha("id", i2s(module_index(c->source))).witha("port", i2s(c->source_port)));
		e.add(xml::Element("target").witha("id", i2s(module_index(c->target))).witha("port", i2s(c->target_port)));
		cc.add(e);
	}
	root.add(cc);
	p.elements.add(root);
	p.save(filename);
}

SignalChain *SignalChain::load(Session *session, const string& filename)
{
	auto *chain = new SignalChain(session, "new");

	try{

		xml::Parser p;
		p.load(filename);
		if (p.elements.num == 0)
			throw Exception("no root element");
		auto &root = p.elements[0];
		if (root.tag != "chain")
			throw Exception("root element is not called 'chain', but: " + root.tag);
		auto *head = root.find("head");
		chain->name = head->value("name");

		auto *mm = root.find("modules");
		for (auto &e: mm->elements){
			string type = e.value("category");
			string sub_type = e.value("class");
			string name = e.value("name");
			string sys = e.value("system");
			Module *m = nullptr;
			/*if ((i < 3) and (this == session->signal_chain)){
				m = modules[i];
			}else*/{
				auto itype = Module::type_from_name(type);
				if ((int)itype < 0)
					throw Exception("unhandled module type: " + type);
				m = chain->_add(ModuleFactory::create(session, itype, sub_type));
			}
			m->config_from_string(e.value("config"));
			m->module_x = e.find("position")->value("x")._float();
			m->module_y = e.find("position")->value("y")._float();
		}

		auto *cc = root.find("cables");
		for (auto &e: cc->elements){
			int s = e.find("source")->value("id")._int();
			int sp = e.find("source")->value("port")._int();
			int t = e.find("target")->value("id")._int();
			int tp = e.find("target")->value("port")._int();
			chain->connect(chain->modules[s], sp, chain->modules[t], tp);
		}

		auto *pp = root.find("ports");
		for (auto &e: pp->elements){
			PortX p;
			int m = e.value("module")._int();
			p.port = e.value("port")._int();
			p.module = chain->modules[m];
			chain->_ports_out.add(p);
		}
		chain->update_ports();
	}catch(Exception &e){
		session->e(e.message());
	}
	return chain;
}

void SignalChain::update_ports()
{
	port_out.clear();
	for (auto &p: _ports_out)
		port_out.add(p.module->port_out[p.port]);
}

void SignalChain::reset()
{
	for (int i=modules.num-1; i>=3; i--)
		remove(modules[i]);
	Array<Cable*> _cables = cables;
	for (auto *c: cables)
		disconnect(c);

	connect(modules[0], 0, modules[1], 0);
	connect(modules[1], 0, modules[2], 0);
}

Module* SignalChain::add(ModuleType type, const string &sub_type)
{
	return _add(ModuleFactory::create(session, type, sub_type));
}

void SignalChain::reset_state()
{
	for (auto *m: modules)
		m->reset_state();
}

void SignalChain::start()
{
	if (is_playback_active())
		return;

	reset_state();
	for (auto *m: modules)
		m->command(ModuleCommand::START);
	playback_active = true;
	notify(MESSAGE_STATE_CHANGE);
	hui_runner = hui::RunRepeated(tick_dt, [&]{ notify(MESSAGE_TICK); });
}

void SignalChain::stop()
{
	if (!is_playback_active())
		return;

	hui::CancelRunner(hui_runner);
	for (auto *m: modules)
		m->command(ModuleCommand::STOP);
	playback_active = false;
	notify(MESSAGE_STATE_CHANGE);
}

void SignalChain::pause(bool paused)
{
	if (!playback_active)
		return;
	for (auto *m: modules)
		m->command(paused ? ModuleCommand::PAUSE : ModuleCommand::UNPAUSE);
	notify(MESSAGE_STATE_CHANGE);
}


bool SignalChain::is_paused()
{
	if (playback_active)
		for (auto *m: modules)
			if (m->module_type == ModuleType::OUTPUT_STREAM_AUDIO)
				return ((OutputStream*)m)->is_paused();
	return false;
}

void SignalChain::command(ModuleCommand cmd)
{
	if (cmd == ModuleCommand::START){
		start();
	}else if (cmd == ModuleCommand::STOP){
		stop();
	}else if (cmd == ModuleCommand::PAUSE){
		pause(true);
	}else if (cmd == ModuleCommand::UNPAUSE){
		pause(false);
	}else{
		for (Module *m: modules)
			m->command(cmd);
	}
}

bool SignalChain::is_playback_active()
{
	return playback_active;
}

void SignalChain::on_module_play_end_of_stream()
{
	notify(MESSAGE_PLAY_END_OF_STREAM);
	stop();
}

int SignalChain::get_pos()
{
	int delta = 0;
	if (playback_active){
		for (auto *m: modules)
			if (m->module_type == ModuleType::OUTPUT_STREAM_AUDIO)
				delta = - ((OutputStream*)m)->get_available();
		for (auto *m: modules)
			if (m->module_type == ModuleType::AUDIO_SOURCE)
				return ((AudioSource*)m)->get_pos(delta);
	}
	return 0;
}

void SignalChain::set_pos(int pos)
{
	for (auto *m: modules){
		if (m->module_type == ModuleType::AUDIO_SOURCE)
			((AudioSource*)m)->set_pos(pos);
		else if (m->module_type == ModuleType::MIDI_SOURCE)
			((MidiSource*)m)->set_pos(pos);
		else if (m->module_type == ModuleType::BEAT_SOURCE)
			((BeatSource*)m)->set_pos(pos);
	}
	command(ModuleCommand::RESET_BUFFER);
}

