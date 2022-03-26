/*
 * AutoConfigPanel.h
 *
 *  Created on: 08.10.2017
 *      Author: michi
 */

#ifndef SRC_MODULE_AUTOCONFIGPANEL_H_
#define SRC_MODULE_AUTOCONFIGPANEL_H_

#include "ConfigPanel.h"

class AutoConfigData;
class ModuleConfiguration;
class Session;

class AutoConfigPanel : public ConfigPanel {
public:
	Array<AutoConfigData*> aa;
	AutoConfigPanel(Array<AutoConfigData*> &_aa, Module *_c);
	~AutoConfigPanel();
	void _cdecl update() override;
};

Array<AutoConfigData*> get_auto_conf(ModuleConfiguration *config, Session *session);


#endif /* SRC_MODULE_AUTOCONFIGPANEL_H_ */
