/*
 * Data.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Data.h"
#include "../Action/ActionManager.h"
#include "../lib/threads/Mutex.h"


const string Data::MESSAGE_FINISHED_LOADING = "FinishedLoading";

Data::Data(Session *_session)
{
	action_manager = new ActionManager(this);
	session = _session;
	binary_file_format = true;
	file_time = 0;
}

Data::~Data()
{
	delete(action_manager);
}



void Data::redo()
{
	action_manager->redo();
}



void Data::undo()
{
	action_manager->undo();
}



void *Data::execute(Action *a)
{
	return action_manager->execute(a);
}

void Data::beginActionGroup()
{
	action_manager->beginActionGroup();
}

void Data::endActionGroup()
{
	action_manager->endActionGroup();
}


void Data::resetHistory()
{
	action_manager->reset();
}

bool Data::history_enabled()
{
	return action_manager->isEnabled();
}

// "low level" -> don't use ActionManager.lock()!
void Data::lock()
{
	action_manager->mtx.lock();
}

bool Data::try_lock()
{
	return action_manager->mtx.try_lock();
}

void Data::unlock()
{
	action_manager->mtx.unlock();
}




