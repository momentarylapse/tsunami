/*
 * SynthesizerDialog.h
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#ifndef SYNTHESIZERDIALOG_H_
#define SYNTHESIZERDIALOG_H_

#include "../../lib/hui/hui.h"

class Track;

class SynthesizerDialog: public HuiDialog
{
public:
	SynthesizerDialog(HuiWindow *_parent, bool _allow_parent, Track *t);
	virtual ~SynthesizerDialog();

	void OnSelect();
	void OnClose();

	Array<string> names;
	Track *track;
};

#endif /* SYNTHESIZERDIALOG_H_ */
