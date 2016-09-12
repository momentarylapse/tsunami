/*
 * Data.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Data.h"
#include "../lib/threads/Mutex.h"

Data::Data(const string &name) :
	Observable(name)
{
	mutex = new Mutex;
	action_manager = new ActionManager(this);
	binary_file_format = true;
	file_time = 0;
}

Data::~Data()
{
	delete(action_manager);
	delete(mutex);
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




