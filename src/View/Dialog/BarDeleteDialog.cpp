/*
 * BarDeleteDialog.cpp
 *
 *  Created on: 06.09.2018
 *      Author: michi
 */

#include "BarDeleteDialog.h"

#include "../../Data/Rhythm/Bar.h"
#include "../../Data/Song.h"
#include "../../Data/base.h"
#include "../AudioView.h"

BarDeleteDialog::BarDeleteDialog(hui::Window *root, Song *s, const Range &_bars):
	hui::Dialog("", 100, 100, root, false)
{
	fromResource("bar_delete_dialog");
	song = s;
	for (int i=_bars.start(); i<_bars.end(); i++)
		sel.add(i);

	check("shift-data", true);
	enable("replace-by-pause", false);

	event("ok", std::bind(&BarDeleteDialog::onOk, this));
	event("cancel", std::bind(&BarDeleteDialog::onClose, this));
	event("hui:close", std::bind(&BarDeleteDialog::onClose, this));
}

void BarDeleteDialog::onOk()
{
	bool move_data = isChecked("shift-data");

	song->beginActionGroup();

	foreachb(int i, sel)
		song->deleteBar(i, move_data);// ? Bar::EditMode::STRETCH : Bar::EditMode::IGNORE);

	song->endActionGroup();

	destroy();
}

void BarDeleteDialog::onClose()
{
	destroy();
}
