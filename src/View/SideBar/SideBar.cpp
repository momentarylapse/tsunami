/*
 * SideBar.cpp
 *
 *  Created on: 25.03.2014
 *      Author: michi
 */

#include "SideBar.h"
#include "AudioFileDialog.h"
#include "TrackDialog.h"
#include "SubDialog.h"

SideBar::SideBar(AudioView *view, AudioFile *audio) :
	Observable("SideBar")
{
	AddControlTable("!noexpandx,width=270", 0, 0, 1, 3, "root_grid");
	SetTarget("root_grid", 0);
	AddControlTable("", 0, 0, 2, 1, "button_grid");
	AddSeparator("", 0, 1, 0, 0, "");
	AddControlTable("", 0, 2, 1, 20, "console_grid");
	SetTarget("button_grid", 0);
	AddButton("!noexpandx,flat", 0, 0, 0, 0, "close");
	SetImage("close", "hui:close");
	AddText("!big,expandx,center\\...", 1, 0, 0, 0, "title");
	audio_file_dialog = new AudioFileDialog(audio);
	track_dialog = new TrackDialog(view);
	sub_dialog = new SubDialog(view, audio);
	Embed(audio_file_dialog, "console_grid", 0, 0);
	Embed(track_dialog, "console_grid", 0, 1);
	Embed(sub_dialog, "console_grid", 0, 2);

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
			SetString("title", "!big\\" + ((SideBarConsole*)p)->title);
			p->Show();
		}else
			p->Hide();
	}
	active_console = console;
	Notify("Change");
}

void SideBar::Open(int console)
{
	NotifyBegin();
	Choose(console);
	active_console = console;
	if (!visible)
		Show();
	NotifyEnd();
}

bool SideBar::IsActive(int console)
{
	return (active_console == console) && visible;
}

