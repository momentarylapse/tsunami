/*
 * BackupManager.h
 *
 *  Created on: 05.03.2018
 *      Author: michi
 */

#ifndef SRC_STUFF_BACKUPMANAGER_H_
#define SRC_STUFF_BACKUPMANAGER_H_

#include "../lib/base/base.h"
#include "../lib/os/path.h"
#include "../lib/pattern/Observable.h"

namespace os::fs {
	class FileStream;
}

namespace tsunami {

enum class BackupMode {
	None,
	Temporary,
	Keep
};

class Session;

class BackupManager : public obs::Node<VirtualBase> {
public:
	BackupManager();

	struct BackupFile {
		Session *session;
		Path filename;
		os::fs::FileStream *f;
		int uuid;
	};

	Array<BackupFile> files;
	static int next_uuid;
	BackupFile* _find_by_file(os::fs::FileStream *f);
	BackupFile* _find_by_uuid(int uuid);
	BackupFile* _find_by_filename(const Path &filename);
	void _clear_old();

	void set_save_state(Session *sessoin);
	void check_old_files();

	static Path get_filename(const string &extension);
	os::fs::FileStream *create_file(const string &extension, Session *session);
	void abort(os::fs::FileStream *f);
	void done(os::fs::FileStream *f);
	void delete_old(int uuid);

	Path get_filename_for_uuid(int uuid);
	int get_uuid_for_filename(const Path &filename);

	bool should_notify_found_backups();
	void notify_found_backups_done();
};

}

#endif /* SRC_STUFF_BACKUPMANAGER_H_ */
