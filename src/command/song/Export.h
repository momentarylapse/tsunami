/*
 * Export.h
 *
 *  Created on: Oct 23, 2022
 *      Author: michi
 */

#ifndef SRC_COMMAND_SONG_EXPORT_H_
#define SRC_COMMAND_SONG_EXPORT_H_

#include "../../lib/base/base.h"
#include "../../lib/base/pointer.h"
#include "../../lib/base/future.h"

class Song;
class Track;
class SongSelection;
class Path;

shared<Song> copy_song_from_selection(Song *song, const SongSelection &sel);
base::future<void> export_selection(Song *song, const SongSelection& sel, const Path& filename, bool force_unmute = false);

#endif /* SRC_COMMAND_SONG_EXPORT_H_ */
