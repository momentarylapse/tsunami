/*
 * PdfConfigDialog.h
 *
 *  Created on: 25.08.2018
 *      Author: michi
 */

#ifndef SRC_STORAGE_DIALOG_PDFCONFIGDIALOG_H_
#define SRC_STORAGE_DIALOG_PDFCONFIGDIALOG_H_

#include "../../lib/hui/hui.h"

class Song;
class StorageOperationData;

class PdfConfigDialog : public hui::Dialog {
public:
	PdfConfigDialog(StorageOperationData *od, hui::Window *parent);

	void on_close();
	void on_ok();

	StorageOperationData *od;
	Song *song;
	bool ok;
};

#endif /* SRC_STORAGE_DIALOG_PDFCONFIGDIALOG_H_ */
