/*
 * NewDialog.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef NEWDIALOG_H_
#define NEWDIALOG_H_


#include "../../Data/Song.h"
#include "../../lib/hui/hui.h"

class NewDialog: public HuiWindow
{
public:
	NewDialog(HuiWindow *_parent, Song *a);

	void loadData();
	void applyData();

	void onOk();
	void onMetronome();
	void onTypeMidi();

	Song *song;
};

#endif /* NEWDIALOG_H_ */
