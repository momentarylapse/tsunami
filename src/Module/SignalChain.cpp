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
#include "../Session.h"
#include "../Plugins/PluginManager.h"
#include "../lib/file/file.h"
#include "../lib/xfile/xml.h"


const string SignalChain::MESSAGE_ADD_MODULE = "AddModule";
const string SignalChain::MESSAGE_DELETE_MODULE = "DeleteModule";
const string SignalChain::MESSAGE_ADD_CABLE = "AddCable";
const string SignalChain::MESSAGE_DELETE_CABLE = "DeleteCable";

SignalChain::SignalChain(Session *s, const string &_name) :
	Module(ModuleType::SIGNAL_CHAIN)
{
	session = s;
	name = _name;
}

SignalChain::~SignalChain()
{
	stop();
	for (Module *m: modules)
		delete m;
	for (Cable *c: cables)
		delete c;
}

SignalChain *SignalChain::create_default(Session *session)
{
	SignalChain *chain = new SignalChain(session, "playback");

	auto *mod_renderer = chain->addAudioSource("SongRenderer");
	auto *mod_peak = chain->addAudioVisualizer("PeakMeter");
	auto *mod_out = chain->addAudioOutputStream();

	chain->connect(mod_renderer, 0, mod_peak, 0);
	chain->connect(mod_peak, 0, mod_out, 0);

	return chain;
}

Module *SignalChain::add(Module *m)
{
	int i = modules.num;
	m->module_x = 50 + (i % 5) * 230;
	m->module_y = 50 + (i % 2) * 30 + 150*(i / 5);

	m->reset_state();
	modules.add(m);
	notify(MESSAGE_ADD_MODULE);
	return m;
}

int SignalChain::module_index(Module *m)
{
	foreachi(Module *mm, modules, i)
		if (mm == m)
			return i;
	return -1;
}

void SignalChain::remove(Module *m)
{
	int index = module_index(m);
	if (index < 0)
		return;
	if ((index < 3) and (this == session->signal_chain)){
		session->e(_("not allowed to delete system modules"));
		return;
	}


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
	foreachi(Cable *cc, cables, i)
		if (cc == c){
			c->target->unplug(c->target_port);

			delete(c);
			cables.erase(i);
			notify(MESSAGE_DELETE_CABLE);
			break;
		}
}

void SignalChain::disconnect_source(Module *source, int source_port)
{
	for (Cable *c: cables)
		if (c->source == source and c->source_port == source_port){
			disconnect(c);
			break;
		}
}

void SignalChain::disconnect_target(Module *target, int target_port)
{
	for (Cable *c: cables)
		if (c->target == target and c->target_port == target_port){
			disconnect(c);
			break;
		}
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

void SignalChain::load(const string& filename)
{
	for (int i=modules.num-1; i>=3; i--)
		remove(modules[i]);
	for (auto *c: cables)
		disconnect(c);



	try{

		xml::Parser p;
		p.load(filename);
		if (p.elements.num == 0)
			throw Exception("no root element");
		auto &root = p.elements[0];
		if (root.tag != "chain")
			throw Exception("root element is not called 'chain', but: " + root.tag);
		auto *head = root.find("head");
		name = head->value("name");

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
				m = add(ModuleFactory::create(session, itype, sub_type));
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
			connect(modules[s], sp, modules[t], tp);
		}

		auto *pp = root.find("ports");
		for (auto &e: pp->elements){
			PortX p;
			int m = e.value("module")._int();
			p.port = e.value("port")._int();
			p.module = modules[m];
			_ports_out.add(p);
		}
		update_ports();
	}catch(Exception &e){
		session->e(e.message());
	}
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
	return add(ModuleFactory::create(session, type, sub_type));
}

Module* SignalChain::addAudioSource(const string &name)
{
	return add(ModuleType::AUDIO_SOURCE, name);
}

Module* SignalChain::addMidiSource(const string &name)
{
	return add(ModuleType::MIDI_SOURCE, name);
}

Module* SignalChain::addAudioEffect(const string &name)
{
	return add(ModuleType::AUDIO_EFFECT, name);
}

Module* SignalChain::addAudioJoiner()
{
	return add(ModuleType::AUDIO_JOINER, "");
}

Module* SignalChain::addAudioSucker()
{
	return add(ModuleType::AUDIO_SUCKER, "");
}

Module* SignalChain::addPitchDetector()
{
	return add(ModuleType::PITCH_DETECTOR, "");
}

Module* SignalChain::addAudioVisualizer(const string &name)
{
	return add(ModuleType::AUDIO_VISUALIZER, name);
}

Module* SignalChain::addAudioInputStream()
{
	return add(ModuleType::INPUT_STREAM_AUDIO, "");
}

Module* SignalChain::addAudioOutputStream()
{
	return add(ModuleType::OUTPUT_STREAM_AUDIO, "");
}

Module* SignalChain::addMidiEffect(const string &name)
{
	return add(ModuleType::MIDI_EFFECT, name);
}

Module* SignalChain::addSynthesizer(const string &name)
{
	return add(ModuleType::SYNTHESIZER, name);
}

Module* SignalChain::addMidiInputStream()
{
	return add(ModuleType::INPUT_STREAM_MIDI, "");
}

Module* SignalChain::addBeatMidifier()
{
	return add(ModuleType::BEAT_MIDIFIER, "");
}

Module* SignalChain::addBeatSource(const string &name)
{
	return add(ModuleType::BEAT_SOURCE, name);
}

void SignalChain::reset_state()
{
	for (auto *m: modules)
		m->reset_state();
}

void SignalChain::start()
{
	reset_state();
	for (auto *m: modules)
		m->module_start();
}

void SignalChain::stop()
{
	for (auto *m: modules)
		m->module_stop();
}

void SignalChain::pause(bool paused)
{
	for (auto *m: modules)
		m->module_pause(paused);
}

