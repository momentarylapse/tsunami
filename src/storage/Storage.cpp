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
#include "../Session.h"
#include "../lib/base/iter.h"
#include "../lib/base/algo.h"
#include "../lib/base/sort.h"
#include "../lib/os/date.h"
#include "../lib/os/filesystem.h"
#include "../lib/hui/config.h"
#include "../lib/hui/language.h"
#include "../lib/hui/common_dlg.h"
#include "../view/helper/Progress.h"
#include "../view/TsunamiWindow.h"
#include "../data/base.h"
#include "../data/Track.h"
#include "../data/TrackLayer.h"
#include "../data/Song.h"
#include "../data/audio/AudioBuffer.h"
#include "../data/SongSelection.h"
#include "../module/audio/BufferStreamer.h"

namespace tsunami {

string Storage::options_in;
string Storage::options_out;
Path Storage::quick_export_directory;
float Storage::default_ogg_quality;
Array<Storage::RecentlyUsedFile> Storage::recently_used_files;

constexpr int MAX_RECENTLY_USED_FILES = 32;


Any Storage::RecentlyUsedFile::to_any() const {
	Any r = Any::EmptyDict;
	r["filename"] = str(filename);
	r["count"] = usage_count;
	r["last"] = (int)last_use.time;
	return r;
}

float Storage::RecentlyUsedFile::significance(const Date& now) const {
	float age = max((float)(now.time - last_use.time), 100.0f);
	float s = (float)usage_count / age;
	if (filename.extension() == "session")
		s *= 4;
	return s;
}

base::optional<Storage::RecentlyUsedFile> Storage::RecentlyUsedFile::parse(const Any& a) {
	if (a.is_dict()) {
		RecentlyUsedFile ruf;
		ruf.filename = str(a["filename"]);
		if (a.has("count"))
			ruf.usage_count = a["count"]._int();
		if (a.has("last"))
			ruf.last_use = Date::from_unix(a["count"]._int());
		return ruf;
	} else if (a.is_string()) {
		// legacy: pure filename
		return Storage::RecentlyUsedFile{a.as_string(), 1, Date::now()};
	}
	return base::None;
}

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

	current_chain_directory = hui::Application::directory_static | "SignalChains";

	hui::config.migrate("QuickExportDir", "Storage.QuickExportDirectory");
	hui::config.migrate("CurrentDirectory", "Storage.CurrentDirectory");
	hui::config.migrate("OggQuality", "Storage.DefaultOggQuality");

	quick_export_directory = hui::config.get_str("Storage.QuickExportDirectory", str(hui::Application::directory));

	default_ogg_quality = hui::config.get_float("Storage.DefaultOggQuality", 0.5f);

	current_directory = hui::config.get_str("Storage.CurrentDirectory", "");

	auto as_list = [](const Any& a) -> Array<Any> {
		if (a.is_list())
			return a.as_list();
		return Any::EmptyList.as_list();
	};

	recently_used_files.clear();
	for (const auto& a: as_list(hui::config.get("Storage.RecentFiles"))) {
		const auto r = RecentlyUsedFile::parse(a);
		if (r.has_value())
			recently_used_files.add(*r);
	}
}

Storage::~Storage() {
	hui::config.set_float("Storage.DefaultOggQuality", default_ogg_quality);
	hui::config.set_str("Storage.CurrentDirectory", current_directory.str());
	hui::config.set_str("Storage.QuickExportDirectory", quick_export_directory.str());
}


bool song_is_simple_audio(Song *s) {
	return ((s->tracks.num == 1) and (s->tracks[0]->type == SignalType::Audio) and (s->tracks[0]->layers.num == 1));
}

bool song_is_simple_midi(Song *s) {
	for (Track* t: weak(s->tracks))
		if ((t->type != SignalType::Midi) and (t->type != SignalType::Beats))
			return false;
	return true;
}

string Storage::suggest_filename(Song *s, const Path &dir) {
	if (s->filename)
		return s->filename.basename();
	const string base = Date::now().format("%Y-%m-%d - %H-%M-%S");

	string ext = "nami";
	if (song_is_simple_audio(s))
		ext = "ogg";
	//else if (song_is_simple_midi(s))
	//	ext = "midi";
	{
		const string name = base + "." + ext;
		if (!os::fs::exists(dir | name))
			return name;
	}

	for (int i=0; i<26; i++) {
		string name = base + "a." + ext;
		name[name.num - ext.num - 2] += i;
		if (!os::fs::exists(dir | name))
			return name;
	}
	return "";
}

base::future<void> Storage::load_ex(Song *song, const Path &filename, Flags flags) {
	current_directory = filename.absolute().parent();
	auto *d = get_format(filename.extension(), 0);
	if (!d)
		return base::failed<void>();

	session->i(_("loading ") + filename.str());

	Format *f = d->create();
	auto od = StorageOperationData(session, f, filename);
	od.song = song;
	od.only_load_metadata = (flags & Flags::OnlyMetadata);
	if (!f->get_parameters(&od, true)) {
		delete f;
		return base::failed<void>();
	}

	od.start_progress(_("loading ") + d->description);

	song->out_start_loading.notify();
	song->reset();
	song->action_manager->enable(false);
	song->filename = filename;

	f->load_song(&od);


	song->action_manager->enable(true);

	if (song->tracks.num > 0)
	{}//	a->SetCurTrack(a->track[0]);
	song->action_manager->reset();
	song->out_new.notify();
	song->out_changed.notify();
	for (Track *t: weak(song->tracks))
		t->out_changed.notify();
	song->out_finished_loading.notify();

	delete f;
	if (od.errors_encountered)
		return base::failed<void>();
	return base::success();
}

base::future<void> Storage::load(Song *song, const Path &filename) {
	return load_ex(song, filename, Flags::None);
}

base::future<void> Storage::load_track(TrackLayer *layer, const Path &filename, int offset) {
	current_directory = filename.parent();
	auto *d = get_format(filename.extension(), FormatDescriptor::Flag::Audio);
	if (!d)
		return base::failed<void>();

	session->i(_("loading track ") + filename.str());

	Format *f = d->create();
	auto od = StorageOperationData(session, f, filename);
	od.set_layer(layer);
	od.offset = offset;
	if (!f->get_parameters(&od, true)) {
		delete f;
		return base::failed<void>();
	}
	od.start_progress(_("loading ") + d->description);

	od.song->begin_action_group("load track");

	f->load_track(&od);

	od.song->end_action_group();

	delete f;
	if (od.errors_encountered)
		return base::failed<void>();
	return base::success();
}

base::future<AudioBuffer> Storage::load_buffer(const Path &filename) {
	session->i(_("loading buffer ") + filename.str());

	Song *aa = new Song(session, session->sample_rate());
	Track *t = aa->add_track(SignalType::Audio);
	TrackLayer *l = t->layers[0].get();
	base::promise<AudioBuffer> promise;

	current_directory = filename.parent();
	auto *d = get_format(filename.extension(), FormatDescriptor::Flag::Audio);
	if (!d)
		return base::failed<AudioBuffer>();

	Format *f = d->create();
	auto od = StorageOperationData(session, f, filename);
	od.allow_channels_change = true;
	od.set_layer(l);
	od.offset = 0;
	if (!f->get_parameters(&od, true)) {
		delete f;
		return base::failed<AudioBuffer>();
	}
	od.start_progress(_("loading ") + d->description);

	od.song->begin_action_group("load track");

	f->load_track(&od);

	od.song->end_action_group();

	delete f;
	if (od.errors_encountered)
		return base::failed<AudioBuffer>();

	AudioBuffer buf = l->buffers[0];
	delete aa;
	promise(buf);

	return promise.get_future();
}

Path Storage::temp_saving_file(const string &ext) {
	for (int i = 0; i < 1000; i++) {
		Path p = (Tsunami::directory | format("-temp-saving-%03d-.", i)).with(ext);
		if (!os::fs::exists(p))
			return p;
	}
	return (Tsunami::directory | "-temp-saving-.").with(ext);
}

// safety: first write to temp file, then (if successful) move
base::future<void> Storage::save_ex(Song *song, const Path &filename, bool exporting) {
	current_directory = filename.absolute().parent();

	auto d = get_format(filename.extension(), 0);
	if (!d)
		return base::failed<void>();

	auto temp_file = temp_saving_file(filename.extension());

	if (!d->test_compatibility(song) and !exporting)
		session->w(_("data loss when saving in this format!"));
	Format *f = d->create();

	auto od = StorageOperationData(session, f, temp_file);
	od.song = song;
	if (!f->get_parameters(&od, true)) {
		delete f;
		return base::failed<void>();
	}
	od.start_progress(_("saving ") + d->description);

	if (!exporting)
		song->filename = filename;

	f->save_song(&od);

	song->action_manager->mark_current_as_save();
	if (session->win)
		session->win->update_menu();

	if (!od.errors_encountered) {
		try {
			os::fs::move(temp_file, filename);
		} catch (Exception &e) {
			od.error("failed to move temp file to target: " + e.message());
		}
	}

	delete f;
	if (od.errors_encountered)
		return base::failed<void>();
	return base::success();
}

base::future<void> Storage::save(Song *song, const Path &filename) {
	return save_ex(song, filename, false);
}

base::future<void> Storage::_export(Song *song, const Path &filename) {
	return save_ex(song, filename, true);
}

base::future<void> Storage::save_via_renderer(AudioOutPort &r, const Path &filename, int num_samples, const Array<Tag> &tags) {
	auto d = get_format(filename.extension(), FormatDescriptor::Flag::Audio | FormatDescriptor::Flag::Write);
	if (!d)
		return base::failed<void>();

	session->i(_("exporting ") + filename.str());

	Format *f = d->create();
	auto od = StorageOperationData(session, f, filename);
	if (!f->get_parameters(&od, true)) {
		delete f;
		return base::failed<void>();
	}
	od.start_progress(_("exporting"));
	od.renderer = &r;
	od.tags = tags;
	od.num_samples = num_samples;
	f->save_via_renderer(&od);
	delete f;
	if (od.errors_encountered)
		return base::failed<void>();
	return base::success();
}

base::future<void> Storage::render_export_selection(Song *song, const SongSelection &sel, const Path &filename) {
	SongRenderer renderer(song);
	renderer.set_range(sel.range());
	renderer.allow_layers(sel.layers());
	return save_via_renderer(renderer.out, filename, renderer.get_num_samples(), song->tags);
}

Storage::Future Storage::ask_by_flags(hui::Window *win, const string &title, int flags, const Array<string> &opt) {
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
			for (auto&& [i,e]: enumerate(f->extensions)) {
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

	if (flags & FormatDescriptor::Flag::Write) {
		opts.add("defaultextension=nami");
		return hui::file_dialog_save(win, title, current_directory, opts);
	} else {
		return hui::file_dialog_open(win, title, current_directory, opts);
	}
}

Storage::Future Storage::ask_open(hui::Window *win, const Array<string> &opt) {
	return ask_by_flags(win, _("Open file"), FormatDescriptor::Flag::Read, opt);
}

Storage::Future Storage::ask_save(hui::Window *win, const Array<string> &opt) {
	return ask_by_flags(win, _("Save file"), FormatDescriptor::Flag::Write, opt);
}

Storage::Future Storage::ask_open_import(hui::Window *win, const Array<string> &opt) {
	return ask_by_flags(win, _("Import file"), FormatDescriptor::Flag::SingleTrack | FormatDescriptor::Flag::Read, opt);
}

Storage::Future Storage::ask_save_render_export(hui::Window *win, const Array<string> &opt) {
	return ask_by_flags(win, _("Export file"), FormatDescriptor::Flag::SingleTrack | FormatDescriptor::Flag::Audio | FormatDescriptor::Flag::Write, opt);
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

bytes Storage::compress(AudioBuffer &buffer, const string &codec) {
	BufferStreamer bs(&buffer);
	Path filename = temp_saving_file(codec);
	bytes r;

	auto dir0 = this->current_directory;
	auto future = save_via_renderer(bs.out, filename, buffer.length, {});
	future.then([this, &r, buffer, filename] {
			r = os::fs::read_binary(filename);
			session->i(format("compressed buffer... %db   %.1f%%", r.num, 100.0f * (float)r.num / (float)(buffer.length * 2 * buffer.channels)));
			os::fs::_delete(filename);
	});
	base::await(future);
	return r;
}

AudioBuffer Storage::decompress(const string &codec, const bytes &data) {
	Path filename = temp_saving_file(codec);
	os::fs::write_binary(filename, data);

	auto dir0 = this->current_directory;
	AudioBuffer r;
	base::await(load_buffer(filename).then([&r] (const AudioBuffer &buf) {
		r = buf;
	}));

	current_directory = dir0;
	os::fs::_delete(filename);
	return r;
}

Storage::Flags operator|(Storage::Flags a, Storage::Flags b) {
	return (Storage::Flags)((int)a | (int)b);
}

void Storage::mark_file_used(const Path& filename, bool saving) {
	auto f = filename.absolute();

	const auto now = Date::now();

	// already in list?
	int index = base::find_index_by_element(recently_used_files, &RecentlyUsedFile::filename, f);
	if (index >= 0) {
		// -> update
		if (!saving) // don't spam count on every tiny save!
			recently_used_files[index].usage_count ++;
		recently_used_files[index].last_use = now;
	} else {
		// new
		recently_used_files.insert({f, 1, now}, 0);
	}

	// FILTER: prefer the .session over the audio file
	for (int i=0; i<recently_used_files.num; i++)
		for (int k=0; k<recently_used_files.num; k++)
			if (recently_used_files[i].filename == recently_used_files[k].filename.with(".tsunami.session")) {
				recently_used_files.erase(k);
				k --;
			}

	// SORT: significance
	base::inplace_sort(recently_used_files, [now] (const RecentlyUsedFile& a, const RecentlyUsedFile& b) {
		return a.significance(now) >= b.significance(now);
	});

	// FILTER: too many
	while (recently_used_files.num > MAX_RECENTLY_USED_FILES)
		recently_used_files.pop();

	Any ruf = Any::EmptyList;
	for (const auto& ff: recently_used_files)
		ruf.add(ff.to_any());
	hui::config.set("Storage.RecentFiles", ruf);
}

}
