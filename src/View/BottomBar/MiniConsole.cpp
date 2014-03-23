/*
 * MiniConsole.cpp
 *
 *  Created on: 23.03.2014
 *      Author: michi
 */

#include "MiniConsole.h"

MiniConsole::MiniConsole()
{
	AddControlTable("!noexpandx", 0, 0, 5, 1, "grid");
	SetTarget("grid", 0);
	AddButton("Test", 0, 0, 0, 0, "test");
}

MiniConsole::~MiniConsole()
{
}

