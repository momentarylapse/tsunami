/*
 * ModulePanel.h
 *
 *  Created on: Jun 21, 2019
 *      Author: michi
 */

#ifndef SRC_VIEW_MODULE_MODULEPANEL_H_
#define SRC_VIEW_MODULE_MODULEPANEL_H_

#include "../../lib/hui/Window.h"
#include "../../lib/pattern/Observable.h"

namespace tsunami {

class Module;
class ConfigPanel;
class Session;


enum class ConfigPanelMode {
	None = 0,
	Profiles = 2,
	Enable = 4,
	Delete = 8,
	Close = 16,
	Replace = 32,
	Wetness = 64,
	FixedWidth = 256,
	FixedHeight = 512,
};
inline ConfigPanelMode operator&(ConfigPanelMode a, ConfigPanelMode b) {
	return (ConfigPanelMode)( (int)a & (int)b );
}
inline ConfigPanelMode operator|(ConfigPanelMode a, ConfigPanelMode b) {
	return (ConfigPanelMode)( (int)a | (int)b );
}

class ConfigPanelSocket : public obs::Node<VirtualBase> {
public:
	ConfigPanelSocket(Module *m, ConfigPanelMode mode);
	~ConfigPanelSocket() override;

	void integrate(hui::Panel *panel);

	std::function<void(bool)> func_enable;
	std::function<void(float)> func_set_wetness;
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
	void on_wetness();
	void on_delete();
	void on_large();
	void on_external();
	void on_replace();
	void on_detune();
	void on_change();


	void set_func_enable(std::function<void(bool)> f);
	void set_func_set_wetness(std::function<void(float)> f);
	void set_func_delete(std::function<void()> f);
	void set_func_close(std::function<void()> f);
	void set_func_replace(std::function<void()> f);
	void set_func_detune(std::function<void()> f);
	void copy_into(ConfigPanelSocket *dest);
};

class ModulePanel : public obs::Node<hui::Panel> {
public:
	ModulePanel(Module *m, hui::Panel *parent, ConfigPanelMode mode);
	~ModulePanel() override;

	ConfigPanelSocket socket;
	
	void set_width(int width);

	void set_func_enable(std::function<void(bool)> f) { socket.set_func_enable(f); }
	void set_func_set_wetness(std::function<void(float)> f) { socket.set_func_set_wetness(f); }
	void set_func_delete(std::function<void()> f) { socket.set_func_delete(f); }
	void set_func_close(std::function<void()> f) { socket.set_func_close(f); }
	void set_func_replace(std::function<void()> f) { socket.set_func_replace(f); }
	void set_func_detune(std::function<void()> f) { socket.set_func_detune(f); }
};


class ModuleExternalDialog : public obs::Node<hui::Dialog> {
public:
	ModuleExternalDialog(Module *_module, hui::Window *parent, ConfigPanelMode mode);
	~ModuleExternalDialog() override;

	ConfigPanelSocket socket;
};

}

#endif /* SRC_VIEW_MODULE_MODULEPANEL_H_ */
