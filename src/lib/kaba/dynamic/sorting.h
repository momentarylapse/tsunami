/*
 * sorting.h
 *
 *  Created on: Nov 20, 2022
 *      Author: michi
 */

#pragma once

class DynamicArray;

namespace kaba {
	
class Class;

DynamicArray array_sort(DynamicArray &array, const Class *type, const string &by);

}
