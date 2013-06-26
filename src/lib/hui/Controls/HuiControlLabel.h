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
	virtual ~HuiControlLabel();
	virtual string GetString();
	virtual void __SetString(const string &str);
};


#endif /* HUI_CONTROL_LABEL_H_ */
