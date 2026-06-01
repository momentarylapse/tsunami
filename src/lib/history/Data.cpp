/*
 * Data.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Data.h"

namespace history {

Data::Data(int _type) {
	type = _type;
	action_manager = new ActionManager(this);
	file_time = 0;
}

Data::~Data() {
	delete action_manager;
}



void Data::begin_action_group(const string &name) {
	action_manager->begin_group(name);
}

void Data::end_action_group() {
	action_manager->end_group();
}

bool Data::redo() {
	return action_manager->redo();
}

bool Data::undo() {
	return action_manager->undo();
}

bool Data::undoable() const {
	return action_manager->undoable();
}

bool Data::redoable() const {
	return action_manager->redoable();
}


void *Data::execute(Action *a) {
	return action_manager->execute(a);
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

void Data::lock_shared() {
	mtx.lock_shared();
}

bool Data::try_lock_shared() {
	return mtx.try_lock_shared();
}

void Data::unlock_shared() {
	mtx.unlock_shared();
}

}

