/*
 * Storage.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef STORAGE_H_
#define STORAGE_H_

#include "../lib/file/file.h"
#include "../lib/hui/hui.h"

class Range;
class Format;
class FormatDescriptor;
class Song;
class Track;
class TrackLayer;
class AudioBuffer;
class Port;
class SongSelection;
class StorageOperationData;
class Tag;
class Session;

class Storage {
public:
	Storage(Session *session);
	virtual ~Storage();

	bool _cdecl load(Song *song, const Path &filename);
	bool _cdecl load_ex(Song *song, const Path &filename, bool only_metadata);
	bool _cdecl load_track(TrackLayer *t, const Path &filename, int offset = 0);
	bool _cdecl load_buffer(AudioBuffer *buf, const Path &filename);
	bool _cdecl save_via_renderer(Port *r, const Path &filename, int num_samples, const Array<Tag> &tags);
	bool _cdecl render_export_selection(Song *song, const SongSelection &sel, const Path &filename);
	bool _cdecl save(Song *song, const Path &filename);

	bool _cdecl ask_by_flags(hui::Window *win, const string &title, int flags);

	bool _cdecl ask_open(hui::Window *win);
	bool _cdecl ask_save(hui::Window *win);
	bool _cdecl ask_open_import(hui::Window *win);
	bool _cdecl ask_save_render_export(hui::Window *win);

	FormatDescriptor *get_format(const string &ext, int flags);

	static Path temp_saving_file(const string &ext);

//private:
	Array<FormatDescriptor*> formats;
	Path current_directory;
	Path current_chain_directory;

	static string options_in;
	static string options_out;

	Session *session;
};

#endif /* STORAGE_H_ */
