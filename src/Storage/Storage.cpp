/*
 * Storage.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "Storage.h"
#include "Format.h"
#include "FormatWave.h"
#include "FormatRaw.h"
#include "FormatOgg.h"
#include "FormatFlac.h"
#include "FormatGp4.h"
#include "FormatM4a.h"
#include "FormatMidi.h"
#include "FormatMp3.h"
#include "FormatNami.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "../lib/hui/hui.h"
#include "../View/Helper/Progress.h"
#include "../Stuff/Log.h"
#include "../Audio/AudioRenderer.h"
#include "../View/AudioView.h"
#include "../Data/AudioFile.h"

Storage::Storage()
{
	format.add(new FormatNami());
	format.add(new FormatWave());
	format.add(new FormatRaw());
	format.add(new FormatOgg());
	format.add(new FormatFlac());
	format.add(new FormatGp4());
	format.add(new FormatMp3());
	format.add(new FormatM4a());
	format.add(new FormatMidi());

	CurrentDirectory = HuiConfig.getStr("CurrentDirectory", "");
}

Storage::~Storage()
{
	HuiConfig.setStr("CurrentDirectory", CurrentDirectory);

	foreach(Format *f, format)
		delete(f);
	format.clear();
}

bool Storage::Load(AudioFile *a, const string &filename)
{
	msg_db_f("Storage.Load", 1);
	bool ok = false;
	bool found = false;

	CurrentDirectory = filename.dirname();
	string ext = filename.extension();

	foreach(Format *f, format)
		if (f->CanHandle(ext)){
			tsunami->progress->Start(_("lade"), 0);

			a->NotifyBegin();

			a->Reset();
			a->action_manager->Enable(false);
			a->filename = filename;

			f->LoadAudio(a, filename);


			a->action_manager->Enable(true);
			//tsunami->progress->Set("peaks", 1);

			tsunami->progress->End();
			if (a->track.num > 0)
			{}//	a->SetCurTrack(a->track[0]);
			a->action_manager->Reset();
			a->Notify(a->MESSAGE_CHANGE);
			a->NotifyEnd();
			ok = (a->track.num > 0);
			found = true;
			break;
		}

	if (!found)
		tsunami->log->Error(_("unbekannte Dateiendung: ") + ext);

	return ok;
}

bool Storage::LoadTrack(Track *t, const string &filename, int offset, int level)
{
	msg_db_f("Storage.LoadTrack", 1);
	bool ok = false;
	bool found = false;

	CurrentDirectory = filename.dirname();
	string ext = filename.extension();

	foreach(Format *f, format)
		if (f->CanHandle(ext)){
			tsunami->progress->Start(_("lade"), 0);

			AudioFile *a = t->root;
			a->NotifyBegin();
			a->action_manager->BeginActionGroup();

			f->LoadTrack(t, filename, offset, level);

			tsunami->progress->End();
			a->action_manager->EndActionGroup();
			a->NotifyEnd();
			found = true;
			break;
		}

	if (!found)
		tsunami->log->Error(_("unbekannte Dateiendung: ") + ext);

	return ok;
}

bool Storage::LoadBufferBox(AudioFile *a, BufferBox *buf, const string &filename)
{
	msg_db_f("Storage.LoadBufferBox", 1);
	AudioFile *aa = new AudioFile;
	aa->NewWithOneTrack(Track::TYPE_AUDIO, a->sample_rate);
	Track *t = aa->track[0];
	bool ok = LoadTrack(t, filename, 0, 0);
	buf->resize(t->level[0].buffer[0].num);
	buf->set(t->level[0].buffer[0], 0, 1);
	delete(aa);
	return ok;
}

bool Storage::Save(AudioFile *a, const string &filename)
{
	msg_db_f("Storage.Save", 1);
	bool ok = false;
	bool found = false;

	CurrentDirectory = filename.dirname();
	string ext = filename.extension();

	foreach(Format *f, format)
		if (f->CanHandle(ext)){
			if (!f->TestFormatCompatibility(a))
				tsunami->log->Warning(_("Datenverlust!"));

			tsunami->progress->Start(_("speichere"), 0);

			a->filename = filename;

			f->SaveAudio(a, filename);

			a->action_manager->MarkCurrentAsSave();
			tsunami->progress->End();
			if (tsunami->win)
				tsunami->win->UpdateMenu();
			ok = true;
			found = true;
			break;
		}

	if (!found)
		tsunami->log->Error(_("unbekannte Dateiendung: ") + ext);

	return ok;
}

bool Storage::Export(AudioFile *a, const Range &r, const string &filename)
{
	msg_db_f("Storage.Export", 1);
	bool ok = false;
	bool found = false;

	CurrentDirectory = filename.dirname();
	string ext = filename.extension();

	foreach(Format *f, format)
		if (f->CanHandle(ext)){
			tsunami->progress->Start(_("exportiere"), 0);

			// render audio...
			tsunami->progress->Set(_("rendere Audio"), 0);
			BufferBox buf;
			tsunami->renderer->RenderAudioFile(a, r, buf);

			// save
			f->SaveBuffer(a, &buf, filename);

			tsunami->progress->End();
			ok = true;
			found = true;
			break;
		}

	if (!found)
		tsunami->log->Error(_("unbekannte Dateiendung: ") + ext);

	return ok;
}

bool Storage::AskByFlags(HuiWindow *win, const string &title, bool save, int flags)
{
	string filter, filter_show;
	filter_show = _("alles m&ogliche (");
	bool first = true;
	foreach(Format *f, format)
		if ((f->flags & flags) == flags){
			foreach(string &e, f->extensions){
				if (!first)
					filter += ";";
				filter += "*." + e;
				if (!first)
					filter_show += ", ";
				filter_show += "*." + e;
				first = false;
			}
		}
	filter_show += ")|" + _("alle Dateien");
	filter += "|*";
	foreach(Format *f, format)
		if ((f->flags & flags) == flags){
			filter += "|";
			filter_show += "|" + f->description + " (";
			foreachi(string &e, f->extensions, i){
				if (i > 0){
					filter += ";";
					filter_show += ", ";
				}
				filter += "*." + e;
				filter_show += "*." + e;
			}
			filter_show += ")";
		}
	if (save)
		return HuiFileDialogSave(win, title, CurrentDirectory, filter_show, filter);
	else
		return HuiFileDialogOpen(win, title, CurrentDirectory, filter_show, filter);
}

bool Storage::AskOpen(HuiWindow *win)
{
	return AskByFlags(win, _("Datei &offnen"), false, Format::FLAG_READ);
}

bool Storage::AskSave(HuiWindow *win)
{
	return AskByFlags(win, _("Datei speichern"), true, Format::FLAG_WRITE);
}

bool Storage::AskOpenImport(HuiWindow *win)
{
	return AskByFlags(win, _("Datei importieren"), false, Format::FLAG_SINGLE_TRACK | Format::FLAG_READ);
}

bool Storage::AskSaveExport(HuiWindow *win)
{
	return AskByFlags(win, _("Datei exportieren"), true, Format::FLAG_SINGLE_TRACK | Format::FLAG_WRITE);
}
