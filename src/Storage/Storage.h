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

class Storage
{
public:
	Storage(Session *session);
	virtual ~Storage();

	bool load(Song *song, const string &filename);
	bool load_ex(Song *song, const string &filename, bool only_metadata);
	bool load_track(TrackLayer *t, const string &filename, int offset = 0);
	bool load_buffer(AudioBuffer *buf, const string &filename);
	bool save_via_renderer(Port *r, const string &filename, int num_samples, const Array<Tag> &tags);
	bool render_export_selection(Song *song, SongSelection *sel, const string &filename);
	bool save(Song *song, const string &filename);

	bool ask_by_flags(hui::Window *win, const string &title, int flags);

	bool ask_open(hui::Window *win);
	bool ask_save(hui::Window *win);
	bool ask_open_import(hui::Window *win);
	bool ask_save_render_export(hui::Window *win);

	FormatDescriptor *get_format(const string &ext, int flags);

//private:
	Array<FormatDescriptor*> formats;
	string current_directory;
	string current_chain_directory;

	Session *session;
};

#endif /* STORAGE_H_ */
