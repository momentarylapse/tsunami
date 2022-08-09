/*
 * ModulePanel.h
 *
 *  Created on: Jun 21, 2019
 *      Author: michi
 */

#ifndef SRC_VIEW_MODULE_MODULEPANEL_H_
#define SRC_VIEW_MODULE_MODULEPANEL_H_

#include "../../lib/hui/hui.h"
#include "../../stuff/Observable.h"

class Module;
class ConfigPanel;
class Session;


enum class ConfigPanelMode {
	NONE = 0,
	HEADER = 1,
	PROFILES = 2,
	ENABLE = 4,
	DELETE = 8,
	CLOSE = 16,
	REPLACE = 32,
	FIXED_WIDTH = 256,
	FIXED_HEIGHT = 512,
	DEFAULT_FIXED_WIDTH = HEADER | PROFILES | ENABLE | FIXED_WIDTH,
	DEFAULT_FIXED_HEIGHT = HEADER | PROFILES | ENABLE | FIXED_HEIGHT,
	DEFAULT_FREE = HEADER | PROFILES | ENABLE,// | REPLACE,
	CONFIG_PANEL = PROFILES
};
inline ConfigPanelMode operator&(ConfigPanelMode a, ConfigPanelMode b) {
	return (ConfigPanelMode)( (int)a & (int)b );
}
inline ConfigPanelMode operator|(ConfigPanelMode a, ConfigPanelMode b) {
	return (ConfigPanelMode)( (int)a | (int)b );
}

class ConfigPanelSocket : public VirtualBase {
public:
	ConfigPanelSocket(Module *m, ConfigPanelMode mode);
	~ConfigPanelSocket() override;

	void integrate(hui::Panel *panel);

	std::function<void(bool)> func_enable;
	std::function<void()> func_delete;
	std::function<void()> func_close;
	std::function<void()> func_replace;
	std::function<void()> func_detune;
	Session *session;
	Module *module;
	string old_param;
	shared<ConfigPanel> config_panel;
	hui::Panel *panel;
	hui::Menu *menu;
	ConfigPanelMode mode;

	void on_load();
	void on_save();
	void on_enabled();
	void on_delete();
	void on_large();
	void on_external();
	void on_replace();
	void on_detune();
	void on_change();


	void set_func_enable(std::function<void(bool)> f);
	void set_func_delete(std::function<void()> f);
	void set_func_close(std::function<void()> f);
	void set_func_replace(std::function<void()> f);
	void set_func_detune(std::function<void()> f);
	void copy_into(ConfigPanelSocket *dest);
};

class ModulePanel : public Observable<hui::Panel> {
public:
	ModulePanel(Module *m, hui::Panel *parent, ConfigPanelMode mode);
	~ModulePanel() override;

	ConfigPanelSocket socket;
	
	void set_width(int width);

	void set_func_enable(std::function<void(bool)> f) { socket.set_func_enable(f); }
	void set_func_delete(std::function<void()> f) { socket.set_func_delete(f); }
	void set_func_close(std::function<void()> f) { socket.set_func_close(f); }
	void set_func_replace(std::function<void()> f) { socket.set_func_replace(f); }
	void set_func_detune(std::function<void()> f) { socket.set_func_detune(f); }
};


class ModuleExternalDialog : public hui::Dialog {
public:
	ModuleExternalDialog(Module *_module, hui::Window *parent, ConfigPanelMode mode);
	~ModuleExternalDialog() override;

	ConfigPanelSocket socket;
};


#endif /* SRC_VIEW_MODULE_MODULEPANEL_H_ */
