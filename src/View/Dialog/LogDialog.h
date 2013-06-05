/*
 * LogDialog.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef LOGDIALOG_H_
#define LOGDIALOG_H_

#include "../../lib/hui/hui.h"

class LogDialog: public HuiWindow
{
public:
	LogDialog(HuiWindow *_parent, bool _allow_parent);
	virtual ~LogDialog();

	void LoadData();
	void ApplyData();

	void OnClose();
};

#endif /* LOGDIALOG_H_ */
