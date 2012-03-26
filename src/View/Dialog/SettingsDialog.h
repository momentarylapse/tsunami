/*
 * SettingsDialog.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SETTINGSDIALOG_H_
#define SETTINGSDIALOG_H_

#include "../../lib/hui/hui.h"

class SettingsDialog: public CHuiWindow
{
public:
	SettingsDialog(CHuiWindow *_parent, bool _allow_parent);
	virtual ~SettingsDialog();

	void LoadData();
	void ApplyData();

	void OnOk();
	void OnClose();
};

#endif /* SETTINGSDIALOG_H_ */
