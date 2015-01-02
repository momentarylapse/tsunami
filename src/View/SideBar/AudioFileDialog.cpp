/*
 * AudioFileDialog.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioFileDialog.h"
#include "../../Tsunami.h"
#include "../../Stuff/Observer.h"
#include "../../Data/AudioFile.h"
#include "../Helper/BarList.h"
#include "../../Stuff/Log.h"

AudioFileDialog::AudioFileDialog(AudioFile *a) :
	SideBarConsole(_("Datei-Eigenschaften")),
	Observer("AudioFileDialog")
{
	audio = a;

	// dialog
//	SetTarget("audio_dialog_table", 0);
	setBorderWidth(5);
	embedDialog("audio_file_dialog", 0, 0);
	setDecimals(1);
	bar_list = new BarList(this, "audio_bar_list", "audio_add_bar", "audio_add_bar_pause", "audio_delete_bar");
	hideControl("ad_t_bars", true);

	expand("ad_t_tags", 0, true);

	loadData();

	setTooltip("tags", _("Vorschlag:\n* title\n* artist\n* album\n* tracknumber\n* year/date\n* genre"));

	eventX("tags", "hui:select", this, &AudioFileDialog::onTagsSelect);
	eventX("tags", "hui:change", this, &AudioFileDialog::onTagsEdit);
	event("add_tag", this, &AudioFileDialog::onAddTag);
	event("delete_tag", this, &AudioFileDialog::onDeleteTag);

	subscribe(audio);
}

AudioFileDialog::~AudioFileDialog()
{
	unsubscribe(audio);
	delete(bar_list);
}

void AudioFileDialog::loadData()
{
	// tags
	reset("tags");
	foreach(Tag &t, audio->tag)
		addString("tags", t.key + "\\" + t.value);
	enable("delete_tag", false);

	// data
	reset("data_list");
	int samples = audio->getRange().length();
	addString("data_list", _("Anfang\\") + audio->get_time_str_long(audio->getRange().start()));
	addString("data_list", _("Ende\\") + audio->get_time_str_long(audio->getRange().end()));
	addString("data_list", _("Dauer\\") + audio->get_time_str_long(samples));
	addString("data_list", _("Samples\\") + i2s(samples));
	addString("data_list", _("Abtastrate\\") + i2s(audio->sample_rate) + " Hz");
	addString("data_list", _("Format\\16 bit stereo (nami)"));
}


string get_vol(float volume, bool muted)
{
	return muted ? _("(stumm)") : format("%.1f%%", volume * 100.0f);
}

void AudioFileDialog::onTagsSelect()
{
	int s = getInt("tags");
	enable("delete_tag", s >= 0);
}

void AudioFileDialog::onTagsEdit()
{
	int r = HuiGetEvent()->row;
	if (r < 0)
		return;
	Tag t = audio->tag[r];
	if (HuiGetEvent()->column == 0)
		t.key = getCell("tags", r, 0);
	else
		t.value = getCell("tags", r, 1);
	audio->editTag(r, t.key, t.value);
}

void AudioFileDialog::onAddTag()
{
	audio->addTag("key", "value");
}

void AudioFileDialog::onDeleteTag()
{
	int s = getInt("tags");
	if (s >= 0)
		audio->deleteTag(s);
}

void AudioFileDialog::onUpdate(Observable *o, const string &message)
{
	loadData();
}
