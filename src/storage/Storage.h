/*
 * Storage.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef STORAGE_H_
#define STORAGE_H_

#include "../lib/base/future.h"
#include "../lib/base/optional.h"
#include "../lib/os/file.h"
#include "../lib/os/date.h"

namespace hui {
	class Window;
}
class Any;

namespace tsunami {

class Range;
class Format;
class FormatDescriptor;
class Song;
class Track;
class TrackLayer;
class AudioBuffer;
struct AudioOutPort;
class SongSelection;
class StorageOperationData;
class Tag;
class Session;

class Storage {
public:
	Storage(Session *session);
	virtual ~Storage();

	enum Flags {
		None = 0,
		OnlyMetadata = 1,
		Force = 2
	};

	base::future<void> load(Song *song, const Path &filename);
	base::future<void> load_ex(Song *song, const Path &filename, Flags flags);
	base::future<void> load_track(TrackLayer *t, const Path &filename, int offset = 0);
	base::future<AudioBuffer> load_buffer(const Path &filename);
	base::future<void> save_via_renderer(AudioOutPort &r, const Path &filename, int num_samples, const Array<Tag> &tags);
	base::future<void> render_export_selection(Song *song, const SongSelection &sel, const Path &filename);
	base::future<void> save_ex(Song *song, const Path &filename, bool exporting);
	base::future<void> save(Song *song, const Path &filename);
	base::future<void> _export(Song *song, const Path &filename);

	using Future = base::future<Path>;

	Future ask_by_flags(hui::Window *win, const string &title, int flags, const Array<string> &opt = {});

	Future ask_open(hui::Window *win, const Array<string> &opt = {});
	Future ask_save(hui::Window *win, const Array<string> &opt = {});
	Future ask_open_import(hui::Window *win, const Array<string> &opt = {});
	Future ask_save_render_export(hui::Window *win, const Array<string> &opt = {});

	FormatDescriptor *get_format(const string &ext, int flags);

	static Path temp_saving_file(const string &ext);

//private:
	owned_array<FormatDescriptor> formats;
	Path current_directory;
	Path current_chain_directory;
	static Path quick_export_directory;
	static float default_ogg_quality;

	struct RecentlyUsedFile {
		Path filename;
		int usage_count = 0;
		Date last_use;
		Any to_any() const;
		float significance(const Date& now) const;
		static base::optional<RecentlyUsedFile> parse(const Any& a);
	};

	static Array<RecentlyUsedFile> recently_used_files;

	static string options_in;
	static string options_out;

	Session *session;
	bool allow_gui = true;

	bytes compress(AudioBuffer &buffer, const string &codec);
	AudioBuffer decompress(const string &codec, const bytes &data);

	static void mark_file_used(const Path& filename, bool saving);

	static string suggest_filename(Song *s, const Path &dir);
};

Storage::Flags operator|(Storage::Flags a, Storage::Flags b);

}

#endif /* STORAGE_H_ */
