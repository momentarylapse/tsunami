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

struct PdfConfigData {
	float horizontal_scale;
	Array<int> track_mode;
};

class PdfConfigDialog : public hui::Dialog {
public:
	PdfConfigDialog(PdfConfigData *data, Song *song, hui::Window *parent);
	virtual ~PdfConfigDialog();

	void on_close();
	void on_ok();

	Song *song;
	PdfConfigData *data;
	bool ok;
};

#endif /* SRC_STORAGE_DIALOG_PDFCONFIGDIALOG_H_ */
