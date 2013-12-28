/*
 * SynthesizerDialog.h
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#ifndef SYNTHESIZERDIALOG_H_
#define SYNTHESIZERDIALOG_H_

#include "../../lib/hui/hui.h"

class Synthesizer;

class SynthesizerDialog: public HuiDialog
{
public:
	SynthesizerDialog(HuiWindow *_parent, const string &old_name = "");
	virtual ~SynthesizerDialog();

	void OnSelect();
	void OnClose();

	Array<string> names;
};

Synthesizer *ChooseSynthesizer(HuiWindow *parent, const string &old_name = "");

#endif /* SYNTHESIZERDIALOG_H_ */
