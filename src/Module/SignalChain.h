/*
 * SignalChain.h
 *
 *  Created on: 01.04.2018
 *      Author: michi
 */

#ifndef SRC_MODULE_SIGNALCHAIN_H_
#define SRC_MODULE_SIGNALCHAIN_H_

#include "Module.h"
#include <mutex>

class ConfigPanel;
class Module;
class Session;
enum class SignalType;
enum class ModuleType;
class SuckerThread;

class SignalChain : public Module {
	friend class SuckerThread;
public:
	SignalChain(Session *session, const string &name);
	virtual ~SignalChain();

	void _cdecl __init__(Session *session, const string &name);
	void _cdecl __delete__();

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
	void delete_module(Module *m);
	int module_index(Module *m);

	Module *get_by_type(ModuleType type, const string &sub_type);

	struct Cable {
		SignalType type;
		Module *source, *target;
		int source_port, target_port;
	};
	Array<Cable> cables();
	struct PortX {
		Module *module;
		int port;
	};
	Array<PortX> _ports_in, _ports_out;
	void update_ports();

	void connect(Module *source, int source_port, Module *target, int target_port);
	void disconnect(Module *source, int source_port, Module *target, int target_port);

	void disconnect_out(Module *source, int port);
	void disconnect_in(Module *target, int port);

	void on_module_play_end_of_stream();

	void reset_state() override;

	int hui_runner;
	float tick_dt;
	void _cdecl set_tick_dt(float dt);
	int command(ModuleCommand cmd, int param) override;
	void prepare_start();
	void start();
	void stop();
	void stop_hard();

	enum class State {
		UNPREPARED,
		ACTIVE,
		PAUSED
	};
	State state;
	bool is_paused();
	bool is_playback_active();

	int get_pos();
	void set_pos(int pos);


	void set_buffer_size(int size);
	static const int DEFAULT_BUFFER_SIZE;
	bool sucking;
	int buffer_size;
	float no_data_wait;
	SuckerThread *thread;
	int do_suck();
	void _start_sucking();
	void _stop_sucking();

	std::mutex mutex;
	int perf_channel_suck;

	void mark_all_modules_as_system();
};

#endif /* SRC_MODULE_SIGNALCHAIN_H_ */
