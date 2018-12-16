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
class AudioPort;
class SongSelection;
class StorageOperationData;
class Tag;
class Session;

class Storage
{
public:
	Storage(Session *session);
	virtual ~Storage();

	bool load(Song *a, const string &filename);
	bool load_ex(Song *a, const string &filename, bool only_metadata);
	bool load_track(TrackLayer *t, const string &filename, int offset = 0);
	bool load_buffer(Song *a, AudioBuffer *buf, const string &filename);
	bool save_via_renderer(AudioPort *r, const string &filename, int num_samples, const Array<Tag> &tags);
	bool render_export_selection(Song *a, SongSelection *sel, const string &filename);
	bool save(Song *a, const string &filename);

	bool ask_by_flags(hui::Window *win, const string &title, int flags);

	bool ask_open(hui::Window *win);
	bool ask_save(hui::Window *win);
	bool ask_open_import(hui::Window *win);
	bool ask_save_export(hui::Window *win);

	FormatDescriptor *get_format(const string &ext, int flags);

//private:
	Array<FormatDescriptor*> formats;
	string current_directory;
	string current_chain_directory;

	Session *session;
};

#endif /* STORAGE_H_ */
