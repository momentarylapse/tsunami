/*
 * ControlEdit.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLEDIT_H_
#define CONTROLEDIT_H_

#include "Control.h"

namespace hui
{

class ControlEdit : public Control
{
public:
	ControlEdit(const string &text, const string &id);
	string get_string() override;
	void __set_string(const string &str) override;
	void __reset() override;
	void completion_add(const string &text) override;
	void completion_clear() override;
	void __set_option(const string &op, const string &value) override;
};

};

#endif /* CONTROLEDIT_H_ */
