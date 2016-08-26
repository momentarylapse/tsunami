/*
 * PauseAddDialog.cpp
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#include "PauseAddDialog.h"
#include "../../Data/Song.h"
#include "../AudioView.h"

PauseAddDialog::PauseAddDialog(HuiWindow *root, Song *s, AudioView *v):
	HuiDialog("", 100, 100, root, false)
{
	fromResource("pause_add_dialog");
	song = s;
	view = v;
	bars = view->sel.bars;

	setFloat("duration", 1.0f);

	check("insert:after", true);

	event("ok", this, &PauseAddDialog::onOk);
	event("cancel", this, &PauseAddDialog::onClose);
	event("hui:close", this, &PauseAddDialog::onClose);
}

void PauseAddDialog::onOk()
{
	float duration = getFloat("duration");
	bool after = isChecked("insert:after");
	song->action_manager->beginActionGroup();

	if (!song->getTimeTrack())
		song->addTrack(Track::TYPE_TIME, 0);

	int index = 0;
	if (after){
		if (bars.length > 0)
			index = bars.end();
	}else{
		if (bars.length > 0)
			index = bars.start();
	}

	song->addPause(index, duration, view->bars_edit_data);
	song->action_manager->endActionGroup();

	if (!after){
		view->sel_raw.offset += song->bars[index].length;
		view->updateSelection();
	}

	delete(this);
}

void PauseAddDialog::onClose()
{
	delete(this);
}
