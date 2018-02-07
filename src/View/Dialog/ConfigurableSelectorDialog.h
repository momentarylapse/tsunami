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

class ConfigurableSelectorDialog: public hui::Window
{
public:
	ConfigurableSelectorDialog(hui::Window *_parent, int type, Song *song, const string &old_name = "");
	virtual ~ConfigurableSelectorDialog();

	void onListSelect();
	void onSelect();
	void onClose();
	void onCancel();
	void onOk();

	int type;
	Song *song;
	struct Label
	{
		string full;
		string name, group;
	};
	Array<Label> labels;
	Set<string> ugroups;

	static Label split_label(const string &s);

	Configurable *_return;
};

#endif /* CONFIGURABLESELECTORDIALOG_H_ */
