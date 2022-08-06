/*
 * ConfigPanel.cpp
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#include "ConfigPanel.h"
#include "ModulePanel.h"
#include "../helper/Progress.h"
#include "../../plugins/PluginManager.h"
#include "../../Session.h"
#include "../../module/Module.h"


hui::Panel *ConfigPanel::_hidden_parent_ = nullptr;
bool ConfigPanel::_hidden_parent_check_ = true;

ConfigPanel::ConfigPanel(Module *_c) : hui::Panel() {
	static int count = 0;
	set_id(format("config-panel-%d", count ++));

	if (_hidden_parent_)
		set_parent(_hidden_parent_);
	else if (_hidden_parent_check_)
		msg_error("TODO: set _config_panel_parent_");
	_hidden_parent_ = nullptr;

	ignore_change = false;
	c = _c;
	if (c) {
		c->subscribe(this, [this]{
			if (!ignore_change) update();
		}, c->MESSAGE_CHANGE);
	}
}

ConfigPanel::~ConfigPanel() {
	if (c)
		c->unsubscribe(this);
}


void ConfigPanel::__init__(Module *_c) {
	new(this) ConfigPanel(_c);
}

void ConfigPanel::__delete__() {
	this->ConfigPanel::~ConfigPanel();
}

void ConfigPanel::changed() {
	ignore_change = true;
	c->changed();
	ignore_change = false;
}



