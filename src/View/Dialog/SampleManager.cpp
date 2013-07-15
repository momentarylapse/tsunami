/*
 * SampleManager.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "SampleManager.h"
#include "../../Data/AudioFile.h"
#include "../../Storage/Storage.h"
#include "../../View/AudioView.h"
#include "../../Tsunami.h"

SampleManager::SampleManager(AudioFile *a, HuiWindow* _parent, bool _allow_parent) :
	HuiDialog(_("Sample Manager"), 600, 400, _parent, _allow_parent)
{
	AddControlTable("", 0, 0, 1, 2, "table1");
	SetTarget("table1", 0);
	AddListView(_("!format=TttC\\Name\\Dauer\\Benutzung\\L&oschen"), 0, 0, 0, 0, "sample_list");
	AddControlTable("!noexpandy", 0, 1, 4, 1, "table2");
	SetTarget("table2", 0);
	AddButton(_("Laden..."), 0, 0, 0, 0, "import_from_file");
	AddButton(_("L&oschen"), 1, 0, 0, 0, "delete_sample");
	AddButton(_("Einf&ugen"), 2, 0, 0, 0, "insert_sample");
	AddButton(_("Aus Auswahl"), 3, 0, 0, 0, "create_from_selection");

	SetTooltip("insert_sample", _("f&ugt am Cursor der aktuellen Spur ein"));

	EventM("hui:close", this, &SampleManager::OnClose);
	EventM("import_from_file", this, &SampleManager::OnImportFromFile);
	EventM("insert_sample", this, &SampleManager::OnInsert);
	EventM("create_from_selection", this, &SampleManager::OnCreateFromSelection);
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
	Enable("insert_sample", false);
}

void SampleManager::OnListSelect()
{
	int n = GetInt("");
	Enable("delete_sample", n >= 0);
	Enable("insert_sample", n >= 0);
}

void SampleManager::OnImportFromFile()
{
	if (tsunami->storage->AskOpenImport(this)){
		BufferBox buf;
		tsunami->storage->LoadBufferBox(audio, &buf, HuiFilename);
		audio->AddSample(HuiFilename, buf);
	}
}

void SampleManager::OnInsert()
{
	int n = GetInt("sample_list");
	if (n >= 0)
		tsunami->view->cur_track->AddSample(audio->selection.start(), n);
}

void SampleManager::OnCreateFromSelection()
{
	audio->CreateSubsFromSelection(tsunami->view->cur_level);
}

void SampleManager::OnDelete()
{
	int n = GetInt("sample_list");
	if (n >= 0)
		audio->DeleteSample(n);
}

void SampleManager::OnClose()
{
	Hide();
}

void SampleManager::OnUpdate(Observable *o)
{
	FillList();
}
