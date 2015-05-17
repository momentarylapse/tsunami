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

bool Storage::load(AudioFile *a, const string &filename)
{
	msg_db_f("Storage.Load", 1);

	CurrentDirectory = filename.dirname();
	Format *f = getFormat(filename.extension(), 0);
	if (!f)
		return false;

	tsunami->progress->start(_("lade"), 0);
	tsunami->_view->unsubscribe(a);

	a->reset();
	a->action_manager->enable(false);
	a->filename = filename;

	f->loadAudio(a, filename);


	tsunami->_view->subscribe(a);
	a->action_manager->enable(true);
	//tsunami->progress->Set("peaks", 1);

	tsunami->progress->end();
	if (a->track.num > 0)
	{}//	a->SetCurTrack(a->track[0]);
	a->action_manager->reset();
	a->notify(a->MESSAGE_NEW);
	a->notify(a->MESSAGE_CHANGE);

	return true;
}

bool Storage::loadTrack(Track *t, const string &filename, int offset, int level)
{
	msg_db_f("Storage.LoadTrack", 1);

	CurrentDirectory = filename.dirname();
	Format *f = getFormat(filename.extension(), Format::FLAG_AUDIO);
	if (!f)
		return false;

	tsunami->progress->start(_("lade"), 0);

	AudioFile *a = t->root;
	a->action_manager->beginActionGroup();

	f->loadTrack(t, filename, offset, level);

	tsunami->progress->end();
	a->action_manager->endActionGroup();

	return true;
}

bool Storage::loadBufferBox(AudioFile *a, BufferBox *buf, const string &filename)
{
	msg_db_f("Storage.LoadBufferBox", 1);
	AudioFile *aa = new AudioFile;
	aa->newWithOneTrack(Track::TYPE_AUDIO, a->sample_rate);
	Track *t = aa->track[0];
	bool ok = loadTrack(t, filename, 0, 0);
	buf->resize(t->level[0].buffer[0].num);
	buf->set(t->level[0].buffer[0], 0, 1);
	delete(aa);
	return ok;
}

bool Storage::saveBufferBox(AudioFile *a, BufferBox *buf, const string &filename)
{
	msg_db_f("Storage.saveBuf", 1);

	CurrentDirectory = filename.dirname();
	Format *f = getFormat(filename.extension(), Format::FLAG_AUDIO);
	if (!f)
		return false;

	tsunami->progress->start(_("exportiere"), 0);

	// save
	f->saveBuffer(a, buf, filename);

	tsunami->progress->end();

	return true;
}

bool Storage::save(AudioFile *a, const string &filename)
{
	msg_db_f("Storage.Save", 1);

	CurrentDirectory = filename.dirname();

	Format *f = getFormat(filename.extension(), 0);
	if (!f)
		return false;

	if (!f->testFormatCompatibility(a))
		tsunami->log->warning(_("Datenverlust!"));

	tsunami->progress->start(_("speichere"), 0);

	a->filename = filename;

	f->saveAudio(a, filename);

	a->action_manager->markCurrentAsSave();
	tsunami->progress->end();
	if (tsunami->win)
		tsunami->win->updateMenu();

	return true;
}

bool Storage::_export(AudioFile *a, const Range &r, const string &filename)
{
	msg_db_f("Storage.Export", 1);
	if (!getFormat(filename.extension(), Format::FLAG_AUDIO))
		return false;

	tsunami->progress->start(_("exportiere"), 0);

	// render audio...
	tsunami->progress->set(_("rendere Audio"), 0);
	BufferBox buf;
	AudioRenderer renderer;
	renderer.renderAudioFile(a, r, buf);

	bool ok = saveBufferBox(a, &buf, filename);

	tsunami->progress->end();
	return ok;
}

bool Storage::askByFlags(HuiWindow *win, const string &title, int flags)
{
	string filter, filter_show;
	filter_show = _("alles m&ogliche");
	bool first = true;
	foreach(Format *f, format)
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
	if (flags & Format::FLAG_WRITE)
		return HuiFileDialogSave(win, title, CurrentDirectory, filter_show, filter);
	else
		return HuiFileDialogOpen(win, title, CurrentDirectory, filter_show, filter);
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
	foreach(Format *f, format)
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
