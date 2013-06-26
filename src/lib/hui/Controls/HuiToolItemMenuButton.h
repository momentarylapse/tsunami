/*
 * HuiToolItemMenuButton.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef HUITOOLITEMMENUBUTTON_H_
#define HUITOOLITEMMENUBUTTON_H_

#include "HuiControl.h"

class HuiMenu;

class HuiToolItemMenuButton : public HuiControl
{
public:
	HuiToolItemMenuButton(const string &title, HuiMenu *menu, const string &image, const string &id);
	virtual ~HuiToolItemMenuButton();
};

#endif /* HUITOOLITEMMENUBUTTON_H_ */
