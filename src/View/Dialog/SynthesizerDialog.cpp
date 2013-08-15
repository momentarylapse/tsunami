/*
 * SynthesizerDialog.cpp
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#include "SynthesizerDialog.h"
#include "../../Tsunami.h"
#include "../../Audio/Synth/Synthesizer.h"

SynthesizerDialog::SynthesizerDialog(HuiWindow* _parent, bool _allow_parent, Track *t) :
	HuiDialog("Synthesizer", 400, 300, _parent, _allow_parent)
{
	AddListView("Name", 0, 0, 0, 0, "synth_list");
	track = t;

	names = FindSynthesizers();
	foreachi(string &name, names, i){
		SetString("synth_list", name);
		if (name == track->synth->name)
			SetInt("synth_list", i);
	}

	EventM("hui:close", this, &SynthesizerDialog::OnClose);
	EventM("synth_list", this, &SynthesizerDialog::OnSelect);
}

SynthesizerDialog::~SynthesizerDialog()
{
}

void SynthesizerDialog::OnSelect()
{
	int n = GetInt("synth_list");
	if (n < 0)
		return;
	Synthesizer *s = CreateSynthesizer(names[n]);
	if (s){
		delete(track->synth);
		track->synth = s;
		delete(this);
	}
}

void SynthesizerDialog::OnClose()
{
	delete(this);
}
