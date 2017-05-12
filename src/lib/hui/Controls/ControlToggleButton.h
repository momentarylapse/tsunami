/*
 * ControlToggleButton.h
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#ifndef CONTROLTOGGLEBUTTON_H_
#define CONTROLTOGGLEBUTTON_H_

#include "Control.h"

namespace hui
{

class ControlToggleButton : public Control
{
public:
	ControlToggleButton(const string &text, const string &id);
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void setImage(const string &str);
	virtual void __check(bool checked);
	virtual bool isChecked();
};

};

#endif /* CONTROLTOGGLEBUTTON_H_ */
