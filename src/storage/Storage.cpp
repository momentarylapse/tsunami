/*
 * Storage.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "Storage.h"

#include "../module/audio/SongRenderer.h"
#include "format/Format.h"
#include "format/FormatWave.h"
#include "format/FormatRaw.h"
#include "format/FormatOgg.h"
#include "format/FormatFlac.h"
#include "format/FormatM4a.h"
#include "format/FormatMidi.h"
#include "format/FormatPdf.h"
#include "format/FormatMp3.h"
#include "format/FormatSoundFont2.h"
#include "format/FormatGuitarPro.h"
#include "format/FormatNami.h"
#include "../action/ActionManager.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "../Session.h"
#include "../lib/hui/hui.h"
#include "../lib/os/filesystem.h"
#include "../view/helper/Progress.h"
#include "../stuff/Log.h"
#include "../data/base.h"
#include "../data/Track.h"
#include "../data/TrackLayer.h"
#include "../data/Song.h"
#include "../data/audio/AudioBuffer.h"
#include "../data/SongSelection.h"

string Storage::options_in;
string Storage::options_out;

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

	current_directory = hui::config.get_str("CurrentDirectory", "");
	current_chain_directory = hui::Application::directory_static << "SignalChains";
}

Storage::~Storage() {
	hui::config.set_str("CurrentDirectory", current_directory.str());

	for (auto *d: formats)
		delete d;
	formats.clear();
}

bool Storage::load_ex(Song *song, const Path &filename, Flags flags) {
	current_directory = filename.absolute().parent();
	auto *d = get_format(filename.extension(), 0);
	if (!d)
		return false;

	session->i(_("loading ") + filename.str());

	Format *f = d->create();
	auto od = StorageOperationData(session, f, filename);
	od.song = song;
	od.only_load_metadata = (flags & Flags::ONLY_METADATA);
	if (!f->get_parameters(&od, true)) {
		delete f;
		return false;
	}

	od.start_progress(_("loading ") + d->description);

	song->notify(song->MESSAGE_START_LOADING);
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
	for (Track *t: weak(song->tracks))
		t->notify();
	song->notify(song->MESSAGE_FINISHED_LOADING);

	delete f;
	return !od.errors_encountered;
}

bool Storage::load(Song *song, const Path &filename) {
	return load_ex(song, filename, Flags::NONE);
}

bool Storage::load_track(TrackLayer *layer, const Path &filename, int offset) {
	current_directory = filename.parent();
	auto *d = get_format(filename.extension(), FormatDescriptor::Flag::AUDIO);
	if (!d)
		return false;

	session->i(_("loading track ") + filename.str());

	Format *f = d->create();
	auto od = StorageOperationData(session, f, filename);
	od.set_layer(layer);
	od.offset = offset;
	if (!f->get_parameters(&od, true)) {
		delete f;
		return false;
	}
	od.start_progress(_("loading ") + d->description);

	od.song->begin_action_group("load track");

	f->load_track(&od);

	od.song->end_action_group();

	delete f;
	return !od.errors_encountered;
}

bool Storage::load_buffer(AudioBuffer *buf, const Path &filename) {
	session->i(_("loading buffer ") + filename.str());

	Song *aa = new Song(session, session->sample_rate());
	Track *t = aa->add_track(SignalType::AUDIO);
	TrackLayer *l = t->layers[0].get();
	bool ok = load_track(l, filename, 0);
	if (l->buffers.num > 0){
		buf->resize(l->buffers[0].length);
		buf->set(l->buffers[0], 0, 1);
	}
	delete aa;
	return ok;
}

Path Storage::temp_saving_file(const string &ext) {
	for (int i = 0; i < 1000; i++) {
		Path p = (tsunami->directory << format("-temp-saving-%03d-.", i)).with(ext);
		if (!os::fs::exists(p))
			return p;
	}
	return (tsunami->directory << "-temp-saving-.").with(ext);
}

// safety: first write to temp file, then (if successful) move
bool Storage::save(Song *song, const Path &filename) {
	current_directory = filename.absolute().parent();

	auto d = get_format(filename.extension(), 0);
	if (!d)
		return false;

	auto temp_file = temp_saving_file(filename.extension());

	if (!d->test_compatibility(song))
		session->w(_("data loss when saving in this format!"));
	Format *f = d->create();

	auto od = StorageOperationData(session, f, temp_file);
	od.song = song;
	if (!f->get_parameters(&od, true)) {
		delete f;
		return false;
	}
	od.start_progress(_("saving ") + d->description);

	song->filename = filename;

	f->save_song(&od);

	song->action_manager->mark_current_as_save();
	if (session->win)
		session->win->update_menu();

	if (!od.errors_encountered) {
		try {
			os::fs::rename(temp_file, filename);
		} catch (Exception &e) {
			od.error("failed to move temp file to target: " + e.message());
		}
	}

	delete f;
	return !od.errors_encountered;
}

bool Storage::save_via_renderer(Port *r, const Path &filename, int num_samples, const Array<Tag> &tags) {
	auto d = get_format(filename.extension(), FormatDescriptor::Flag::AUDIO | FormatDescriptor::Flag::WRITE);
	if (!d)
		return false;

	session->i(_("exporting ") + filename.str());

	Format *f = d->create();
	auto od = StorageOperationData(session, f, filename);
	if (!f->get_parameters(&od, true)) {
		delete f;
		return false;
	}
	od.start_progress(_("exporting"));
	od.renderer = r;
	od.tags = tags;
	od.num_samples = num_samples;
	f->save_via_renderer(&od);
	delete f;
	return !od.errors_encountered;
}

bool Storage::render_export_selection(Song *song, const SongSelection &sel, const Path &filename) {
	SongRenderer renderer(song);
	renderer.set_range(sel.range());
	renderer.allow_layers(sel.layers());
	return save_via_renderer(renderer.port_out[0], filename, renderer.get_num_samples(), song->tags);
}

void Storage::ask_by_flags(hui::Window *win, const string &title, int flags, const hui::FileDialogCallback &cb, const Array<string> &opt) {
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
	Array<string> opts = {"filter=" + filter, "showfilter=" + filter_show};
	opts.append(opt);
	if (flags & FormatDescriptor::Flag::WRITE) {
		opts.add("defaultextension=nami");
		return hui::file_dialog_save(win, title, current_directory, opts, cb);
	} else {
		return hui::file_dialog_open(win, title, current_directory, opts, cb);
	}
}

void Storage::ask_open(hui::Window *win, const hui::FileDialogCallback &cb, const Array<string> &opt) {
	return ask_by_flags(win, _("Open file"), FormatDescriptor::Flag::READ, cb, opt);
}

void Storage::ask_save(hui::Window *win, const hui::FileDialogCallback &cb, const Array<string> &opt) {
	return ask_by_flags(win, _("Save file"), FormatDescriptor::Flag::WRITE, cb, opt);
}

void Storage::ask_open_import(hui::Window *win, const hui::FileDialogCallback &cb, const Array<string> &opt) {
	return ask_by_flags(win, _("Import file"), FormatDescriptor::Flag::SINGLE_TRACK | FormatDescriptor::Flag::READ, cb, opt);
}

void Storage::ask_save_render_export(hui::Window *win, const hui::FileDialogCallback &cb, const Array<string> &opt) {
	return ask_by_flags(win, _("Export file"), FormatDescriptor::Flag::SINGLE_TRACK | FormatDescriptor::Flag::AUDIO | FormatDescriptor::Flag::WRITE, cb, opt);
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

#include "../module/audio/BufferStreamer.h"

bytes Storage::compress(AudioBuffer &buffer, const string &codec) {
	BufferStreamer bs(&buffer);
	Path filename = temp_saving_file(codec);

	auto dir0 = this->current_directory;
	if (!save_via_renderer(bs.port_out[0], filename, buffer.length, {}))
		return {};
	current_directory = dir0;

	auto data = os::fs::read_binary(filename);
	session->i(format("compressed buffer... %db   %.1f%%", data.num, 100.0f * (float)data.num / (float)(buffer.length * 2 * buffer.channels)));
	os::fs::_delete(filename);
	return data;
}

void Storage::decompress(AudioBuffer &buffer, const string &codec, const bytes &data) {
	Path filename = temp_saving_file(codec);
	os::fs::write_binary(filename, data);

	auto dir0 = this->current_directory;
	load_buffer(&buffer, filename);
	current_directory = dir0;
	os::fs::_delete(filename);
}

Storage::Flags operator|(const Storage::Flags a, const Storage::Flags b) {
	return (Storage::Flags)((int)a | (int)b);
}