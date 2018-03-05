/*
 * BackupManager.h
 *
 *  Created on: 05.03.2018
 *      Author: michi
 */

#ifndef SRC_STUFF_BACKUPMANAGER_H_
#define SRC_STUFF_BACKUPMANAGER_H_

#include "../lib/base/base.h"

enum
{
	BACKUP_MODE_NONE,
	BACKUP_MODE_TEMP,
	BACKUP_MODE_KEEP
};

class File;

class BackupManager
{
public:
	//BackupManager();
	//virtual ~BackupManager();

	struct BackupFile
	{
		bool from_this_session;
		string filename;
		File *f;
	};
	static Array<BackupFile> files;
	static BackupFile* _find_by_file(File *f);
	static void _clear_old();

	static void set_save_state();
	static void check_old_files();

	static string get_filename(const string &extension);
	static File *get_file(const string &extension);
	static void abort(File *f);
	static void done(File *f);
};

#endif /* SRC_STUFF_BACKUPMANAGER_H_ */
