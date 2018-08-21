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
	string getString() override;
	void __setString(const string &str) override;
	void setImage(const string &str) override;
	void __check(bool checked) override;
	bool isChecked() override;
	void __setOption(const string &op, const string &value) override;
};

};

#endif /* CONTROLTOGGLEBUTTON_H_ */
