/*
 * ConfigurableSelectorDialog.h
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#ifndef CONFIGURABLESELECTORDIALOG_H_
#define CONFIGURABLESELECTORDIALOG_H_

#include "../../lib/hui/hui.h"

class Configurable;
class Synthesizer;

class ConfigurableSelectorDialog: public HuiDialog
{
public:
	ConfigurableSelectorDialog(HuiWindow *_parent, int type, const string &old_name = "");
	virtual ~ConfigurableSelectorDialog();

	void onListSelect();
	void onSelect();
	void onClose();
	void onCancel();
	void onOk();

	int type;
	Array<string> names;
	Set<string> ugroups;
	Array<string> groups;

	static Configurable *_return;
};

Synthesizer *ChooseSynthesizer(HuiWindow *parent, const string &old_name = "");

#endif /* CONFIGURABLESELECTORDIALOG_H_ */
