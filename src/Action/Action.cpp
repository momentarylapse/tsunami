/*
 * Action.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Action.h"


// default behavior for redo...
void Action::redo(Data *d) {
	execute(d);
}


