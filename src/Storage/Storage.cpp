/*
 * Storage.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "Storage.h"
#include "StorageWave.h"
#include "StorageNami.h"
#include "../Tsunami.h"
#include "../lib/hui/hui.h"

Storage::Storage()
{
	format.add(new StorageNami());
	format.add(new StorageWave());

	CurrentDirectory = HuiConfigReadStr("CurrentDirectory", "");
}

Storage::~Storage()
{
	HuiConfigWriteStr("CurrentDirectory", CurrentDirectory);

	foreach(format, f)
		delete(f);
	format.clear();
}

bool Storage::Load(AudioFile *a, const string &filename)
{
	msg_db_r("LoadFromFile", 1);
	bool ok;

	CurrentDirectory = dirname(filename);
	string ext = file_extension(filename);

	foreach(format, f)
		if (f->CanHandle(ext)){
			tsunami->progress->Start(_("lade"), 0);

			a->NotifyBegin();

			a->Reset();
			a->used = true;
			a->filename = filename;

			f->LoadAudio(a, filename);

/*
	//-----------------------------------------  import wave / ogg / flac
	if ((ext == "wav") or (ext == "ogg") or (ext == "flac")){
		int channels, bits, samples, freq;
		char *data = NULL;
		if (load_audio_file(filename, channels, bits, samples, freq, data)){
			ProgressStatus(_("importiere Daten"), perc_import);
			a->sample_rate = (int)freq;
			Track *t = AddEmptyTrack(a);
			ImportData(t, data, channels, bits, samples);
			ProgressStatus(_("erzeuge Peaks"), perc_peaks);
			UpdatePeaks(t);
			//strcat(a->filename, ".nami");
			delete[](data);
		}
	//-----------------------------------------  load native format
	}else{ //if (ext == "nami")
		format[0]->LoadAudio(a, filename);
	}*/
			tsunami->progress->End();
			tsunami->ForceRedraw();
			if (a->track.num > 0)
				a->SetCurTrack(&a->track[0]);
			else
				a->used = false;
			a->action_manager->Reset();
			a->NotifyEnd();
			ok = a->used;
		}

	msg_db_l(1);
	return ok;
}

bool Storage::Save(AudioFile *a, const string &filename)
{
	msg_db_r("Storage.Save", 1);
	bool ok = false;

	CurrentDirectory = dirname(filename);
	string ext = file_extension(filename);

	foreach(format, f)
		if (f->CanHandle(ext)){
			if (!TestFormatCompatibility(a, f))
				tsunami->log->Warning(_("Datenverlust!"));

			tsunami->progress->Start(_("speichere"), 0);

			a->filename = filename;

			f->SaveAudio(a, filename);

			a->action_manager->MarkCurrentAsSave();
			tsunami->progress->End();
			tsunami->UpdateMenu();
			ok = true;
			break;
		}

	msg_db_l(1);
	return ok;
}

bool Storage::Export(AudioFile *a, const string &filename)
{
	msg_db_r("Storage.Export", 1);
	bool ok = false;

	CurrentDirectory = dirname(filename);
	string ext = file_extension(filename);

	foreach(format, f)
		if (f->CanHandle(ext)){
			tsunami->progress->Start(_("exportiere"), 0);

			f->SaveAudio(a, filename);

			tsunami->progress->End();
			ok = true;
			break;
		}

	msg_db_l(1);
	return ok;
}

bool Storage::TestFormatCompatibility(AudioFile *a, StorageAny *f)
{
	int num_subs = 0;
	int num_fx = a->fx.num;
	foreach(a->track, t){
		num_subs += t.sub.num;
		num_fx += t.fx.num;
	}

	if ((a->track.num > 1) && ((f->flags & StorageAny::FLAG_MULTITRACK) == 0))
		return false;
	/*if ((a->tag.num > 0) && ((f->flags & StorageAny::FLAG_TAGS) == 0))
		return false;*/
	if ((num_fx > 0) && ((f->flags & StorageAny::FLAG_FX) == 0))
		return false;
	if ((num_subs > 0) && ((f->flags & StorageAny::FLAG_SUBS) == 0))
		return false;
	return true;
}

bool Storage::AskByFlags(CHuiWindow *win, const string &title, bool save, int flags)
{
	string filter, filter_show;
	foreach(format, f)
		if ((f->flags & flags) == flags){
			if (filter != "")
				filter += ";";
			filter += "*." + f->extension;
			if (filter_show != "")
				filter_show += ",";
			filter_show += "*." + f->extension;
		}
	if (save)
		return HuiFileDialogSave(win, title, CurrentDirectory, filter_show, filter);
	else
		return HuiFileDialogOpen(win, title, CurrentDirectory, filter_show, filter);
}

bool Storage::AskOpen(CHuiWindow *win)
{
	return AskByFlags(win, _("Datei &offnen"), false, 0);
}

bool Storage::AskSave(CHuiWindow *win)
{
	return AskByFlags(win, _("Datei speichern"), true, 0);
}

bool Storage::AskOpenImport(CHuiWindow *win)
{
	return AskByFlags(win, _("Datei importieren"), false, StorageAny::FLAG_SINGLE_TRACK);
}

bool Storage::AskSaveExport(CHuiWindow *win)
{
	return AskByFlags(win, _("Datei exportieren"), true, StorageAny::FLAG_SINGLE_TRACK);
}
