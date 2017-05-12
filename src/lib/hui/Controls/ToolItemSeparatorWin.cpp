/*
 * HuiToolItemSeparatorWin.cpp
 *
 *  Created on: 13.07.2013
 *      Author: michi
 */

#include "ToolItemSeparator.h"

namespace hui
{

#ifdef HUI_API_WIN

HuiToolItemSeparator::HuiToolItemSeparator() :
	HuiControl(HuiKindToolSeparator, "")
{
}

HuiToolItemSeparator::~HuiToolItemSeparator()
{
}

#endif

}
