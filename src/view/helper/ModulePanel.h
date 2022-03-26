/*
 * ModulePanel.h
 *
 *  Created on: Jun 21, 2019
 *      Author: michi
 */

#ifndef SRC_VIEW_HELPER_MODULEPANEL_H_
#define SRC_VIEW_HELPER_MODULEPANEL_H_

#include "../../lib/hui/hui.h"
#include "../../stuff/Observable.h"

class Module;
class ConfigPanel;
class Session;


class ModulePanel : public Observable<hui::Panel> {
public:
	
	enum Mode {
		NONE = 0,
		HEADER = 1,
		PROFILES = 2,
		ENABLE = 4,
		DELETE = 8,
		CLOSE = 16,
		FIXED_WIDTH = 256,
		FIXED_HEIGHT = 512,
		DEFAULT = HEADER | PROFILES | ENABLE | FIXED_WIDTH,
		DEFAULT_H = HEADER | PROFILES | ENABLE | FIXED_HEIGHT,
		DEFAULT_S = HEADER | PROFILES | ENABLE,
		CONFIG_PANEL = PROFILES
	};

	ModulePanel(Module *m, hui::Panel *parent, Mode mode = Mode::DEFAULT);
	~ModulePanel() override;
	void on_load();
	void on_save();
	void on_enabled();
	void on_delete();
	void on_large();
	void on_external();
	void on_replace();
	void on_detune();
	void on_change();
	
	void set_width(int width);
	
	void copy_into(ModulePanel *dest);
	
	void set_func_enable(std::function<void(bool)> f);
	void set_func_delete(std::function<void()> f);
	void set_func_close(std::function<void()> f);
	void set_func_replace(std::function<void()> f);
	void set_func_detune(std::function<void()> f);
	
	std::function<void(bool)> func_enable;
	std::function<void()> func_delete;
	std::function<void()> func_close;
	std::function<void()> func_replace;
	std::function<void()> func_detune;
	Session *session;
	Module *module;
	string old_param;
	ConfigPanel *p;
	hui::Panel *outer;
	hui::Menu *menu;
};


class ModuleExternalDialog : public hui::Dialog {
public:
	Module *module;
	ModulePanel *module_panel;
	ModuleExternalDialog(Module *_module, hui::Window *parent);
	~ModuleExternalDialog() override;
};


#endif /* SRC_VIEW_HELPER_MODULEPANEL_H_ */
