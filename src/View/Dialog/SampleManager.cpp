/*
 * SampleManager.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "SampleManager.h"
#include "../../Data/AudioFile.h"
#include "../../Storage/Storage.h"
#include "../../Tsunami.h"

SampleManager::SampleManager(AudioFile *a, HuiWindow* _parent, bool _allow_parent) :
	HuiDialog(_("Sample Manager"), 600, 400, _parent, _allow_parent)
{
	AddControlTable("", 0, 0, 1, 2, "table1");
	SetTarget("table1", 0);
	AddListView(_("!format=TttC\\Name\\Dauer\\Benutzung\\L&oschen"), 0, 0, 0, 0, "sample_list");
	AddControlTable("!noexpandy", 0, 1, 2, 1, "table2");
	SetTarget("table2", 0);
	AddButton("+", 0, 0, 0, 0, "add_sample");
	AddButton("-", 1, 0, 0, 0, "delete_sample");

	EventM("add_sample", this, &SampleManager::OnAdd);
	EventM("delete_sample", this, &SampleManager::OnDelete);
	EventMX("sample_list", "hui:select", this, &SampleManager::OnListSelect);

	audio = a;
	FillList();

	Subscribe(audio);
}

SampleManager::~SampleManager()
{
	Unsubscribe(audio);
}

void SampleManager::FillList()
{
	Reset("sample_list");
	foreach(Sample *s, audio->sample)
		SetString("sample_list", s->name + "\\" + audio->get_time_str(s->buf.num) + "\\" + i2s(s->ref_count) + "\\" + b2s(s->auto_delete));
	Enable("delete_sample", false);
}

void SampleManager::OnListSelect()
{
}

void SampleManager::OnAdd()
{
	if (tsunami->storage->AskOpenImport(this)){
		BufferBox buf;
		tsunami->storage->LoadBufferBox(audio, &buf, HuiFilename);
		audio->AddSample(HuiFilename, buf);
	}
}

void SampleManager::OnDelete()
{
}

void SampleManager::OnClose()
{
	delete(this);
}

void SampleManager::OnUpdate(Observable *o)
{
	FillList();
}
