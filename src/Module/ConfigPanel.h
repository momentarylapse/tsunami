/*
 * ConfigPanel.h
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#ifndef SRC_MODULE_CONFIGPANEL_H_
#define SRC_MODULE_CONFIGPANEL_H_

#include "../lib/hui/hui.h"

class Module;

class ConfigPanel : public hui::Panel {
public:
	ConfigPanel(Module *c);
	~ConfigPanel() override;
	void _cdecl __init__(Module *c);
	void _cdecl __delete__() override;

	void _cdecl changed();
	virtual void _cdecl update() {}
	virtual void _cdecl set_large(bool large) {}

	Module *c;
	bool ignore_change;
};


bool configure_module(hui::Window *win, Module *m);

#endif /* SRC_MODULE_CONFIGPANEL_H_ */
