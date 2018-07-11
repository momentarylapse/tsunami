/*
 * ConfigPanel.h
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#ifndef SRC_PLUGINS_CONFIGPANEL_H_
#define SRC_PLUGINS_CONFIGPANEL_H_

#include "../lib/hui/hui.h"

class Module;

class ConfigPanel : public hui::Panel
{
public:
	ConfigPanel();
	ConfigPanel(Module *c);
	~ConfigPanel() override;
	void _cdecl __init__(Module *c);
	void _cdecl __delete__() override;

	void _cdecl changed();
	virtual void _cdecl update(){}

	Module *c;
};

#endif /* SRC_PLUGINS_CONFIGPANEL_H_ */
