/*
 * BackupManager.cpp
 *
 *  Created on: 05.03.2018
 *      Author: michi
 */

#include "BackupManager.h"
#include "../data/Track.h"
#include "../Tsunami.h"
#include "../Session.h"
#include "../lib/os/date.h"
#include "../lib/os/file.h"
#include "../lib/os/filesystem.h"

Array<BackupManager::BackupFile> BackupManager::files;
int BackupManager::next_uuid;

Path BackupManager::get_filename(const string &extension) {
	Date d = Date::now();
	string base = d.format("backup-%Y-%m-%d");
	for (int i=0; i<26; i++) {
		string fn = base + "a." + extension;
		fn[fn.num - extension.num - 2] += i;
		if (!os::fs::exists(tsunami->directory << fn))
			return tsunami->directory << fn;
	}
	return "";
}


void BackupManager::set_save_state(Session *session) {
	for (auto &bf: files) {
		if (bf.session == session) {
			session->i(_("deleting backup: ") + bf.filename.str());
			os::fs::_delete(bf.filename);
			bf.session = nullptr; // auto remove
		}
	}
	_clear_old();
}

void BackupManager::check_old_files(Session *session) {
	_clear_old();

	// update list
	auto _files = os::fs::search(tsunami->directory, "backup-*", "f");
	for (auto &f: _files) {
		BackupFile bf;
		bf.uuid = next_uuid ++;
		bf.session = nullptr;
		bf.f = nullptr;
		bf.filename = tsunami->directory << f;
		files.add(bf);
	}

	// check
	for (auto &bf: files) {
		if (!bf.session)
			session->q(_("recording backup found: ") + bf.filename.str(), {format("import-backup-%d:", bf.uuid) + _("import"), format("delete-backup-%d:", bf.uuid) + _("delete")});
	}
}

os::fs::FileStream *BackupManager::create_file(const string &extension, Session *session) {
	BackupFile bf;
	bf.uuid = -1;//next_uuid ++;
	bf.session = session;
	bf.filename = get_filename(extension);
	session->i(_("creating backup: ") + bf.filename.str());
	try {
		bf.f = os::fs::open(bf.filename, "wb");
		files.add(bf);
		return bf.f;
	} catch(os::fs::FileError &e) {
		session->e(e.message());
	}
	return nullptr;
}

void BackupManager::abort(os::fs::FileStream *f) {
	//delete f;
	done(f);
}

void BackupManager::done(os::fs::FileStream *f) {
	delete f;
	auto bf = _find_by_file(f);
	if (bf) {
		bf->session->i(_("backup done: ") + bf->filename.str());
		bf->f = nullptr;
	}
}

BackupManager::BackupFile* BackupManager::_find_by_file(os::fs::FileStream *f) {
	if (!f)
		return nullptr;
	for (auto &bf: files)
		if (bf.f == f)
			return &bf;
	return nullptr;
}

BackupManager::BackupFile* BackupManager::_find_by_uuid(int uuid) {
	for (auto &bf: files)
		if (bf.uuid == uuid)
			return &bf;
	return nullptr;
}

Path BackupManager::get_filename_for_uuid(int uuid) {
	auto *bf = _find_by_uuid(uuid);
	if (bf)
		return bf->filename;
	return "";
}

void BackupManager::delete_old(int uuid) {
	auto *bf = _find_by_uuid(uuid);
	if (bf) {
		Session::GLOBAL->i(_("deleting backup: ") + bf->filename.str());
		os::fs::_delete(bf->filename);
		bf->session = nullptr;
	}
}

void BackupManager::_clear_old() {
	for (int i=0; i<files.num; i++)
		if (!files[i].session) {
			files.erase(i);
			i --;
		}
}
