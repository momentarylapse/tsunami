/*
 * BackupManager.cpp
 *
 *  Created on: 05.03.2018
 *      Author: michi
 */

#include "BackupManager.h"
#include "../Data/Track.h"
#include "../Tsunami.h"
#include "../Session.h"

Array<BackupManager::BackupFile> BackupManager::files;

string BackupManager::get_filename(const string &extension)
{
	Date d = get_current_date();
	string base = tsunami->directory + d.format("backup-%Y-%m-%d");
	for (int i=0; i<26; i++){
		string fn = base + "a." + extension;
		fn[fn.num - extension.num - 2] += i;
		msg_write(fn);
		if (!file_test_existence(fn))
			return fn;
	}
	return "";
}


void BackupManager::set_save_state(Session *session)
{
	for (auto &bf: files){
		if (bf.session == session){
			session->i(_("deleting backup: ") + bf.filename);
			file_delete(bf.filename);
			bf.session = NULL; // auto remove
		}
	}
	_clear_old();
}

void BackupManager::check_old_files(Session *session)
{
	_clear_old();

	// update list
	auto _files = dir_search(tsunami->directory, "backup-*", false);
	for (auto &f: _files){
		BackupFile bf;
		bf.session = NULL;
		bf.f = NULL;
		bf.filename = tsunami->directory + f.name;
		files.add(bf);
	}

	// check
	for (auto &bf: files){
		if (!bf.session)
			session->w(_("recording backup found: ") + bf.filename);
	}
}

File *BackupManager::create_file(const string &extension, Session *session)
{
	BackupFile bf;
	bf.session = session;
	bf.filename = get_filename(extension);
	session->i(_("creating backup: ") + bf.filename);
	try{
		bf.f = FileCreate(bf.filename);
		files.add(bf);
		return bf.f;
	}catch(FileError &e){
		session->e(e.message());
	}
	return NULL;
}

void BackupManager::abort(File *f)
{
	//delete f;
	done(f);
}

void BackupManager::done(File *f)
{
	delete f;
	auto bf = _find_by_file(f);
	if (bf){
		bf->session->i(_("backup done: ") + bf->filename);
		bf->f = NULL;
	}
}

BackupManager::BackupFile* BackupManager::_find_by_file(File *f)
{
	if (!f)
		return NULL;
	for (auto &bf: files)
		if (bf.f == f)
			return &bf;
	return NULL;
}

void BackupManager::_clear_old()
{
	for (int i=0; i<files.num; i++)
		if (!files[i].session){
			files.erase(i);
			i --;
		}
}
