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
#include "../Data/TrackLayer.h"
#include "../Data/Song.h"
#include "../Data/Audio/AudioBuffer.h"
#include "../Data/SongSelection.h"

Storage::Storage(Session *_session) {
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

	current_directory = hui::Config.get_str("CurrentDirectory", "");
	current_chain_directory = hui::Application::directory_static + "SignalChains/";
}

Storage::~Storage() {
	hui::Config.set_str("CurrentDirectory", current_directory);

	for (auto *d: formats)
		delete(d);
	formats.clear();
}

bool Storage::load_ex(Song *song, const string &filename, bool only_metadata) {
	current_directory = filename.dirname();
	FormatDescriptor *d = get_format(filename.extension(), 0);
	if (!d)
		return false;

	session->i(_("loading ") + filename);

	Format *f = d->create();
	auto od = StorageOperationData(session, f, filename, _("loading ") + d->description);
	od.song = song;
	od.only_load_metadata = only_metadata;
	if (!f->get_parameters(&od, true)) {
		delete(f);
		return false;
	}

	song->reset();
	song->action_manager->enable(false);
	song->filename = filename;

	f->load_song(&od);


	song->action_manager->enable(true);

	if (song->tracks.num > 0)
	{}//	a->SetCurTrack(a->track[0]);
	song->action_manager->reset();
	song->notify(song->MESSAGE_NEW);
	song->notify(song->MESSAGE_CHANGE);
	for (Track *t: song->tracks)
		t->notify();
	song->notify(song->MESSAGE_FINISHED_LOADING);

	delete f;
	return !od.errors_encountered;
}

bool Storage::load(Song *song, const string &filename) {
	return load_ex(song, filename, false);
}

bool Storage::load_track(TrackLayer *layer, const string &filename, int offset) {
	current_directory = filename.dirname();
	auto *d = get_format(filename.extension(), FormatDescriptor::Flag::AUDIO);
	if (!d)
		return false;

	session->i(_("loading track ") + filename);

	Format *f = d->create();
	auto od = StorageOperationData(session, f, filename, _("loading ") + d->description);
	od.set_layer(layer);
	od.offset = offset;
	if (!f->get_parameters(&od, true)) {
		delete f;
		return false;
	}

	od.song->begin_action_group();

	f->load_track(&od);

	od.song->end_action_group();

	delete f;
	return !od.errors_encountered;
}

bool Storage::load_buffer(AudioBuffer *buf, const string &filename) {
	session->i(_("loading buffer ") + filename);

	Song *aa = new Song(session, session->sample_rate());
	Track *t = aa->add_track(SignalType::AUDIO);
	TrackLayer *l = t->layers[0];
	bool ok = load_track(l, filename, 0);
	if (l->buffers.num > 0){
		buf->resize(l->buffers[0].length);
		buf->set(l->buffers[0], 0, 1);
	}
	delete aa;
	return ok;
}

bool Storage::save(Song *song, const string &filename) {
	current_directory = filename.dirname();

	FormatDescriptor *d = get_format(filename.extension(), 0);
	if (!d)
		return false;

	session->i(_("saving ") + filename);

	if (!d->test_compatibility(song))
		session->w(_("data loss when saving in this format!"));
	Format *f = d->create();

	auto od = StorageOperationData(session, f, filename, _("saving ") + d->description);
	od.song = song;
	if (!f->get_parameters(&od, true)) {
		delete f;
		return false;
	}

	song->filename = filename;

	f->save_song(&od);

	song->action_manager->mark_current_as_save();
	if (session->win)
		session->win->update_menu();

	delete f;
	return !od.errors_encountered;
}

bool Storage::save_via_renderer(Port *r, const string &filename, int num_samples, const Array<Tag> &tags) {
	FormatDescriptor *d = get_format(filename.extension(), FormatDescriptor::Flag::AUDIO | FormatDescriptor::Flag::WRITE);
	if (!d)
		return false;

	session->i(_("exporting ") + filename);

	Format *f = d->create();
	auto od = StorageOperationData(session, f, filename, _("exporting"));
	if (!f->get_parameters(&od, true)) {
		delete f;
		return false;
	}
	od.renderer = r;
	od.tags = tags;
	od.num_samples = num_samples;
	f->save_via_renderer(&od);
	delete f;
	return !od.errors_encountered;
}

bool Storage::render_export_selection(Song *song, SongSelection *sel, const string &filename) {
	SongRenderer renderer(song);
	renderer.set_range(sel->range());
	renderer.allow_layers(sel->layers());
	return save_via_renderer(renderer.out, filename, renderer.get_num_samples(), song->tags);
}

bool Storage::ask_by_flags(hui::Window *win, const string &title, int flags) {
	string filter, filter_show;
	filter_show = _("all known files");
	bool first = true;
	for (auto *f: formats)
		if ((f->flags & flags) == flags) {
			for (string &e: f->extensions) {
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
	for (auto *f: formats)
		if ((f->flags & flags) == flags){
			filter += "|";
			filter_show += "|" + f->description + " (";
			foreachi(string &e, f->extensions, i) {
				if (i > 0) {
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

bool Storage::ask_open(hui::Window *win) {
	return ask_by_flags(win, _("Open file"), FormatDescriptor::Flag::READ);
}

bool Storage::ask_save(hui::Window *win) {
	return ask_by_flags(win, _("Save file"), FormatDescriptor::Flag::WRITE);
}

bool Storage::ask_open_import(hui::Window *win) {
	return ask_by_flags(win, _("Import file"), FormatDescriptor::Flag::SINGLE_TRACK | FormatDescriptor::Flag::READ);
}

bool Storage::ask_save_render_export(hui::Window *win) {
	return ask_by_flags(win, _("Export file"), FormatDescriptor::Flag::SINGLE_TRACK | FormatDescriptor::Flag::AUDIO | FormatDescriptor::Flag::WRITE);
}


FormatDescriptor *Storage::get_format(const string &ext, int flags) {
	for (auto *d: formats) {
		if (d->can_handle(ext)) {
			if ((d->flags & flags) == flags) {
				return d;
			} else {
				session->e(_("file format is incompatible for this action: ") + ext);
				return nullptr;
			}
		}
	}

	session->e(_("unknown file extension: ") + ext);
	return nullptr;
}
