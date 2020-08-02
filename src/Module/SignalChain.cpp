/*
 * SignalChain.cpp
 *
 *  Created on: 01.04.2018
 *      Author: michi
 */

#include "SignalChain.h"

#include "../Device/Stream/AudioOutput.h"
#include "Module.h"
#include "ModuleFactory.h"
#include "Port/Port.h"
#include "Audio/AudioSource.h"
#include "Midi/MidiSource.h"
#include "Beats/BeatSource.h"
#include "../Session.h"
#include "../Plugins/PluginManager.h"
#include "../lib/file/file.h"
#include "../lib/xfile/xml.h"
#include "../lib/threads/Thread.h"
#include "../lib/hui/hui.h"
#include "../Stuff/PerformanceMonitor.h"


const string SignalChain::MESSAGE_ADD_MODULE = "AddModule";
const string SignalChain::MESSAGE_DELETE_MODULE = "DeleteModule";
const string SignalChain::MESSAGE_ADD_CABLE = "AddCable";
const string SignalChain::MESSAGE_DELETE_CABLE = "DeleteCable";


const float DEFAULT_UPDATE_DT = 0.050f;
extern bool ugly_hack_slow;


const int SignalChain::DEFAULT_BUFFER_SIZE = 2048;


class SuckerThread : public Thread {
public:
	SignalChain *sucker;
	bool keep_running = true;

	SuckerThread(SignalChain *s) {
		sucker = s;
	}

	void on_run() override {
		while(keep_running) {
			if (sucker->sucking) {
				int r = sucker->do_suck();
				if (r == Port::END_OF_STREAM)
					break;
				if (r == Port::NOT_ENOUGH_DATA) {
					hui::Sleep(sucker->no_data_wait);
					continue;
				}
			} else {
				hui::Sleep(0.200f);
			}
			Thread::cancelation_point();
		}
	}
};


SignalChain::SignalChain(Session *s, const string &_name) :
	Module(ModuleType::SIGNAL_CHAIN, "")
{
	session = s;
	name = _name;
	state = State::UNPREPARED;
	hui_runner = -1;
	tick_dt = DEFAULT_UPDATE_DT;
	if (ugly_hack_slow)
		tick_dt *= 10;

	perf_channel_suck = PerformanceMonitor::create_channel("suck", this);

	sucking = false;
	thread = nullptr;//new AudioSuckerThread(this);
	buffer_size = hui::Config.get_int("SignalChain.BufferSize", DEFAULT_BUFFER_SIZE);
	no_data_wait = 0.005f;
}

SignalChain::~SignalChain() {
	if (thread) {
		thread->keep_running = false;
		thread->join();
		//thread->kill();
		delete(thread);
		thread = nullptr;
	}
	stop();
	for (Module *m: modules)
		m->unsubscribe(this);
	for (Module *m: modules)
		delete m;
	PerformanceMonitor::delete_channel(perf_channel_suck);
}

void SignalChain::__init__(Session *s, const string &_name) {
	new(this) SignalChain(s, _name);
}

void SignalChain::__delete__() {
	this->SignalChain::~SignalChain();
}

void SignalChain::set_tick_dt(float dt) {
	tick_dt = dt;
}

Module *SignalChain::_add(Module *m) {
	int i = modules.num;
	m->module_x = 50 + (i % 5) * 230;
	m->module_y = 50 + (i % 2) * 30 + 150*(i / 5);

	m->reset_state();
	modules.add(m);
	notify(MESSAGE_ADD_MODULE);
	m->subscribe(this, [=]{ on_module_play_end_of_stream(); }, Module::MESSAGE_PLAY_END_OF_STREAM);
	return m;
}

int SignalChain::module_index(Module *m) {
	foreachi(Module *mm, modules, i)
		if (mm == m)
			return i;
	return -1;
}

bool is_system_module(Module *m) {
	return m->belongs_to_system;
}

Module *SignalChain::get_by_type(ModuleType type, const string &sub_type) {
	for (Module *m: modules)
		if (m->module_type == type and m->module_subtype == sub_type)
			return m;
	return nullptr;
}

void SignalChain::delete_module(Module *m) {
	int index = module_index(m);
	if (index < 0)
		return;
	if (is_system_module(m)) {
		session->e(_("not allowed to delete system modules"));
		return;
	}

	m->unsubscribe(this);

	for (int i=0; i<m->port_in.num; i++)
		disconnect_in(m, i);
	for (int i=0; i<m->port_out.num; i++)
		disconnect_out(m, i);

	modules.erase(index);
	delete m;
	notify(MESSAGE_DELETE_MODULE);
}

Array<SignalChain::Cable> SignalChain::cables() {
	Array<Cable> cables;
	for (Module *target: modules) {
		foreachi (auto tp, target->port_in, tpi) {
			if (*tp.port) {
				for (Module *source: modules) {
					foreachi (auto sp, source->port_out, spi) {
						if (*tp.port == sp) {
							Cable c;
							c.target = target;
							c.target_port = tpi;
							c.source = source;
							c.source_port = spi;
							c.type = tp.type;
							cables.add(c);
						}
					}
				}
			}
		}
	}
	return cables;
}


void SignalChain::connect(Module *source, int source_port, Module *target, int target_port) {
	if (source_port < 0 or source_port >= source->port_out.num)
		throw Exception("connect: invalid source port");
	if (target_port < 0 or target_port >= target->port_in.num)
		throw Exception("connect: invalid source port");
	//msg_write("connect " + i2s(source->port_out[source_port].type) + " -> " + i2s(target->port_in[target_port].type));

	disconnect_out(source, source_port);
	disconnect_in(target, target_port);

	target->_plug_in(target_port, source, source_port);
	notify(MESSAGE_ADD_CABLE);
}

void SignalChain::disconnect(Module *source, int source_port, Module *target, int target_port) {
	disconnect_in(target, target_port);
}

void SignalChain::disconnect_out(Module *source, int source_port) {
	Port *sp = source->port_out[source_port];

	for (Module *target: modules)
		for (auto &p: target->port_in) {
			if ((*p.port) == sp) {
				*p.port = nullptr;
				notify(MESSAGE_DELETE_CABLE);
			}
		}
}

void SignalChain::disconnect_in(Module *target, int target_port) {
	auto tp = target->port_in[target_port];
	*(tp.port) = nullptr;
	notify(MESSAGE_DELETE_CABLE);
}

void SignalChain::save(const Path& filename) {
	xml::Parser p;
	xml::Element root("chain");
	xml::Element hh("head");
	hh.add(xml::Element("version", "1.0"));
	hh.add(xml::Element("system", "false"));
	hh.add(xml::Element("name", name));
	root.add(hh);
	xml::Element mm("modules");
	for (Module *m: modules) {
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
	for (Cable &c: cables()) {
		xml::Element e("cable");
		e.add(xml::Element("source").witha("id", i2s(module_index(c.source))).witha("port", i2s(c.source_port)));
		e.add(xml::Element("target").witha("id", i2s(module_index(c.target))).witha("port", i2s(c.target_port)));
		cc.add(e);
	}
	root.add(cc);
	p.elements.add(root);
	p.save(filename);
}

SignalChain *SignalChain::load(Session *session, const Path &filename) {
	auto *chain = new SignalChain(session, "new");

	try {

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
		for (auto &e: mm->elements) {
			string type = e.value("category");
			string sub_type = e.value("class");
			string name = e.value("name");
			string sys = e.value("system");
			Module *m = nullptr;
			/*if ((i < 3) and (this == session->signal_chain)) {
				m = modules[i];
			} else*/ {
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
		for (auto &e: cc->elements) {
			int s = e.find("source")->value("id")._int();
			int sp = e.find("source")->value("port")._int();
			int t = e.find("target")->value("id")._int();
			int tp = e.find("target")->value("port")._int();
			chain->connect(chain->modules[s], sp, chain->modules[t], tp);
		}

		auto *pp = root.find("ports");
		for (auto &e: pp->elements) {
			PortX p;
			int m = e.value("module")._int();
			p.port = e.value("port")._int();
			p.module = chain->modules[m];
			chain->_ports_out.add(p);
		}
		chain->update_ports();
	} catch(Exception &e) {
		session->e(e.message());
	}
	return chain;
}

void SignalChain::update_ports() {
	port_out.clear();
	for (auto &p: _ports_out)
		port_out.add(p.module->port_out[p.port]);
}

void SignalChain::reset() {
	msg_write("aaaargh  reset");
	for (int i=modules.num-1; i>=3; i--)
		delete_module(modules[i]);

	connect(modules[0], 0, modules[1], 0);
	connect(modules[1], 0, modules[2], 0);
}

Module* SignalChain::add(ModuleType type, const string &sub_type) {
	return _add(ModuleFactory::create(session, type, sub_type));
}

void SignalChain::reset_state() {
	session->debug("chain", "reset");

	std::lock_guard<std::mutex> lock(mutex);
	for (auto *m: modules)
		m->reset_state();
}

void SignalChain::prepare_start() {
	if (state != State::UNPREPARED)
		return;
	session->debug("chain", "prepare");

	{
		std::lock_guard<std::mutex> lock(mutex);
		for (auto *m: modules)
			m->command(ModuleCommand::PREPARE_START, 0);
	}
	if (!sucking)
		_start_sucking();
	state = State::PAUSED;
}

void SignalChain::start() {
	if (state == State::ACTIVE)
		return;
	session->debug("chain", "start");

	if (state == State::UNPREPARED)
		prepare_start();

	{
		std::lock_guard<std::mutex> lock(mutex);
		for (auto *m: modules)
			m->command(ModuleCommand::START, 0);
	}

	state = State::ACTIVE;
	notify(MESSAGE_STATE_CHANGE);
	hui_runner = hui::RunRepeated(tick_dt, [=]{ notify(MESSAGE_TICK); });
}

void SignalChain::stop() {
	if (state != State::ACTIVE)
		return;
	session->debug("chain", "stop");

	hui::CancelRunner(hui_runner);

	{
		std::lock_guard<std::mutex> lock(mutex);
		for (auto *m: modules)
			m->command(ModuleCommand::STOP, 0);
	}
	state = State::PAUSED;
	notify(MESSAGE_STATE_CHANGE);
}

void SignalChain::stop_hard() {
	session->debug("chain", "stop HARD");
	stop();
	_stop_sucking();
	reset_state();
	state = State::UNPREPARED;
	notify(MESSAGE_STATE_CHANGE);
}


bool SignalChain::is_paused() {
	return state == State::PAUSED;
}

int SignalChain::command(ModuleCommand cmd, int param) {
	if (cmd == ModuleCommand::START) {
		start();
		return 0;
	} else if (cmd == ModuleCommand::STOP) {
		stop();
		return 0;
	} else if (cmd == ModuleCommand::PREPARE_START) {
		prepare_start();
		return 0;
	} else {
		int r = COMMAND_NOT_HANDLED;
		for (Module *m: modules)
			r = max(r, m->command(cmd, param));
		return r;
	}
	return Module::COMMAND_NOT_HANDLED;
}

bool SignalChain::is_playback_active() {
	return (state == State::ACTIVE) or (state == State::PAUSED);
}

// running in gui thread!
void SignalChain::on_module_play_end_of_stream() {
	notify(MESSAGE_PLAY_END_OF_STREAM);
	//session->debug("auto stop");
	stop_hard();
}

void SignalChain::set_buffer_size(int size) {
	buffer_size = size;
}



void SignalChain::_start_sucking() {
	if (sucking)
		return;
	session->debug("chain", "start suck");
	thread = new SuckerThread(this);
	thread->run();
	sucking = true;
}

void SignalChain::_stop_sucking() {
	if (thread) {
		session->debug("chain", "stop suck");
		thread->keep_running = false;
		thread->join();
		delete thread;
		thread = nullptr;
	}
	sucking = false;
}

int SignalChain::do_suck() {
	std::lock_guard<std::mutex> lock(mutex);
	PerformanceMonitor::start_busy(perf_channel_suck);
	int s = Port::END_OF_STREAM;
	for (auto *m: modules) {
		int r = m->command(ModuleCommand::SUCK, buffer_size);
		s = max(s, r);
	}
	PerformanceMonitor::end_busy(perf_channel_suck);
	return s;
}

void SignalChain::mark_all_modules_as_system() {
	for (auto *m: modules)
		m->belongs_to_system = true;
}
