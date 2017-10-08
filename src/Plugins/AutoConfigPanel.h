/*
 * AutoConfigPanel.h
 *
 *  Created on: 08.10.2017
 *      Author: michi
 */

#ifndef SRC_PLUGINS_AUTOCONFIGPANEL_H_
#define SRC_PLUGINS_AUTOCONFIGPANEL_H_

#include "ConfigPanel.h"

class AutoConfigData;
class PluginData;

class AutoConfigPanel : public ConfigPanel
{
public:
	Array<AutoConfigData*> aa;
	AutoConfigPanel(Array<AutoConfigData*> &_aa, Configurable *_c);
	~AutoConfigPanel();
	void onChange();
	virtual void _cdecl update();
};

Array<AutoConfigData*> get_auto_conf(PluginData *config);


#endif /* SRC_PLUGINS_AUTOCONFIGPANEL_H_ */