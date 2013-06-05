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
	EmbeddedDialog(HuiWindow *win);
	virtual ~EmbeddedDialog();

	void Enable(const string &id, bool enabled);
	void Check(const string &id, bool checked);
	bool IsChecked(const string &id);

	void Reset(const string &id);
	void SetString(const string &id, const string &str);
	void AddString(const string &id, const string &str);
	void SetFloat(const string &id, float f);
	void SetInt(const string &id, int i);
	string GetString(const string &id);
	string GetCell(const string &id, int row, int col);
	float GetFloat(const string &id);
	int GetInt(const string &id);

	HuiWindow *win;
};

#endif /* EMBEDDEDDIALOG_H_ */
