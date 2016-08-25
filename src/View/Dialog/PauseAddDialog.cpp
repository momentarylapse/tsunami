/*
 * PauseAddDialog.cpp
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#include "PauseAddDialog.h"
#include "../../Data/Song.h"

PauseAddDialog::PauseAddDialog(HuiWindow *root, Song *_s, const Range &_bars, bool _apply_to_midi):
	HuiDialog("", 100, 100, root, false)
{
	fromResource("pause_add_dialog");
	song = _s;
	bars = _bars;
	apply_to_midi = _apply_to_midi;

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

	int index = 0;
	if (after){
		if (bars.length > 0)
			index = bars.end();
	}else{
		if (bars.length > 0)
			index = bars.start();
	}

	song->addPause(index, duration, apply_to_midi);
	song->action_manager->endActionGroup();

	delete(this);
}

void PauseAddDialog::onClose()
{
	delete(this);
}
