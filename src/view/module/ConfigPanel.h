/*
 * ConfigPanel.h
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#ifndef SRC_VIEW_MODULE_CONFIGPANEL_H_
#define SRC_VIEW_MODULE_CONFIGPANEL_H_

#include "../../lib/hui/Panel.h"
#include "../../lib/pattern/Observable.h"

namespace tsunami {

class Module;

class ConfigPanel : public obs::Node<hui::Panel> {
public:
	explicit ConfigPanel(Module *c);
	~ConfigPanel() override;
	void _cdecl __init__(Module *c);
	void _cdecl __delete__() override;

	void _cdecl changed();
	virtual void _cdecl update() {}
	virtual void _cdecl set_large(bool large) {}

	Module *c;
	bool ignore_change;

	static hui::Panel *_hidden_parent_;
	static bool _hidden_parent_check_;
};

}

#endif /* SRC_VIEW_MODULE_CONFIGPANEL_H_ */
