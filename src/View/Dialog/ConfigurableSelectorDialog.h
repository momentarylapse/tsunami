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
class Song;

class ConfigurableSelectorDialog: public HuiDialog
{
public:
	ConfigurableSelectorDialog(HuiWindow *_parent, int type, Song *song, const string &old_name = "");
	virtual ~ConfigurableSelectorDialog();

	void onListSelect();
	void onSelect();
	void onClose();
	void onCancel();
	void onOk();

	int type;
	Song *song;
	Array<string> names;
	Set<string> ugroups;
	Array<string> groups;

	static Configurable *_return;
};

Synthesizer *ChooseSynthesizer(HuiWindow *parent, Song *song, const string &old_name = "");

#endif /* CONFIGURABLESELECTORDIALOG_H_ */
