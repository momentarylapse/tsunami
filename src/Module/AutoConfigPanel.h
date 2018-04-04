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

class AutoConfigPanel : public ConfigPanel
{
public:
	Array<AutoConfigData*> aa;
	AutoConfigPanel(Array<AutoConfigData*> &_aa, Module *_c);
	~AutoConfigPanel();
	void onChange();
	virtual void _cdecl update();
};

Array<AutoConfigData*> get_auto_conf(ModuleConfiguration *config);


#endif /* SRC_MODULE_AUTOCONFIGPANEL_H_ */
