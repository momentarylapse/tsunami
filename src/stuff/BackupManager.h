/*
 * BackupManager.h
 *
 *  Created on: 05.03.2018
 *      Author: michi
 */

#ifndef SRC_STUFF_BACKUPMANAGER_H_
#define SRC_STUFF_BACKUPMANAGER_H_

#include "../lib/base/base.h"
#include "../lib/file/path.h"

enum class BackupMode {
	NONE,
	TEMP,
	KEEP
};

class FileStream;
class Session;

class BackupManager {
public:
	//BackupManager();
	//virtual ~BackupManager();

	struct BackupFile {
		Session *session;
		Path filename;
		FileStream *f;
		int uuid;
	};
	static Array<BackupFile> files;
	static int next_uuid;
	static BackupFile* _find_by_file(FileStream *f);
	static BackupFile* _find_by_uuid(int uuid);
	static void _clear_old();

	static void set_save_state(Session *sessoin);
	static void check_old_files(Session *session);

	static Path get_filename(const string &extension);
	static FileStream *create_file(const string &extension, Session *session);
	static void abort(FileStream *f);
	static void done(FileStream *f);
	static void delete_old(int uuid);

	static Path get_filename_for_uuid(int uuid);
};

#endif /* SRC_STUFF_BACKUPMANAGER_H_ */
