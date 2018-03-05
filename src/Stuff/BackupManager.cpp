/*
 * BackupManager.cpp
 *
 *  Created on: 05.03.2018
 *      Author: michi
 */

#include "BackupManager.h"
#include "../Data/Track.h"
#include "../Tsunami.h"
#include "Log.h"

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


void BackupManager::set_save_state()
{
	for (auto &bf: files){
		if (bf.from_this_session){
			tsunami->log->info(_("deleting backup: ") + bf.filename);
			file_delete(bf.filename);
			bf.from_this_session = false; // auto remove
		}
	}
	_clear_old();
}

void BackupManager::check_old_files()
{
	_clear_old();

	// update list
	auto _files = dir_search(tsunami->directory, "backup-*", false);
	for (auto &f: _files){
		BackupFile bf;
		bf.from_this_session = false;
		bf.f = NULL;
		bf.filename = tsunami->directory + f.name;
		files.add(bf);
	}

	// check
	for (auto &bf: files){
		if (!bf.from_this_session)
			tsunami->log->warn(_("recording backup found: ") + bf.filename);
	}
}

File *BackupManager::get_file(const string &extension)
{
	BackupFile bf;
	bf.from_this_session = true;
	bf.filename = get_filename(extension);
	tsunami->log->info(_("creating backup: ") + bf.filename);
	try{
		bf.f = FileCreate(bf.filename);
		files.add(bf);
		return bf.f;
	}catch(FileError &e){
		tsunami->log->error(e.message());
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
		tsunami->log->info(_("backup done: ") + bf->filename);
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
		if (!files[i].from_this_session){
			files.erase(i);
			i --;
		}
}
