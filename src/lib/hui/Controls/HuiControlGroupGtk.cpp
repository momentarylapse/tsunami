/*
 * HuiControlGroup.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlGroup.h"

HuiControlGroup::HuiControlGroup(const string &title, const string &id) :
	HuiControl(HuiKindGroup, id)
{
	GetPartStrings(id, title);
	widget = gtk_frame_new(sys_str(PartString[0]));
}

HuiControlGroup::~HuiControlGroup() {
	// TODO Auto-generated destructor stub
}

