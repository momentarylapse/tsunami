/*
 * SignalChain.h
 *
 *  Created on: 01.04.2018
 *      Author: michi
 */

#ifndef SRC_MODULE_SIGNALCHAIN_H_
#define SRC_MODULE_SIGNALCHAIN_H_

#include "Module.h"
#include "../lib/base/optional.h"
#include <mutex>

class ConfigPanel;
class Module;
class Session;
class Path;
enum class SignalType;
enum class ModuleCategory;
class SuckerThread;

struct Cable {
	SignalType type;
	Module *source, *target;
	int source_port, target_port;
};

class SignalChain : public Module {
	friend class SuckerThread;
public:
	SignalChain(Session *session, const string &name);
	virtual ~SignalChain();

	void _cdecl __init__(Session *session, const string &name);
	void _cdecl __delete__() override;

	obs::source out_add_module{this, "add-module"};
	obs::source out_delete_module{this, "delete-module"};
	obs::source out_add_cable{this, "add-cable"};
	obs::source out_delete_cable{this, "delete-cable"};

	void unregister();

	void reset(bool hard);
	void save(const Path &filename);
	static xfer<SignalChain> load(Session *session, const Path &filename);

	string name;

	shared_array<Module> modules;
	shared<Module> _add(shared<Module> m);
	shared<Module> add(ModuleCategory type, const string &sub_type = "");
	template<class M = Module>
	shared<M> addx(ModuleCategory type, const string &sub_type = "") {
		return (M*)add(type, sub_type).get();
	}
	void delete_module(Module *m);
	int module_index(Module *m);

	Module *get_by_type(ModuleCategory type, const string &sub_type);

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

	struct ConnectionQueryResult {
		Module *m;
		int port;
	};

	base::optional<ConnectionQueryResult> find_connected(Module *m, int port, int direction) const;

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
	bool is_prepared();
	bool is_active();


	void set_buffer_size(int size);
	static const int DEFAULT_BUFFER_SIZE;
	int buffer_size;
	float no_data_wait;
	owned<SuckerThread> sucker_thread;
	int do_suck();
	void _start_sucking();
	void _stop_sucking_soft();
	void _stop_sucking_hard();

	std::mutex mutex;

	void mark_all_modules_as_system();
};

#endif /* SRC_MODULE_SIGNALCHAIN_H_ */
