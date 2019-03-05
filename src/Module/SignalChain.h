/*
 * SignalChain.h
 *
 *  Created on: 01.04.2018
 *      Author: michi
 */

#ifndef SRC_MODULE_SIGNALCHAIN_H_
#define SRC_MODULE_SIGNALCHAIN_H_

#include "Module.h"

class AudioPort;
class MidiPort;
class BeatPort;
class ConfigPanel;
class Module;
class Session;
enum class SignalType;
enum class ModuleType;

class SignalChain : public Module
{
public:
	SignalChain(Session *session, const string &name);
	virtual ~SignalChain();

	static const string MESSAGE_ADD_MODULE;
	static const string MESSAGE_DELETE_MODULE;
	static const string MESSAGE_ADD_CABLE;
	static const string MESSAGE_DELETE_CABLE;

	void reset();
	void save(const string &filename);
	static SignalChain *load(Session *session, const string &filename);

	void create_default_modules();

	string name;

	Array<Module*> modules;
	Module* _add(Module *m);
	Module* add(ModuleType type, const string &sub_type = "");
	void remove(Module *m);
	int module_index(Module *m);

	Module *get_by_type(ModuleType type, const string &sub_type);

	struct Cable
	{
		SignalType type;
		Module *source, *target;
		int source_port, target_port;
	};
	Array<Cable*> cables;
	struct PortX
	{
		Module *module;
		int port;
	};
	Array<PortX> _ports_in, _ports_out;
	void update_ports();

	void connect(Module *source, int source_port, Module *target, int target_port);
	Cable *from_source(Module *source, int port);
	Cable *to_target(Module *target, int port);
	void disconnect_source(Module *source, int port);
	void disconnect_target(Module *target, int port);
	void disconnect(Cable *c);

	void reset_state();

	int hui_runner;
	float update_dt;
	void _cdecl set_update_dt(float dt);
	void start();
	void pause(bool paused);
	void stop();

	bool playback_active;
	bool is_paused();
	bool is_playback_active();
};

#endif /* SRC_MODULE_SIGNALCHAIN_H_ */
