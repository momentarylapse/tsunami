/*
 * Data.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Data.h"

Data::Data(const string &name) :
	Observable(name)
{
	action_manager = new ActionManager(this);
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




