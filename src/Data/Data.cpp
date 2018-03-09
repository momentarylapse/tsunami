/*
 * Data.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Data.h"
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



void Data::resetHistory()
{
	action_manager->reset();
}

// "low level" -> don't use ActionManager.lock()!
void Data::lock()
{
	action_manager->mutex->lock();
}

bool Data::try_lock()
{
	return action_manager->mutex->tryLock();
}

void Data::unlock()
{
	action_manager->mutex->unlock();
}




