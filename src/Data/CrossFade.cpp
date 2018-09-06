/*
 * CrossFade.cpp
 *
 *  Created on: 06.09.2018
 *      Author: michi
 */

#include "CrossFade.h"


Range CrossFade::range()
{
	return Range(position, samples);
}

