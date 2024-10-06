/*
 * PdfConfigDialog.h
 *
 *  Created on: 25.08.2018
 *      Author: michi
 */

#ifndef SRC_STORAGE_DIALOG_PDFCONFIGDIALOG_H_
#define SRC_STORAGE_DIALOG_PDFCONFIGDIALOG_H_

#include "../../lib/hui/Window.h"

namespace tsunami {

class Song;
class StorageOperationData;

class PdfConfigDialog : public hui::Dialog {
public:
	PdfConfigDialog(StorageOperationData *od, hui::Window *parent);

	void on_close();
	void on_ok();

	void on_draw(Painter *p) override;
	void _on_mouse_wheel();

	void update_params();

	StorageOperationData *od;
	Song *song;
	bool ok;
	float preview_offset_y = 0;
	float area_width = 0;
};

}

#endif /* SRC_STORAGE_DIALOG_PDFCONFIGDIALOG_H_ */
