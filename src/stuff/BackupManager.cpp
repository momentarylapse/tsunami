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

namespace tsunami {

Array<BackupManager::BackupFile> BackupManager::files;
int BackupManager::next_uuid;

Path BackupManager::get_filename(const string &extension) {
	Date d = Date::now();
	string base = d.format("backup-%Y-%m-%d");
	for (int i=0; i<26; i++) {
		string fn = base + "a." + extension;
		fn[fn.num - extension.num - 2] += i;
		if (!os::fs::exists(Tsunami::directory | fn))
			return Tsunami::directory | fn;
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

void BackupManager::check_old_files() {
	_clear_old();

	// update list
	auto _files = os::fs::search(Tsunami::directory, "backup-*", "f");
	for (auto &f: _files) {
		BackupFile bf;
		bf.uuid = next_uuid ++;
		bf.session = nullptr;
		bf.f = nullptr;
		bf.filename = Tsunami::directory | f;
		files.add(bf);
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

BackupManager::BackupFile* BackupManager::_find_by_filename(const Path &filename) {
	for (auto &bf: files)
		if (bf.filename == filename)
			return &bf;
	return nullptr;
}

Path BackupManager::get_filename_for_uuid(int uuid) {
	auto *bf = _find_by_uuid(uuid);
	if (bf)
		return bf->filename;
	return "";
}

int BackupManager::get_uuid_for_filename(const Path &filename) {
	auto *bf = _find_by_filename(filename);
	if (bf)
		return bf->uuid;
	return -1;
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

bool BackupManager::should_notify_found_backups() {
	if (files.num == 0)
		return false;
	auto ln = hui::config.get_str("Backup.LastNotified");
	if (ln == "")
		return true;
	for (auto &f: files)
		if (f.filename.basename_no_ext() > Path(ln).basename_no_ext())
			return true;
	return false;
}

void BackupManager::notify_found_backups_done() {
	if (files.num == 0)
		return;
	hui::config.set_str("Backup.LastNotified", files.back().filename.basename());
}

}
