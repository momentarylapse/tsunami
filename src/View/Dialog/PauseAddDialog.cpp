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

	event("ok", std::bind(&PauseAddDialog::onOk, this));
	event("cancel", std::bind(&PauseAddDialog::onClose, this));
	event("hui:close", std::bind(&PauseAddDialog::onClose, this));
}

void PauseAddDialog::onOk()
{
	float duration = getFloat("duration");
	song->action_manager->beginActionGroup();

	if (!song->getTimeTrack())
		song->addTrack(Track::TYPE_TIME, 0);

	int index = max(0, bars.end());

	song->addPause(index, duration, view->bars_edit_data);
	song->action_manager->endActionGroup();

	destroy();
}

void PauseAddDialog::onClose()
{
	destroy();
}
