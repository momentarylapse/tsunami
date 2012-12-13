/*
 * EmbeddedDialog.h
 *
 *  Created on: 13.12.2012
 *      Author: michi
 */

#ifndef EMBEDDEDDIALOG_H_
#define EMBEDDEDDIALOG_H_

#include "../../lib/hui/hui.h"

class EmbeddedDialog : public HuiEventHandler
{
public:
	EmbeddedDialog(CHuiWindow *win);
	virtual ~EmbeddedDialog();

	void Enable(const string &id, bool enabled);
	void Check(const string &id, bool checked);
	bool IsChecked(const string &id);

	void SetString(const string &id, const string &str);
	void SetFloat(const string &id, float f);
	void SetInt(const string &id, int i);
	string GetString(const string &id);
	float GetFloat(const string &id);
	int GetInt(const string &id);

	CHuiWindow *win;
};

#endif /* EMBEDDEDDIALOG_H_ */
