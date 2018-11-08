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
	from_resource("bar_delete_dialog");
	song = s;
	for (int i=_bars.start(); i<_bars.end(); i++)
		sel.add(i);

	check("shift-data", true);
	//enable("replace-by-pause", false);

	event("ok", [&]{ on_ok(); });
	event("cancel", [&]{ on_close(); });
	event("hui:close", [&]{ on_close(); });
	event("replace-by-pause", [&]{ on_replace_by_pause(); });
}

void BarDeleteDialog::on_replace_by_pause()
{
	enable("shift-data", !is_checked(""));
}

void BarDeleteDialog::on_ok()
{
	bool move_data = is_checked("shift-data");
	bool replace_by_pause = is_checked("replace-by-pause");

	song->begin_action_group();

	if (replace_by_pause){
		int length = 0;
		foreachb(int i, sel){
			length += song->bars[i]->length;
			song->delete_bar(i, false);
		}
		song->add_pause(sel[0], length, Bar::EditMode::IGNORE);
	}else{

		foreachb(int i, sel)
			song->delete_bar(i, move_data);// ? Bar::EditMode::STRETCH : Bar::EditMode::IGNORE);
	}

	song->end_action_group();

	destroy();
}

void BarDeleteDialog::on_close()
{
	destroy();
}
