#include "../../base/base.h"
#include "../../base/map.h"

namespace kaba {

Array<string> dict_get_keys(const DynamicArray& a) {
	Array<string> keys;
	for (int i=0; i<a.num; i++)
		keys.add(*(string*)((char*)a.data + i * a.element_size));
	return keys;
}



}
