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
}

bool Storage::Load(AudioFile *a, const string &filename)
{
	msg_db_r("LoadFromFile", 1);
//	ProgressStart(_("lade"), 0);

	a->NotifyBegin();

	a->Reset();
	a->used = true;
//	SelectTrack(t, false);
	a->filename = filename;

	CurrentDirectory = dirname(filename);

	string ext = file_extension(filename);
	//msg_write(ext);

	foreach(format, f)
		if (f->CanHandle(ext))
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
//	ProgressEnd();
	tsunami->ForceRedraw();
	if (a->track.num > 0){
//		a->OptimizeView();
		a->SetCurTrack(&a->track[0]);
	}else
		a->used = false;
	a->action_manager->Reset();
	a->NotifyEnd();

	msg_db_l(1);
	return true;
}

bool Storage::Save(AudioFile *a, const string &filename)
{
	return false;
}

bool Storage::AskOpen(CHuiWindow *win)
{
	return HuiFileDialogOpen(win, _("Datei &offnen"), CurrentDirectory, "*.nami,*.wav,*.ogg,*.flac", "*.wav;*.ogg;*flac;*.nami");
}

bool Storage::AskSave(CHuiWindow *win)
{
	return HuiFileDialogSave(win, _("Datei speichern"), CurrentDirectory, "*.nami,*.wav,*.ogg,*.flac", "*.wav;*.ogg;*flac;*.nami");
}

bool Storage::AskOpenImport(CHuiWindow *win)
{
	return HuiFileDialogOpen(win, _("Datei importieren"), CurrentDirectory, "**.wav,*.ogg,*.flac", "*.wav;*.ogg;*flac");
}

bool Storage::AskSaveExport(CHuiWindow *win)
{
	return HuiFileDialogSave(win, _("Datei exportieren"), CurrentDirectory, "*.wav,*.ogg,*.flac", "*.wav;*.ogg;*flac");
}
