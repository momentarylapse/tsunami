/*
 * ConfigPanel.cpp
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#include "ConfigPanel.h"
#include "Configurable.h"

ConfigPanel::ConfigPanel(Configurable *_c)
{
	c = _c;
}

ConfigPanel::ConfigPanel()
{
	c = NULL;
}

ConfigPanel::~ConfigPanel()
{
}


void ConfigPanel::__init__(Configurable *_c)
{
	new(this) ConfigPanel(_c);
}

void ConfigPanel::__delete__()
{
	this->ConfigPanel::~ConfigPanel();
}

void ConfigPanel::notify()
{
	c->notify();
}

