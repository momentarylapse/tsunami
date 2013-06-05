/*
 * RecordDialog.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "RecordDialog.h"

RecordDialog::RecordDialog(HuiWindow *_parent, bool _allow_parent):
	HuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls)
{
}

RecordDialog::~RecordDialog()
{
}
