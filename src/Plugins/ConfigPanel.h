/*
 * ConfigPanel.h
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#ifndef CONFIGPANEL_H_
#define CONFIGPANEL_H_

#include "../lib/hui/hui.h"

class Configurable;

class ConfigPanel : public HuiPanel
{
public:
	ConfigPanel();
	ConfigPanel(Configurable *c);
	virtual ~ConfigPanel();
	void _cdecl __init__(Configurable *c);
	virtual void _cdecl __delete__();

	void notify();
	virtual void update(){}

	Configurable *c;
};

#endif /* CONFIGPANEL_H_ */
