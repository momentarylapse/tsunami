/*
 * RawConfigDialog.h
 *
 *  Created on: 07.07.2013
 *      Author: michi
 */

#ifndef SRC_STORAGE_DIALOG_RAWCONFIGDIALOG_H_
#define SRC_STORAGE_DIALOG_RAWCONFIGDIALOG_H_

#include "../../lib/hui/hui.h"

class StorageOperationData;

class RawConfigDialog : public hui::Window {
public:
	RawConfigDialog(StorageOperationData *od, hui::Window *parent);

	void on_close();
	void on_ok();

	StorageOperationData *od;
	bool ok;
};

#endif /* SRC_STORAGE_DIALOG_RAWCONFIGDIALOG_H_ */
