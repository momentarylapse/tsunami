/*
 * main.cpp
 *
 *  Created on: 15.09.2024
 *      Author: michi
 */

/*#include "Tsunami.h"
#include "lib/base/pointer.h"
#include "lib/hui/config.h"
#include "lib/hui/language.h"
#include "lib/os/CommandLineParser.h"
#include "lib/os/msg.h"
#include "data/base.h"
#include "data/Song.h"
#include "module/SignalChain.h"
#include "module/audio/AudioEffect.h"
#include "module/audio/PeakMeter.h"
#include "view/TsunamiWindow.h"
#include "Session.h"
#include "EditModes.h"
#include "storage/Storage.h"
#include "stuff/Log.h"
#include "stuff/Clipboard.h"
#include "stuff/PerformanceMonitor.h"
#include "stuff/BackupManager.h"
#include "stuff/SessionManager.h"
#include "stuff/ErrorHandler.h"
#include "plugins/PluginManager.h"
#include "plugins/TsunamiPlugin.h"
#include "device/DeviceManager.h"
#include "command/song/Diff.h"
#include "command/song/Show.h"
#include "test/TestRingBuffer.h"
#ifndef NDEBUG
#include "module/ModuleFactory.h"
#include "view/module/ConfigPanel.h"
#include "view/module/ConfigurationDialog.h"
#endif*/

#include "../lib/base/base.h"
#include "../lib/base/pointer.h"
#include "../lib/hui/hui.h"
#include "../lib/os/CommandLineParser.h"
#include "../Tsunami.h"
#include "TestRingBuffer.h"
#include "../module/ModuleFactory.h"
#include "../view/module/ConfigPanel.h"
#include "../view/module/ConfigurationDialog.h"
#include "../storage/Storage.h"
#include "../stuff/Log.h"
#include "../stuff/Clipboard.h"
#include "../stuff/PerformanceMonitor.h"
#include "../stuff/BackupManager.h"
#include "../stuff/SessionManager.h"
#include "../stuff/ErrorHandler.h"
#include "../Session.h"
#include "../device/DeviceManager.h"
#include "../plugins/PluginManager.h"
#include "../module/Module.h"
#include "../lib/os/msg.h"
#include "../view/TsunamiWindow.h"
#include "../data/Song.h"

namespace tsunami {



class Song;
class PluginManager;
class Slider;
class Log;
class AudioInput;
class DeviceManager;
class AudioView;
class Storage;
class Clipboard;
class TsunamiWindow;
class Session;
class SessionManager;
class BackupManager;
class PerformanceMonitor;

class Tester : public Tsunami {
public:
	hui::AppStatus on_startup_before_gui_init(const Array<string> &arg) override;

	hui::AppStatus handle_arguments(const Array<string> &arg);
};

hui::AppStatus Tester::on_startup_before_gui_init(const Array<string> &arg) {

	ErrorHandler::init();

	backup_manager = new BackupManager;
	session_manager = new SessionManager(backup_manager.get());

	perf_mon = new PerformanceMonitor;

	log = new Log;

	Session::GLOBAL = new Session(log.get(), nullptr, nullptr, session_manager.get(), perf_mon.get());

	clipboard = new Clipboard;

	device_manager = new DeviceManager(Session::GLOBAL);
	Session::GLOBAL->device_manager = device_manager.get();

	// create (link) PluginManager after all other components are ready
	plugin_manager = new PluginManager;
	Session::GLOBAL->plugin_manager = plugin_manager.get();

	plugin_manager->link_app_data();

	return handle_arguments(arg);
}

extern bool module_config_debug;

hui::AppStatus Tester::handle_arguments(const Array<string> &args) {
	string chain_file;
	string plugin_file;
	auto flags = Storage::Flags::None;

	CommandLineParser p;
	p.info("tester", "run tests");
	p.cmd("list", "", "debug: list internal unit tests", [] (const Array<string> &) {
		UnitTest::print_all_names();
	});
	p.cmd("run", "[FILTER]", "debug: run internal unit tests", [] (const Array<string> &a) {
		if (a.num > 0)
			UnitTest::run_all(a[0]);
		else
			UnitTest::run_all("*");
	});
	p.cmd("previewgui", "TYPE NAME", "debug: show the config gui of a plugin", [this] (const Array<string> &a) {
		run_after_gui_init([this, a] {
			auto session = session_manager->spawn_new_session();
			session->win->hide();
			Module *m = nullptr;
			if (a[0] == "fx") {
				m = ModuleFactory::create(session, ModuleCategory::AudioEffect, a[1]);
				configure_module(session->win.get(), m);
			} else if (a[0] == "mfx") {
				m = ModuleFactory::create(session, ModuleCategory::MidiEffect, a[1]);
				configure_module(session->win.get(), m);
			} else if (a[0] == "synth") {
				m = ModuleFactory::create(session, ModuleCategory::Synthesizer, a[1]);
				configure_module(session->win.get(), m);
			} else if (a[0] == "vis") {
				m = ModuleFactory::create(session, ModuleCategory::AudioVisualizer, a[1]);
				auto *dlg = new hui::Window("", 800, 600);
				dlg->add_grid("", 0, 0, "root");
				ConfigPanel::_hidden_parent_ = dlg;
				auto *p = m->create_panel();
				dlg->embed(p, "root", 0,0);
				hui::fly(dlg);
			} else {
				msg_error("unknown type: " + a[0] + " (try fx|mfx|synth|vis)");
			}
		});
	});
	p.cmd("listview", "", "bla", [this] (const Array<string> &a) {
		run_after_gui_init([this, a] {
			auto dlg = new hui::Window("ListView", 800, 600);
			dlg->from_source("ListView list 'a'");
			dlg->add_string("list", "0");
			dlg->add_string("list", "1");
			dlg->add_string("list", "2");
			if (a == Array<string>({"long"}))
				for (int i=3; i<1000; i++)
					dlg->add_string("list", i2s(i));
			hui::fly_and_wait(dlg);
		});
	});
	p.cmd("columnview", "", "bla", [this] (const Array<string> &) {
		run_after_gui_init([this] {
			auto dlg = new hui::Window("ColumnView", 800, 600);
			dlg->from_source("Grid ? ''\n\tListView list 'a\\b\\c' format=CmT\n\tButton button 'x'");
			dlg->add_string("list", "true\\<b>test</b>\\x");
			dlg->add_string("list", "false\\<i>more test</i>\\y");
			dlg->event_x("list", "hui:change", [dlg] {
				auto e = hui::get_event();
				msg_write(format("edit: %d %d  %s", e->row, e->column, dlg->get_cell("list", e->row, e->column)));
			});
			dlg->event("button", [dlg] {
				dlg->change_string("list", 0, "false\\bla\\z");
			});
			hui::fly_and_wait(dlg);
		});
	});
	p.cmd("treeview", "", "bla", [this] (const Array<string> &a) {
		run_after_gui_init([this, a] {
			auto dlg = new hui::Window("TreeView", 400, 600);
			dlg->from_source("TreeView list 'a'");
			dlg->add_string("list", "0");
			dlg->add_string("list", "1");
			dlg->add_string("list", "2");
			for (int i=3; i<30; i++)
				dlg->add_child_string("list", i % 3, i2s(i));
			hui::fly_and_wait(dlg);
		});
	});
	p.parse(args);


	return hui::AppStatus::AUTO;
}
}

HUI_EXECUTE(tsunami::Tester);
