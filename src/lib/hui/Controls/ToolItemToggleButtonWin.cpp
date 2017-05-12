/*
 * HuiToolItemToggleButtonWin.cpp
 *
 *  Created on: 13.07.2013
 *      Author: michi
 */

#include "ToolItemToggleButton.h"

#ifdef HUI_API_WIN

namespace hui
{

HuiToolItemToggleButton::HuiToolItemToggleButton(const string &title, const string &image, const string &id) :
	HuiControl(HuiKindToolToggleButton, id)
{
}

HuiToolItemToggleButton::~HuiToolItemToggleButton()
{
}

void HuiToolItemToggleButton::__Check(bool checked)
{
}

bool HuiToolItemToggleButton::IsChecked()
{
	return false;
}

}

#endif
