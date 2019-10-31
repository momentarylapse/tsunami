/*
 * ModulePanel.h
 *
 *  Created on: Jun 21, 2019
 *      Author: michi
 */

#ifndef SRC_VIEW_HELPER_MODULEPANEL_H_
#define SRC_VIEW_HELPER_MODULEPANEL_H_

#include "../../lib/hui/hui.h"
#include "../../Stuff/Observable.h"

class Module;
class ConfigPanel;
class Session;


class ModulePanel : public Observable<hui::Panel> {
public:
	
	enum Mode {
		NONE = 0,
		HEADER = 1,
		FAVOURITES = 2,
		ENABLE = 4,
		DELETE = 8,
		CLOSE = 16,
		FIXED_WIDTH = 256,
		FIXED_HEIGHT = 512,
		DEFAULT = HEADER | FAVOURITES | ENABLE | FIXED_WIDTH,
		DEFAULT_H = HEADER | FAVOURITES | ENABLE | FIXED_HEIGHT
	};
	
	ModulePanel(Module *m, Mode mode = Mode::DEFAULT);
	virtual ~ModulePanel();
	void on_load();
	void on_save();
	void on_enabled();
	void on_delete();
	void on_large();
	void on_external();
	void on_replace();
	void on_change();
	void on_change_by_action();
	
	void set_width(int width);
	
	ModulePanel *copy();
	
	void set_func_enable(std::function<void(bool)> func_enable);
	void set_func_edit(std::function<void(const string&)> func_edit);
	void set_func_delete(std::function<void()> func_delete);
	void set_func_close(std::function<void()> func_close);
	void set_func_replace(std::function<void()> func_replace);
	
	std::function<void(bool)> func_enable;
	std::function<void(const string&)> func_edit;
	std::function<void()> func_delete;
	std::function<void()> func_close;
	std::function<void()> func_replace;
	Session *session;
	Module *module;
	string old_param;
	ConfigPanel *p;
	hui::Menu *menu;
};



#endif /* SRC_VIEW_HELPER_MODULEPANEL_H_ */
