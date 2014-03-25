/*
 * SideBar.cpp
 *
 *  Created on: 25.03.2014
 *      Author: michi
 */

#include "SideBar.h"
#include "AudioFileDialog.h"
#include "TrackDialog.h"

SideBar::SideBar(AudioFile *audio) :
	Observable("SideBar")
{
	AddControlTable("!noexpandx,width=260", 0, 0, 1, 3, "root_grid");
	SetTarget("root_grid", 0);
	AddControlTable("", 0, 0, 2, 1, "button_grid");
	AddSeparator("", 0, 1, 0, 0, "");
	AddControlTable("", 0, 2, 1, 20, "console_grid");
	SetTarget("button_grid", 0);
	AddButton("!noexpandx,flat", 0, 0, 0, 0, "close");
	SetImage("close", "hui:close");
	AddText("!big,bold,expandx,center\\...", 1, 0, 0, 0, "title");
	audio_file_dialog = new AudioFileDialog(audio);
	track_dialog = new TrackDialog;
	Embed(audio_file_dialog, "console_grid", 0, 0);
	Embed(track_dialog, "console_grid", 0, 1);

	EventM("close", (HuiPanel*)this, (void(HuiPanel::*)())&SideBar::OnClose);

	Choose(AUDIO_FILE_DIALOG);
	visible = true;
}

SideBar::~SideBar()
{
}

void SideBar::OnClose()
{
	Hide();
}

void SideBar::OnShow()
{
	visible = true;
	Notify("Change");
}

void SideBar::OnHide()
{
	visible = false;
	Notify("Change");
}

void SideBar::Choose(int console)
{
	foreachi(HuiPanel *p, children, i){
		if (i == console){
			SetString("title", "!big,bold\\" + ((SideBarConsole*)p)->title);
			p->Show();
		}else
			p->Hide();
	}
	active_console = console;
	if (visible)
		Notify("Change");
	else
		Show();
}

bool SideBar::IsActive(int console)
{
	return (active_console == console) && visible;
}

void SideBar::SetTrack(Track *t)
{
	track_dialog->SetTrack(t);
}

