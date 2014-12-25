/*
 * HuiControlLabel.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUI_CONTROL_LABEL_H_
#define HUI_CONTROL_LABEL_H_

#include "HuiControl.h"


class HuiControlLabel : public HuiControl
{
public:
	HuiControlLabel(const string &text, const string &id);
	virtual string getString();
	virtual void __setString(const string &str);
	virtual void __setOption(const string &op, const string &value);
};


#endif /* HUI_CONTROL_LABEL_H_ */
