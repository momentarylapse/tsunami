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
#include "Format/FormatGuitarPro.h"
#include "Format/FormatNami.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "../lib/hui/hui.h"
#include "../View/Helper/Progress.h"
#include "../Stuff/Log.h"
#include "../Audio/Renderer/SongRenderer.h"
#include "../Data/Song.h"

Storage::Storage()
{
	formats.add(new FormatDescriptorNami());
	formats.add(new FormatDescriptorWave());
	formats.add(new FormatDescriptorRaw());
	formats.add(new FormatDescriptorOgg());
	formats.add(new FormatDescriptorFlac());
	formats.add(new FormatDescriptorGuitarPro());
	formats.add(new FormatDescriptorSoundFont2());
	formats.add(new FormatDescriptorMp3());
	formats.add(new FormatDescriptorM4a());
	formats.add(new FormatDescriptorMidi());

	current_directory = HuiConfig.getStr("CurrentDirectory", "");
}

Storage::~Storage()
{
	HuiConfig.setStr("CurrentDirectory", current_directory);

	foreach(FormatDescriptor *d, formats)
		delete(d);
	formats.clear();
}

bool Storage::load(Song *a, const string &filename)
{
	msg_db_f("Storage.Load", 1);

	current_directory = filename.dirname();
	FormatDescriptor *d = getFormat(filename.extension(), 0);
	if (!d)
		return false;

	Format *f = d->create();
	StorageOperationData od = StorageOperationData(this, f, a, NULL, NULL, filename, _("loading ") + d->description, tsunami->win);

	a->reset();
	a->action_manager->enable(false);
	a->filename = filename;

	f->loadSong(&od);


	a->action_manager->enable(true);

	if (a->tracks.num > 0)
	{}//	a->SetCurTrack(a->track[0]);
	a->action_manager->reset();
	a->notify(a->MESSAGE_NEW);
	a->notify(a->MESSAGE_CHANGE);

	delete(f);
	return true;
}

bool Storage::loadTrack(Track *t, const string &filename, int offset, int level)
{
	msg_db_f("Storage.LoadTrack", 1);

	current_directory = filename.dirname();
	FormatDescriptor *d = getFormat(filename.extension(), FormatDescriptor::FLAG_AUDIO);
	if (!d)
		return false;

	Format *f = d->create();
	Song *a = t->song;
	StorageOperationData od = StorageOperationData(this, f, a, t, NULL, filename, _("loading ") + d->description, tsunami->win);
	od.offset = offset;
	od.level = level;

	a->action_manager->beginActionGroup();

	f->loadTrack(&od);
	a->action_manager->endActionGroup();

	delete(f);
	return true;
}

bool Storage::loadBufferBox(Song *a, BufferBox *buf, const string &filename)
{
	msg_db_f("Storage.LoadBufferBox", 1);
	Song *aa = new Song;
	aa->newWithOneTrack(Track::TYPE_AUDIO, a->sample_rate);
	Track *t = aa->tracks[0];
	bool ok = loadTrack(t, filename, 0, 0);
	if (t->levels[0].buffers.num > 0){
		buf->resize(t->levels[0].buffers[0].length);
		buf->set(t->levels[0].buffers[0], 0, 1);
	}
	delete(aa);
	return ok;
}

#if 0
bool Storage::saveBufferBox(Song *a, BufferBox *buf, const string &filename)
{
	msg_db_f("Storage.saveBuf", 1);

	current_directory = filename.dirname();
	Format *f = getFormat(filename.extension(), Format::FLAG_AUDIO);
	if (!f)
		return false;

	StorageOperationData od = StorageOperationData(this, f, a, NULL, buf, filename, _("exporting ") + f->description, tsunami->_win);

	// save
	return _saveBufferBox(&od);
}
#endif

bool Storage::save(Song *a, const string &filename)
{
	msg_db_f("Storage.Save", 1);

	current_directory = filename.dirname();

	FormatDescriptor *d = getFormat(filename.extension(), 0);
	if (!d)
		return false;

	if (!d->testFormatCompatibility(a))
		tsunami->log->warn(_("Data loss!"));
	Format *f = d->create();

	StorageOperationData od = StorageOperationData(this, f, a, NULL, NULL, filename, _("saving ") + d->description, tsunami->win);

	a->filename = filename;

	f->saveSong(&od);

	a->action_manager->markCurrentAsSave();
	if (tsunami->win)
		tsunami->win->updateMenu();

	delete(f);
	return true;
}

bool Storage::saveViaRenderer(AudioRenderer *r, const string &filename)
{
	msg_db_f("Storage.saveViaRenderer", 1);
	FormatDescriptor *d = getFormat(filename.extension(), FormatDescriptor::FLAG_AUDIO);
	if (!d)
		return false;

	Format *f = d->create();
	StorageOperationData od = StorageOperationData(this, f, NULL, NULL, NULL, filename, _("exporting"), tsunami->win);

	od.renderer = r;
	f->saveViaRenderer(&od);
	delete(f);
	return true;
}

bool Storage::askByFlags(HuiWindow *win, const string &title, int flags)
{
	string filter, filter_show;
	filter_show = _("all known files");
	bool first = true;
	foreach(FormatDescriptor *f, formats)
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
	filter_show += "|" + _("all files");
	filter += "|*";
	foreach(FormatDescriptor *f, formats)
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
	if (flags & FormatDescriptor::FLAG_WRITE)
		return HuiFileDialogSave(win, title, current_directory, filter_show, filter);
	else
		return HuiFileDialogOpen(win, title, current_directory, filter_show, filter);
}

bool Storage::askOpen(HuiWindow *win)
{
	return askByFlags(win, _("Open file"), FormatDescriptor::FLAG_READ);
}

bool Storage::askSave(HuiWindow *win)
{
	return askByFlags(win, _("Save file"), FormatDescriptor::FLAG_WRITE);
}

bool Storage::askOpenImport(HuiWindow *win)
{
	return askByFlags(win, _("Import file"), FormatDescriptor::FLAG_SINGLE_TRACK | FormatDescriptor::FLAG_READ);
}

bool Storage::askSaveExport(HuiWindow *win)
{
	return askByFlags(win, _("Export file"), FormatDescriptor::FLAG_SINGLE_TRACK | FormatDescriptor::FLAG_WRITE);
}


FormatDescriptor *Storage::getFormat(const string &ext, int flags)
{
	bool found = false;
	foreach(FormatDescriptor *d, formats){
		if (d->canHandle(ext)){
			found = true;
			if ((d->flags & flags) == flags)
				return d;
		}
	}

	if (found)
		tsunami->log->error(_("file format is incompatible for this action: ") + ext);
	else
		tsunami->log->error(_("unknown file extension: ") + ext);
	return NULL;
}
