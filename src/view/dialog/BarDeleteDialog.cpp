/*
 * BarDeleteDialog.cpp
 *
 *  Created on: 06.09.2018
 *      Author: michi
 */

#include "BarDeleteDialog.h"
#include "../audioview/AudioView.h"
#include "../../data/base.h"
#include "../../data/rhythm/Bar.h"
#include "../../data/Song.h"

extern bool bar_dialog_move_data;
static bool bar_dialog_replace_by_pause = false;

BarDeleteDialog::BarDeleteDialog(hui::Window *parent, Song *s, const Array<int> &_bars):
	hui::Dialog("bar_delete_dialog", parent)
{
	song = s;
	sel = _bars;

	check("shift-data", bar_dialog_move_data);
	check("replace-by-pause", bar_dialog_replace_by_pause);
	enable("shift-data", !bar_dialog_replace_by_pause);
	//enable("replace-by-pause", !bar_dialog_move_data);

	event("ok", [this] { on_ok(); });
	event("cancel", [this] { request_destroy(); });
	event("hui:close", [this] { request_destroy(); });
	event("replace-by-pause", [this] { on_replace_by_pause(); });
}

void BarDeleteDialog::on_replace_by_pause() {
	enable("shift-data", !is_checked(""));
}

void BarDeleteDialog::on_ok() {
	bar_dialog_move_data = is_checked("shift-data");
	bar_dialog_replace_by_pause = is_checked("replace-by-pause");

	song->begin_action_group("delete bars");

	if (bar_dialog_replace_by_pause) {
		int length = 0;
		foreachb(int i, sel) {
			length += song->bars[i]->length;
			song->delete_bar(i, false);
		}
		song->add_pause(sel[0], length, BarEditMode::Ignore);
	} else {

		foreachb(int i, sel)
			song->delete_bar(i, bar_dialog_move_data);// ? Bar::EditMode::STRETCH : Bar::EditMode::IGNORE);
	}

	song->end_action_group();

	request_destroy();
}
