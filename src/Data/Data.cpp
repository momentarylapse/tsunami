/*
 * Data.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Data.h"
#include "../Action/ActionManager.h"
#include "../lib/threads/Mutex.h"


const string Data::MESSAGE_START_LOADING = "StartLoading";
const string Data::MESSAGE_FINISHED_LOADING = "FinishedLoading";
const string Data::MESSAGE_BEFORE_CHANGE = "BeforeChange";
const string Data::MESSAGE_AFTER_CHANGE = "AfterChange";

Data::Data(Session *_session) {
	action_manager = new ActionManager(this);
	session = _session;
	binary_file_format = true;
	file_time = 0;
}

Data::~Data() {
	delete action_manager;
}



bool Data::redo() {
	return action_manager->redo();
}

bool Data::undo() {
	return action_manager->undo();
}



void *Data::execute(Action *a) {
	return action_manager->execute(a);
}

void Data::begin_action_group() {
	action_manager->group_begin();
}

void Data::end_action_group() {
	action_manager->group_end();
}


void Data::reset_history() {
	action_manager->reset();
}

bool Data::history_enabled() {
	return action_manager->is_enabled();
}

// "low level" -> don't use ActionManager.lock()!
void Data::lock() {
	mtx.lock();
}

bool Data::try_lock() {
	return mtx.try_lock();
}

void Data::unlock() {
	mtx.unlock();
}




