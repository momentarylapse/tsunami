/*
 * ConfigPanel.cpp
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#include "ConfigPanel.h"
#include "Module.h"

ConfigPanel::ConfigPanel(Module *_c)
{
	c = _c;
}

ConfigPanel::ConfigPanel()
{
	c = nullptr;
}

ConfigPanel::~ConfigPanel()
{
}


void ConfigPanel::__init__(Module *_c)
{
	new(this) ConfigPanel(_c);
}

void ConfigPanel::__delete__()
{
	this->ConfigPanel::~ConfigPanel();
}

void ConfigPanel::changed()
{
	c->changed();
}

