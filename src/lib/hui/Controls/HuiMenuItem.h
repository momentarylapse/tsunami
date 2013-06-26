/*
 * HuiMenuItem.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef HUIMENUITEM_H_
#define HUIMENUITEM_H_

#include "HuiControl.h"

class HuiMenuItem : public HuiControl
{
public:
	HuiMenuItem(const string &title, const string &id);
	virtual ~HuiMenuItem();

	virtual void SetImage(const string &image);
};

#endif /* HUIMENUITEM_H_ */
