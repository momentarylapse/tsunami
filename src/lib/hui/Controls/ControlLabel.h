/*
 * ControlLabel.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROL_LABEL_H_
#define CONTROL_LABEL_H_

#include "Control.h"

namespace hui
{


class ControlLabel : public Control
{
public:
	ControlLabel(const string &text, const string &id);
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void __setOption(const string &op, const string &value);
};

};


#endif /* CONTROL_LABEL_H_ */
