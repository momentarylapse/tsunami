/*
 * ModulePanel.h
 *
 *  Created on: Jun 21, 2019
 *      Author: michi
 */

#ifndef SRC_VIEW_HELPER_MODULEPANEL_H_
#define SRC_VIEW_HELPER_MODULEPANEL_H_

#include "../../lib/hui/hui.h"

class Module;
class ConfigPanel;
class Session;


class ModulePanel : public hui::Panel {
public:
	ModulePanel(Module *_m, std::function<void(bool)> _func_enable, std::function<void()> _func_delete, std::function<void(const string&)> _func_edit);
	virtual ~ModulePanel();
	void on_load();
	void on_save();
	void on_enabled();
	void on_delete();
	void on_large();
	void on_change();
	void on_change_by_action();
	std::function<void(bool)> func_enable;
	std::function<void(const string&)> func_edit;
	std::function<void()> func_delete;
	Session *session;
	Module *module;
	string old_param;
	ConfigPanel *p;
};



#endif /* SRC_VIEW_HELPER_MODULEPANEL_H_ */
