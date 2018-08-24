/*
 * Storage.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "Storage.h"

#include "../Module/Audio/SongRenderer.h"
#include "Format/Format.h"
#include "Format/FormatWave.h"
#include "Format/FormatRaw.h"
#include "Format/FormatOgg.h"
#include "Format/FormatFlac.h"
#include "Format/FormatM4a.h"
#include "Format/FormatMidi.h"
#include "Format/FormatPdf.h"
#include "Format/FormatMp3.h"
#include "Format/FormatSoundFont2.h"
#include "Format/FormatGuitarPro.h"
#include "Format/FormatNami.h"
#include "../Action/ActionManager.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "../Session.h"
#include "../lib/hui/hui.h"
#include "../View/Helper/Progress.h"
#include "../Stuff/Log.h"
#include "../Data/base.h"
#include "../Data/Track.h"
#include "../Data/Song.h"
#include "../Data/Audio/AudioBuffer.h"

Storage::Storage(Session *_session)
{
	session = _session;
	formats.add(new FormatDescriptorNami());
	formats.add(new FormatDescriptorWave());
	formats.add(new FormatDescriptorRaw());
#if HAS_LIB_OGG
	formats.add(new FormatDescriptorOgg());
#endif
#if HAS_LIB_FLAC
	formats.add(new FormatDescriptorFlac());
#endif
	formats.add(new FormatDescriptorGuitarPro());
	formats.add(new FormatDescriptorSoundFont2());
#ifndef OS_WINDOWS
	formats.add(new FormatDescriptorMp3());
	formats.add(new FormatDescriptorM4a());
#endif
	formats.add(new FormatDescriptorMidi());
	formats.add(new FormatDescriptorPdf());

	current_directory = hui::Config.getStr("CurrentDirectory", "");
	current_chain_directory = hui::Application::directory_static + "SignalChains/";
}

Storage::~Storage()
{
	hui::Config.setStr("CurrentDirectory", current_directory);

	for (FormatDescriptor *d : formats)
		delete(d);
	formats.clear();
}

bool Storage::load_ex(Song *a, const string &filename, bool only_metadata)
{
	current_directory = filename.dirname();
	FormatDescriptor *d = getFormat(filename.extension(), 0);
	if (!d)
		return false;

	session->i(_("loading ") + filename);

	Format *f = d->create();
	StorageOperationData od = StorageOperationData(this, f, a, nullptr, filename, _("loading ") + d->description, session->win);
	od.only_load_metadata = only_metadata;

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
	a->notify(a->MESSAGE_FINISHED_LOADING);

	delete(f);
	return true;
}

bool Storage::load(Song *a, const string &filename)
{
	return load_ex(a, filename, false);
}

bool Storage::loadTrack(TrackLayer *layer, const string &filename, int offset)
{
	current_directory = filename.dirname();
	FormatDescriptor *d = getFormat(filename.extension(), FormatDescriptor::Flag::AUDIO);
	if (!d)
		return false;

	session->i(_("loading track ") + filename);

	Format *f = d->create();
	Song *a = layer->track->song;
	StorageOperationData od = StorageOperationData(this, f, a, layer, filename, _("loading ") + d->description, session->win);
	od.offset = offset;
	od.layer = layer;

	a->beginActionGroup();

	f->loadTrack(&od);
	a->endActionGroup();

	delete(f);
	return true;
}

bool Storage::loadBufferBox(Song *a, AudioBuffer *buf, const string &filename)
{
	session->i(_("loading buffer ") + filename);

	Song *aa = new Song(session, a->sample_rate);
	Track *t = aa->addTrack(SignalType::AUDIO);
	TrackLayer *l = t->layers[0];
	bool ok = loadTrack(l, filename, 0);
	if (l->buffers.num > 0){
		buf->resize(l->buffers[0].length);
		buf->set(l->buffers[0], 0, 1);
	}
	delete(aa);
	return ok;
}

#if 0
bool Storage::saveBufferBox(Song *a, AudioBuffer *buf, const string &filename)
{
	current_directory = filename.dirname();
	Format *f = getFormat(filename.extension(), Format::Flag::AUDIO);
	if (!f)
		return false;

	StorageOperationData od = StorageOperationData(this, f, a, nullptr, buf, filename, _("exporting ") + f->description, tsunami->_win);

	// save
	return _saveBufferBox(&od);
}
#endif

bool Storage::save(Song *a, const string &filename)
{
	current_directory = filename.dirname();

	FormatDescriptor *d = getFormat(filename.extension(), 0);
	if (!d)
		return false;

	session->i(_("saving ") + filename);

	if (!d->testFormatCompatibility(a))
		session->w(_("data loss when saving in this format!"));
	Format *f = d->create();

	StorageOperationData od = StorageOperationData(this, f, a, nullptr, filename, _("saving ") + d->description, session->win);

	a->filename = filename;

	f->saveSong(&od);

	a->action_manager->mark_current_as_save();
	if (session->win)
		session->win->updateMenu();

	delete(f);
	return true;
}

bool Storage::saveViaRenderer(AudioPort *r, const string &filename, int num_samples, const Array<Tag> &tags)
{
	FormatDescriptor *d = getFormat(filename.extension(), FormatDescriptor::Flag::AUDIO);
	if (!d)
		return false;

	session->i(_("exporting ") + filename);

	Format *f = d->create();
	StorageOperationData od = StorageOperationData(this, f, nullptr, nullptr, filename, _("exporting"), session->win);

	od.renderer = r;
	od.tags = tags;
	od.num_samples = num_samples;
	f->saveViaRenderer(&od);
	delete(f);
	return true;
}

bool Storage::askByFlags(hui::Window *win, const string &title, int flags)
{
	string filter, filter_show;
	filter_show = _("all known files");
	bool first = true;
	for (FormatDescriptor *f : formats)
		if ((f->flags & flags) == flags){
			for (string &e : f->extensions){
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
	for (FormatDescriptor *f : formats)
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
	if (flags & FormatDescriptor::Flag::WRITE)
		return hui::FileDialogSave(win, title, current_directory, filter_show, filter);
	else
		return hui::FileDialogOpen(win, title, current_directory, filter_show, filter);
}

bool Storage::askOpen(hui::Window *win)
{
	return askByFlags(win, _("Open file"), FormatDescriptor::Flag::READ);
}

bool Storage::askSave(hui::Window *win)
{
	return askByFlags(win, _("Save file"), FormatDescriptor::Flag::WRITE);
}

bool Storage::askOpenImport(hui::Window *win)
{
	return askByFlags(win, _("Import file"), FormatDescriptor::Flag::SINGLE_TRACK | FormatDescriptor::Flag::READ);
}

bool Storage::askSaveExport(hui::Window *win)
{
	return askByFlags(win, _("Export file"), FormatDescriptor::Flag::SINGLE_TRACK | FormatDescriptor::Flag::WRITE);
}


FormatDescriptor *Storage::getFormat(const string &ext, int flags)
{
	for (FormatDescriptor *d : formats){
		if (d->canHandle(ext)){
			if ((d->flags & flags) == flags){
				return d;
			}else{
				session->e(_("file format is incompatible for this action: ") + ext);
				return nullptr;

			}
		}
	}

	session->e(_("unknown file extension: ") + ext);
	return nullptr;
}
