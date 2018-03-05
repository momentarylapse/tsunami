/*
 * NewDialog.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef NEWDIALOG_H_
#define NEWDIALOG_H_


#include "../../lib/hui/hui.h"

class Song;

class NewDialog: public hui::Window
{
public:
	NewDialog(hui::Window *_parent);

	void loadData();
	void applyData();

	void onOk();
	void onMetronome();
	void onTypeMidi();
};

#endif /* NEWDIALOG_H_ */
