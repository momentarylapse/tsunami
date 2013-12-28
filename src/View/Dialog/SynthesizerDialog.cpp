/*
 * SynthesizerDialog.cpp
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#include "SynthesizerDialog.h"
#include "../../Tsunami.h"
#include "../../Audio/Synth/Synthesizer.h"

static Synthesizer *SynthesizerDialogReturn = NULL;

SynthesizerDialog::SynthesizerDialog(HuiWindow* _parent, const string &old_name) :
	HuiDialog("Synthesizer", 400, 300, _parent, false)
{
	AddListView("Name", 0, 0, 0, 0, "synth_list");

	names = FindSynthesizers();
	foreachi(string &name, names, i){
		SetString("synth_list", name);
		if (name == old_name)
			SetInt("synth_list", i);
	}

	EventM("hui:close", this, &SynthesizerDialog::OnClose);
	EventM("synth_list", this, &SynthesizerDialog::OnSelect);

	SynthesizerDialogReturn = NULL;
}

SynthesizerDialog::~SynthesizerDialog()
{
}

void SynthesizerDialog::OnSelect()
{
	int n = GetInt("synth_list");
	if (n < 0)
		return;
	SynthesizerDialogReturn = CreateSynthesizer(names[n]);
	delete(this);
}

void SynthesizerDialog::OnClose()
{
	delete(this);
}

Synthesizer *ChooseSynthesizer(HuiWindow *parent, const string &old_name)
{
	SynthesizerDialog *dlg = new SynthesizerDialog(parent, old_name);
	dlg->Run();
	return SynthesizerDialogReturn;
}
