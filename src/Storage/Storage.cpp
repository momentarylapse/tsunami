/*
 * Storage.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "Storage.h"
#include "Format/Format.h"
#include "Format/FormatWave.h"
#include "Format/FormatRaw.h"
#include "Format/FormatOgg.h"
#include "Format/FormatFlac.h"
#include "Format/FormatM4a.h"
#include "Format/FormatMidi.h"
#include "Format/FormatMp3.h"
#include "Format/FormatSoundFont2.h"
#include "Format/FormatNami.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "../lib/hui/hui.h"
#include "../View/Helper/Progress.h"
#include "../Stuff/Log.h"
#include "../Audio/AudioRenderer.h"
#include "../View/AudioView.h"
#include "../Data/AudioFile.h"
#include "Format/FormatGuitarPro.h"

Storage::Storage()
{
	formats.add(new FormatNami());
	formats.add(new FormatWave());
	formats.add(new FormatRaw());
	formats.add(new FormatOgg());
	formats.add(new FormatFlac());
	formats.add(new FormatGuitarPro());
	formats.add(new FormatSoundFont2());
	formats.add(new FormatMp3());
	formats.add(new FormatM4a());
	formats.add(new FormatMidi());

	current_directory = HuiConfig.getStr("CurrentDirectory", "");
}

Storage::~Storage()
{
	HuiConfig.setStr("CurrentDirectory", current_directory);

	foreach(Format *f, formats)
		delete(f);
	formats.clear();
}

bool Storage::load(AudioFile *a, const string &filename)
{
	msg_db_f("Storage.Load", 1);

	current_directory = filename.dirname();
	Format *f = getFormat(filename.extension(), 0);
	if (!f)
		return false;

	StorageOperationData od = StorageOperationData(a, NULL, NULL, filename, _("lade ") + f->description, tsunami->_win);

	a->reset();
	tsunami->_view->enable(false);
	a->action_manager->enable(false);
	a->filename = filename;

	f->loadAudio(&od);


	tsunami->_view->enable(true);
	a->action_manager->enable(true);

	if (a->tracks.num > 0)
	{}//	a->SetCurTrack(a->track[0]);
	a->action_manager->reset();
	a->notify(a->MESSAGE_NEW);
	a->notify(a->MESSAGE_CHANGE);

	return true;
}

bool Storage::loadTrack(Track *t, const string &filename, int offset, int level)
{
	msg_db_f("Storage.LoadTrack", 1);

	current_directory = filename.dirname();
	Format *f = getFormat(filename.extension(), Format::FLAG_AUDIO);
	if (!f)
		return false;

	AudioFile *a = t->root;
	StorageOperationData od = StorageOperationData(a, t, NULL, filename, _("lade ") + f->description, tsunami->_win);
	od.offset = offset;
	od.level = level;

	a->action_manager->beginActionGroup();

	f->loadTrack(&od);
	a->action_manager->endActionGroup();

	return true;
}

bool Storage::loadBufferBox(AudioFile *a, BufferBox *buf, const string &filename)
{
	msg_db_f("Storage.LoadBufferBox", 1);
	AudioFile *aa = new AudioFile;
	aa->newWithOneTrack(Track::TYPE_AUDIO, a->sample_rate);
	Track *t = aa->tracks[0];
	bool ok = loadTrack(t, filename, 0, 0);
	buf->resize(t->levels[0].buffers[0].num);
	buf->set(t->levels[0].buffers[0], 0, 1);
	delete(aa);
	return ok;
}

bool Storage::saveBufferBox(AudioFile *a, BufferBox *buf, const string &filename)
{
	msg_db_f("Storage.saveBuf", 1);

	current_directory = filename.dirname();
	Format *f = getFormat(filename.extension(), Format::FLAG_AUDIO);
	if (!f)
		return false;

	StorageOperationData od = StorageOperationData(a, NULL, buf, filename, _("exportiere ") + f->description, tsunami->_win);

	// save
	return _saveBufferBox(&od);
}

bool Storage::_saveBufferBox(StorageOperationData *od)
{
	msg_db_f("Storage.saveBuf", 1);

	current_directory = od->filename.dirname();
	Format *f = getFormat(od->filename.extension(), Format::FLAG_AUDIO);
	if (!f)
		return false;

	// save
	f->saveBuffer(od);

	return true;
}

bool Storage::save(AudioFile *a, const string &filename)
{
	msg_db_f("Storage.Save", 1);

	current_directory = filename.dirname();

	Format *f = getFormat(filename.extension(), 0);
	if (!f)
		return false;

	if (!f->testFormatCompatibility(a))
		tsunami->log->warning(_("Datenverlust!"));

	StorageOperationData od = StorageOperationData(a, NULL, NULL, filename, _("speichere ") + f->description, tsunami->_win);

	a->filename = filename;

	f->saveAudio(&od);

	a->action_manager->markCurrentAsSave();
	if (tsunami->win)
		tsunami->win->updateMenu();

	return true;
}

bool Storage::_export(AudioFile *a, const Range &r, const string &filename)
{
	msg_db_f("Storage.Export", 1);
	if (!getFormat(filename.extension(), Format::FLAG_AUDIO))
		return false;

	BufferBox buf;
	StorageOperationData *od = new StorageOperationData(a, NULL, &buf, filename, _("exportiere"), tsunami->_win);

	// render audio...
	od->progress->set(_("rendere Audio"), 0);
	AudioRenderer renderer;
	renderer.renderAudioFile(a, r, buf);
	delete(od);

	return _saveBufferBox(od);
}

bool Storage::askByFlags(HuiWindow *win, const string &title, int flags)
{
	string filter, filter_show;
	filter_show = _("alles m&ogliche");
	bool first = true;
	foreach(Format *f, formats)
		if ((f->flags & flags) == flags){
			foreach(string &e, f->extensions){
				if (!first)
					filter += ";";
				filter += "*." + e;
				/*if (!first)
					filter_show += ", ";
				filter_show += "*." + e;*/
				first = false;
			}
		}
	filter_show += "|" + _("alle Dateien");
	filter += "|*";
	foreach(Format *f, formats)
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
	if (flags & Format::FLAG_WRITE)
		return HuiFileDialogSave(win, title, current_directory, filter_show, filter);
	else
		return HuiFileDialogOpen(win, title, current_directory, filter_show, filter);
}

bool Storage::askOpen(HuiWindow *win)
{
	return askByFlags(win, _("Datei &offnen"), Format::FLAG_READ);
}

bool Storage::askSave(HuiWindow *win)
{
	return askByFlags(win, _("Datei speichern"), Format::FLAG_WRITE);
}

bool Storage::askOpenImport(HuiWindow *win)
{
	return askByFlags(win, _("Datei importieren"), Format::FLAG_SINGLE_TRACK | Format::FLAG_READ);
}

bool Storage::askSaveExport(HuiWindow *win)
{
	return askByFlags(win, _("Datei exportieren"), Format::FLAG_SINGLE_TRACK | Format::FLAG_WRITE);
}


Format *Storage::getFormat(const string &ext, int flags)
{
	bool found = false;
	foreach(Format *f, formats)
		if (f->canHandle(ext)){
			found = true;
			if ((f->flags & flags) == flags)
				return f;
		}

	if (found)
		tsunami->log->error(_("inkompatibles Dateiformat f&ur diese Aktion: ") + ext);
	else
		tsunami->log->error(_("unbekannte Dateiendung: ") + ext);
	return NULL;
}
